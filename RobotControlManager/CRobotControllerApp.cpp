/* +---------------------------------------------------------------------------+
   |                 Open MORA (MObile Robot Arquitecture)                     |
   |                                                                           |
   |                        http://babel.isa.uma.es/mora/                      |
   |                                                                           |
   |   Copyright (C) 2010  University of Malaga                                |
   |                                                                           |
   |    This software was written by the Machine Perception and Intelligent    |
   |      Robotics (MAPIR) Lab, University of Malaga (Spain).                  |
   |    Contact: Jose-Luis Blanco  <jlblanco@ctima.uma.es>                     |
   |                                                                           |
   |  This file is part of the MORA project.                                   |
   |                                                                           |
   |     MORA is free software: you can redistribute it and/or modify          |
   |     it under the terms of the GNU General Public License as published by  |
   |     the Free Software Foundation, either version 3 of the License, or     |
   |     (at your option) any later version.                                   |
   |                                                                           |
   |   MORA is distributed in the hope that it will be useful,                 |
   |     but WITHOUT ANY WARRANTY; without even the implied warranty of        |
   |     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         |
   |     GNU General Public License for more details.                          |
   |                                                                           |
   |     You should have received a copy of the GNU General Public License     |
   |     along with MORA.  If not, see <http://www.gnu.org/licenses/>.         |
   |                                                                           |
   +---------------------------------------------------------------------------+ */


/**  @moos_module Module in charge of controlling the Operation mode (autonomous/Manual) of a Robot.
  *  The module controls when the robot is to be teleoperated manually via Pilot, etc.,
  *  or when the OpenMORA modules (ReactiveNavigation, NavigatorTopological, etc.) are in control.
  *  This module also implements some security procedures in case of errors or malfunctions, sucha as:
  *  - Stopping the robot and returning the control to manual mode
  *  - Returning to Docking when low batery is detected (if enabled).
  *  - Returning to Docking or stooping when Client connection is lost (if enabled).
  */

#include "CRobotControllerApp.h"

#include <mrpt/version.h>
#if MRPT_VERSION>=0x130
#	include <mrpt/obs/CObservationBatteryState.h>
#else
#	include <mrpt/slam/CObservationBatteryState.h>
#endif

#include <sstream>
#include <iomanip>
#include <iostream>
#include <stdlib.h>     /* abs */
#include <mrpt/system.h>

#ifdef _WIN32
  #define atoll(S) _atoi64(S)
#endif

using namespace std;
using namespace mrpt;
#if MRPT_VERSION>=0x130
using namespace mrpt::obs;
#else
using namespace mrpt::slam;
#endif
using namespace mrpt::utils;
using namespace mrpt::poses;


/** Constructor */
CRobotControllerApp::CRobotControllerApp()
{
}

/** Destructor*/
CRobotControllerApp::~CRobotControllerApp()
{
}

bool CRobotControllerApp::OnStartUp()
{

	try
    {
		printf("STARTING ROBOT_CONTROL_MANAGER\n");
		printf("---------------------------------\n");
		printf("[RobotController]: Loading Options from file...\n");
		std::string section = "";
		verbose = true;

		//! @moos_param working_mode Indicates (auto/onlyOpenMORA) if the robot controller should listen to the commands sent by an external client (pilot, etc.), or not.
		working_mode = m_ini.read_string(section,"working_mode","auto",true);
		//cout << "working_mode = " << working_mode << endl;

		if( MOOSStrCmp(working_mode,"onlyOpenMORA") )
		{
			//Start robot in autonomous mode (0 = Manual=Pilot, 2=Autonomous=OpenMORA)
			Robot_control_mode = 2;
			printf("[RobotController]: Robot starts in mode: AUTONOMOUS.\n");
		}
		else
		{
			//Allow changing from Pilot to OpenMORA as requested (0 = Manual=Pilot, 2=Autonomous=OpenMORA)
			//By default, start in Manual Mode
			Robot_control_mode = 0;
			printf("[RobotController]:  Robot starts in mode: MANUAL.\n");
		}
		//! @moos_publish ROBOT_CONTROL_MODE The robot working mode: 0=Manual=Pilot, 2=Autonomous=OpenMORA
		m_Comms.Notify("ROBOT_CONTROL_MODE", Robot_control_mode);

		//! @moos_publish COLLABORATIVE Indicates if collaborative control is requested by the user
		m_Comms.Notify("COLLABORATIVE", 0.0);

		//! @moos_publish TOPOLOGICAL_DESTINY Name of the node where the reactive navigation has been planned.
		m_Comms.Notify("TOPOLOGICAL_DESTINY","NULL");

		// CLIENT ACK
		//! @moos_param use_client_alive_ack Indicates (true/false) if the robot controller should periodically check the ACK sent by the MQTT client to cancel Navigation on error.
		use_client_alive_ack = m_ini.read_bool(section,"use_client_alive_ack","true",true);
		//! @moos_param max_client_ack_interval The maximum interval (seconds) between Client's ACK if use_client_alive_ack=true
		max_client_ack_interval = m_ini.read_double(section,"max_client_ack_interval",2,true);

		// BATTERY STATUS
		//! @moos_param check_battery_status Indicates (true/false) if the robot controller should periodically check the battery status and return to Docking when low.
		check_battery_status = m_ini.read_bool(section,"check_battery_status","true",true);

		//! @moos_param battery_base Indicates true=Monitor battery from the robot base, false=monitor external battery (see BatteryManager module)
		battery_base = m_ini.read_bool(section,"battery_base","true",true);

		//! @moos_param battery_threshold_warning Battery level that once is reached will generate a user warning
		battery_threshold_warning = m_ini.read_float(section,"battery_threshold_warning",26.0,false);

		//! @moos_param battery_threshold_recharge Battery level that once is reached will cause the robot to go Recharge
		battery_threshold_recharge = m_ini.read_float(section,"battery_threshold_recharge",25.0,false);

		//! @moos_param battery_threshold_charged Battery level corresponding to a complete charge
		battery_threshold_charged = m_ini.read_float(section,"battery_threshold_charged",29.0,false);

		mqtt_status = "OFFLINE";
		client_connected = "NONE";
		Is_Charging = 1.0;		//Start as charging (default behaviour)
		last_bettery_display_time = mrpt::system::now();
		last_bettery_warning_time = mrpt::system::now();
		going_to_docking = false;
		last_working_zone = "NULL";
		printf("[RobotController]: Initial configuration done.\n\n\n");
		return DoRegistrations();
    }
	catch (std::exception &e)
	{
		return MOOSFail( (string("**ERROR**") + string(e.what())).c_str() );
	}
}

bool CRobotControllerApp::OnCommandMsg( CMOOSMsg Msg )
{
    if(Msg.IsSkewed(MOOSTime()))
        return true;

    if(!Msg.IsString())
        return MOOSFail("Robot_Control_Manager only accepts string command messages\n");

    std::string sCmd = Msg.GetString();

    MOOSTrace("COMMAND RECEIVED: %s\n",sCmd.c_str());

    return true;
}

bool CRobotControllerApp::Iterate()
{
	try
	{
		//----------------------------
		// Check level of the battery
		//----------------------------
		if( check_battery_status )
		{
			if( battery_base )
			{
				CMOOSVariable * pVarBattery = GetMOOSVar( "BATTERY_V" );
				if( pVarBattery && pVarBattery->IsFresh() )
				{
					pVarBattery->SetFresh(false);
					CSerializablePtr obj;
					mrpt::utils::RawStringToObject(pVarBattery->GetStringVal(),obj);

					if( IS_CLASS(obj, CObservationBatteryState) )
					{
						CObservationBatteryStatePtr battery_obs = CObservationBatteryStatePtr(obj);
						CheckBattery(battery_obs->voltageMainRobotBattery);
					}
				}
			}
			else
			{
				CMOOSVariable * pVarBatteryManager = GetMOOSVar( "BATTERY_MANAGER_V" );
				if( pVarBatteryManager && pVarBatteryManager->IsFresh() )
				{
					pVarBatteryManager->SetFresh(false);
					CSerializablePtr obj;
					mrpt::utils::RawStringToObject(pVarBatteryManager->GetStringVal(),obj);

					if( IS_CLASS(obj, CObservationBatteryState) )
					{
						CObservationBatteryStatePtr battery_obs = CObservationBatteryStatePtr(obj);
						CheckBattery(battery_obs->voltageMainRobotBattery);
					}
				}
			}
		}//end-if CheckBattery


		//----------------------------
		// Check Client connection
		//----------------------------
		if( mrpt::system::timeDifference(last_mqtt_ack_time,mrpt::system::now()) > max_client_ack_interval )
		{
			//Connection to client lost, close logger session
			if( client_connected != "NONE" )
			{
				printf("[RobotController]: Client ACK timeout!. Clossing session\n.");
				//! @moos_publish LOGGER_STOP Stops the current session in the SessionLogger module
				m_Comms.Notify("LOGGER_STOP","1");
				client_connected = "NONE";
			}

			if( use_client_alive_ack && Is_Charging==0 )
			{
				// SECURITY - Stop robot and return to Manual Mode (User control only)
				cout << "[Robot_Control_Manager]: ERROR - MQTT Client ACK timeout! " << endl;
				//! @moos_publish CANCEL_NAVIGATION
				m_Comms.Notify("CANCEL_NAVIGATION", "RobotController:MQTT lost");
			}
		}

		return true;
    }
	catch (std::exception &e)
	{
		return MOOSFail( (string("**ERROR**") + string(e.what())).c_str() );
	}
}



bool CRobotControllerApp::OnConnectToServer()
{
    DoRegistrations();
    return true;
}


bool CRobotControllerApp::DoRegistrations()
{
	//! @moos_subscribe GO_TO_NODE
    AddMOOSVariable( "GO_TO_NODE", "GO_TO_NODE", "GO_TO_NODE", 0);

	//! @moos_subscribe NAVIGATE_TARGET
    AddMOOSVariable( "NAVIGATE_TARGET", "NAVIGATE_TARGET", "NAVIGATE_TARGET", 0);

	//! @moos_subscribe PLAN_FINISHED
    AddMOOSVariable( "PLAN_FINISHED", "PLAN_FINISHED", "PLAN_FINISHED", 0);

	//! @moos_subscribe CANCEL_NAVIGATION
    AddMOOSVariable( "CANCEL_NAVIGATION", "CANCEL_NAVIGATION", "CANCEL_NAVIGATION", 0);

	//! @moos_subscribe CLIENT_MQTT_ACK
    AddMOOSVariable( "CLIENT_MQTT_ACK", "CLIENT_MQTT_ACK", "CLIENT_MQTT_ACK", 0);

	//! @moos_subscribe MQTT_CONNECT_STATUS
	AddMOOSVariable( "MQTT_CONNECT_STATUS", "MQTT_CONNECT_STATUS", "MQTT_CONNECT_STATUS", 0);

	//! @moos_subscribe GO_TO_RECHARGE
	AddMOOSVariable( "GO_TO_RECHARGE", "GO_TO_RECHARGE", "GO_TO_RECHARGE", 0);


	//! @moos_subscribe BATTERY_V
	AddMOOSVariable( "BATTERY_V", "BATTERY_V", "BATTERY_V", 0);

	//! @moos_subscribe BATTERY_IS_CHARGING
	AddMOOSVariable( "BATTERY_IS_CHARGING", "BATTERY_IS_CHARGING", "BATTERY_IS_CHARGING", 0);


	//! @moos_subscribe BATTERY_MANAGER_V
	AddMOOSVariable( "BATTERY_MANAGER_V", "BATTERY_MANAGER_V", "BATTERY_MANAGER_V", 0);

	//! @moos_subscribe BATTERY_MANAGER_IS_CHARGING
	AddMOOSVariable( "BATTERY_MANAGER_IS_CHARGING", "BATTERY_MANAGER_IS_CHARGING", "BATTERY_MANAGER_IS_CHARGING", 0);


	//! @moos_subscribe RANDOM_NAVIGATOR
	AddMOOSVariable( "RANDOM_NAVIGATOR", "RANDOM_NAVIGATOR", "RANDOM_NAVIGATOR", 0);

	//! @moos_subscribe PARKING
	AddMOOSVariable( "PARKING", "PARKING", "PARKING", 0);

	//! @moos_subscribe MOTION_REQUEST
	AddMOOSVariable( "MOTION_REQUEST", "MOTION_REQUEST", "MOTION_REQUEST", 0);

	//! @moos_subscribe COLLABORATIVE_REQUEST
	AddMOOSVariable( "COLLABORATIVE_REQUEST", "COLLABORATIVE_REQUEST", "COLLABORATIVE_REQUEST", 0);

	//! @moos_subscribe LOCALIZATION
	AddMOOSVariable( "LOCALIZATION", "LOCALIZATION", "LOCALIZATION", 0);

	//! @moos_subscribe SHUTDOWN
	AddMOOSVariable( "SHUTDOWN", "SHUTDOWN", "SHUTDOWN", 0);

    RegisterMOOSVariables();
    return true;
}

bool CRobotControllerApp::OnNewMail(MOOSMSG_LIST &NewMail)
{
	for (MOOSMSG_LIST::const_iterator it=NewMail.begin();it!=NewMail.end();++it)
	{
	    const CMOOSMsg &m = *it;

		// The user requested a Reactive Navigation through MQTT
		if( MOOSStrCmp(m.GetKey(),"GO_TO_NODE") )
		{
			//Set Autonomous mode
			SetAutonomousMode();

			//! @moos_publish NEW_TASK Request a new task to the Agenda.
			m_Comms.Notify("NEW_TASK", "ROBOT_CONTROLLER 0 MOVE "+m.GetString() );
		}

		// PLAN_FINISHED TaskID UserTaskID UserID
		if( MOOSStrCmp(m.GetKey(),"PLAN_FINISHED") )
		{
			std::deque<std::string> cad;
			mrpt::system::tokenize(m.GetString()," ",cad);

			//End of Reactive Navigation
			if( cad[2]=="ROBOT_CONTROLLER" && atoi(cad[1].c_str())==0)
			{
				//Navigation eneded: Set Manual mode
				printf("[Robot_Control_Manager] Task ReactiveNavigation FINISHED!\n");
				SetManualMode();
			}

			//end of GO_TO_RECHARGE (Robot is in the Docking node, and the Autodocking module is active)
			else if( cad[2]=="ROBOT_CONTROLLER" && atoi(cad[1].c_str())==505)
			{
				going_to_docking = false;
				/*
				cout << "[Robot_Control_Manager]: Task Battery_Recharge has been completed" << endl;
				//Check if random navigations was temporarily disabled
				CMOOSVariable * pVar = GetMOOSVar("RANDOM_NAVIGATOR");
				if(pVar)
				{
					double random_navigator_mode = pVar->GetDoubleVal();
					if( random_navigator_mode==2.0 )
					{
						//Battery is ok. Reactivate Random Navigation Module
						//! @moos_publish NEW_TASK Request a new task to the Agenda.
						m_Comms.Notify("NEW_TASK", "ROBOT_CONTROLLER 1 UNDOCK");
						mrpt::system::sleep(2000);
						//! @moos_publish RANDOM_NAVIGATOR Activates/Deactivates the random navigator module
						m_Comms.Notify("RANDOM_NAVIGATOR", 1.0);
					}
				}
				*/
			}
		}

		// NAVIGATE_TARGET x y
	    if( MOOSStrCmp(m.GetKey(),"NAVIGATE_TARGET") )
		{
			//New target (x,y) set - Collaborative or Reactive!
			//Set Autonomous mode
			SetAutonomousMode();
		}


		if( MOOSStrCmp(m.GetKey(),"CANCEL_NAVIGATION") )
		{
			//Someone request to Cancel current Navigation!
			//if( Robot_control_mode == 2 && !MOOSStrCmp(working_mode,"onlyOpenMORA") )
			{
				cout << endl << "[Robot_Control_Manager]: CANCEL_NAVIGATION - " << m.GetString() << endl;
				//STOP THE ROBOT & Set Manual mode
				//---------------------------------
				//! @moos_publish PNAVIGATORREACTIVEPTG3D_CMD Request the reactive3D to stop current navigation
				m_Comms.Notify("PNAVIGATORREACTIVEPTG3D_CMD", "CANCEL");
				//! @moos_publish CANCEL_IN_EXECUTION Cancel current Navigation Plans (Executor)
				m_Comms.Notify("CANCEL_IN_EXECUTION", 1.0);
				//! @moos_publish RANDOM_NAVIGATOR Activates/Deactivates the random navigator module
				m_Comms.Notify("RANDOM_NAVIGATOR", 0.0);
				//! @moos_publish PARKING Variable that indicates when the robot should start the Docking process.
				m_Comms.Notify("PARKING", 0.0);
				going_to_docking = false;
				SetManualMode();
			}
		}


		if( MOOSStrCmp(m.GetKey(),"MQTT_CONNECT_STATUS") )
		{
			// MQTT status changed
			if( !MOOSStrCmp(mqtt_status,m.GetString()) )
			{
				mqtt_status = m.GetString();
				cout << "[RobotController]: MQTT status changed to: " << m.GetString() << endl;
			}
		}


		if( MOOSStrCmp(m.GetKey(),"CLIENT_MQTT_ACK") )
		{
			//Update time of last client ACK
			last_mqtt_ack_time = mrpt::system::now();
			if( client_connected == "NONE" )
			{
				client_connected = m.GetString();
				printf("[RobotController]: Starting new session for Client: %s\n", client_connected.c_str());
				//! @moos_publish LOGGER_START Start a new session in the SessionLogger Module with the provided username
				m_Comms.Notify("LOGGER_START", client_connected.c_str());
			}
		}



		if( MOOSStrCmp(m.GetKey(),"BATTERY_IS_CHARGING") || MOOSStrCmp(m.GetKey(),"BATTERY_MANAGER_IS_CHARGING") )
		{
			//Update charging status
			Is_Charging = m.GetDouble();
			cout << "[RobotController]: Robot Charging status changed to: " << Is_Charging << endl;
			if( Is_Charging == 1.0 )
				going_to_docking = false; //Already there
		}


		if( MOOSStrCmp(m.GetKey(),"PARKING") )
		{
			//Someone requested to de/activate the Autodocking assistant
			double parking_status = m.GetDouble();
			if (parking_status == 1.0)
			{
				//printf("[Robot_Control_Manager]: STARTING Autodocking!\n");
				SetAutonomousMode();
			}
			else
			{
				//printf("[Robot_Control_Manager]: STOPPING Autodocking!\n");
				SetManualMode();
			}
		}

		if( MOOSStrCmp(m.GetKey(),"GO_TO_RECHARGE") )
		{
			//Someone requested the robot to recharge its batteries
			GoToRecharge();
		}

		//MOTION_REQUEST v w
		if( MOOSStrCmp(m.GetKey(),"MOTION_REQUEST") )
		{
			std::deque<std::string> list;
			mrpt::system::tokenize(m.GetString()," ",list);
			if( list.size() ==2 )
			{
				std::string setV = list.at(0);
				std::string setW = list.at(1);

				SetAutonomousMode();

				//! @moos_publish MOTION_CMD_V Set robot linear speed
				m_Comms.Notify("MOTION_CMD_V",atof(setV.c_str()));
				//! @moos_publish MOTION_CMD_W Set robot angular speed
				m_Comms.Notify("MOTION_CMD_W",atof(setW.c_str()));
			}
			else
				cout << "[RobotController] ERROR: Incorrect Motion Command" << endl;
		}
		//COLLABORATIVE_REQUEST mod([0,1]) ang(deg) ttimestamp (ms)
		//COLLABORATIVE_REQUEST start ttimestamp
		//COLLABORATIVE_REQUEST stop ttimestamp
		if( MOOSStrCmp(m.GetKey(),"COLLABORATIVE_REQUEST") )
		{
			std::deque<std::string> list;
			mrpt::system::tokenize(m.GetString()," ",list);

			if( list.size() == 2 )
			{
				if (list.at(0) == "start")
				{
					//! @moos_publish COLLABORATIVE Indicates if collaborative control is requested by the user
					m_Comms.Notify("COLLABORATIVE", 1.0);

					//Get reference of user time
					try
					{
						mrpt::system::TTimeStamp ref = mrpt::system::now()/10000;	//Current time in ms
						initial_collaborative_delay = ref - static_cast<uint64_t>(atoll(list.at(1).c_str())) ;
						printf("[RobotController]: Collaborative control ACTIVATED.\n");
					}
					catch (std::exception &e) {	return MOOSFail( (string("**COLLABORATIVE START ERROR: ") + string(e.what())).c_str() ); }
					catch (...)	{ printf("UNKNOWN COLLABORATIVE START ERROR.\n"); }
				}
				else
				{
					//! @moos_publish COLLABORATIVE Indicates if collaborative control is requested by the user
					m_Comms.Notify("COLLABORATIVE", 0.0);
					printf("[RobotController]: Collaborative control DEACTIVATED.\n");
				}
			}
			else if( list.size() == 3 )
			{
				//Check that delay is not to big, else discard command.
				//COLLABORATIVE_REQUEST mod([0,1]) ang(deg) ttimestamp (ms)
				uint64_t current_delay;
				try
				{
					mrpt::system::TTimeStamp ref = mrpt::system::now()/10000;	//Current time in ms
					current_delay = ref - static_cast<uint64_t>(atoll(list.at(2).c_str()));
				}
				catch (std::exception &e){ return MOOSFail( (string("**COLLABORATIVE COMMAND ERROR**") + string(e.what())).c_str() ); }
				catch (...){ printf("UNKNOWN COLLABORATIVE COMMAND ERROR.\n"); }

				if (current_delay > initial_collaborative_delay+ 200)
				{
					printf("[MQTTMosquitto:Collaborative]:ERROR Collaborative command TOO OLD. Discarding...\n");
				}
				else
				{
					double commandModuleNormalized = atof(list.at(0).c_str());
					double commandAngle = atof(list.at(1).c_str());

					double maxDistanceForCollaborativeTarget = 3.0;					//Máxima distancia del target en el caso de recibir un módulo máximo
					double maxAngularSpeed = 2.0;									 //Definimos aquí el valor máximo de la velocidad angular
					double thresholdAngletoCommandPureRotationMovement = 45.0;

					SetAutonomousMode();

					//Get current Robot localization : LOCALIZATION [x y phi]
					CMOOSVariable * pVarLoc = GetMOOSVar("LOCALIZATION");
					double targetX, targetY,locX,locY,angle;
					if(pVarLoc)
					{
							//delete brackets from localization
							std::string locS = pVarLoc->GetStringVal().substr(1,pVarLoc->GetStringVal().size()-2);

							std::deque<std::string> list_loc;
							mrpt::system::tokenize(locS," ",list_loc);

							locX = atof(list_loc.at(0).c_str());
							locY = atof(list_loc.at(1).c_str());
							angle = atof(list_loc.at(2).c_str());
					}
					else
						printf("[MQTTMosquitto:Collaborative]: Error, Localization not available. Collaborative command discarded.\n");


					if(commandAngle < -thresholdAngletoCommandPureRotationMovement)
					{
						printf("[MQTTMosquitto:Collaborative]:Turning Right..\n");

						if (last_working_zone == "target")
						{
							targetX = locX;
							targetY = locY;
							const string target = format("[%.03f %.03f]", targetX,targetY);
							//! @moos_publish NAVIGATE_TARGET Set destination ([x y]) for robot to go autonomously
							printf("[MQTTMosquitto:Collaborative]: Selected target is: %s\n",target.c_str());
							m_Comms.Notify("NAVIGATE_TARGET",target);

							//! @moos_publish CANCEL_NAVIGATION
							m_Comms.Notify("CANCEL_NAVIGATION", "RobotController-Collaborative");
						}

						//! @moos_publish MOTION_CMD_V Set robot linear speed
						m_Comms.Notify("MOTION_CMD_V",0.0);
						//! @moos_publish MOTION_CMD_W Set robot angular speed
						m_Comms.Notify("MOTION_CMD_W",-commandModuleNormalized*maxAngularSpeed);
						last_working_zone = "turn";
					}
					else if( commandAngle > thresholdAngletoCommandPureRotationMovement)
					{
						printf("[MQTTMosquitto:Collaborative]:Turning Left..\n");
						if (last_working_zone == "target")
						{
							targetX = locX;
							targetY = locY;
							const string target = format("[%.03f %.03f]", targetX,targetY);
							//! @moos_publish NAVIGATE_TARGET Set destination ([x y]) for robot to go autonomously
							printf("[MQTTMosquitto:Collaborative]: Selected target is: %s\n",target.c_str());
							m_Comms.Notify("NAVIGATE_TARGET",target);

							//! @moos_publish CANCEL_NAVIGATION
							m_Comms.Notify("CANCEL_NAVIGATION", "RobotController-Collaborative");
						}

						//! @moos_publish MOTION_CMD_V Set robot linear speed
						m_Comms.Notify("MOTION_CMD_V",0.0);
						//! @moos_publish MOTION_CMD_W Set robot angular speed
						m_Comms.Notify("MOTION_CMD_W",commandModuleNormalized*maxAngularSpeed);
						last_working_zone = "turn";
					}
					else
					{
						printf("[MQTTMosquitto:Collaborative]:Going to target..\n");
						try
						{
							targetX = locX + maxDistanceForCollaborativeTarget*commandModuleNormalized*cos( (angle+commandAngle) * PI / 180.0 );
							targetY = locY + maxDistanceForCollaborativeTarget*commandModuleNormalized*sin( (angle+commandAngle) * PI / 180.0 );
							printf("Targets are: %.03f %.03f\n",targetX, targetY);

							const string target = format("[%.03f %.03f]", targetX,targetY);
							//! @moos_publish NAVIGATE_TARGET Set destination ([x y]) for robot to go autonomously
							printf("[MQTTMosquitto:Collaborative]: Selected target is: %s\n",target.c_str());
							m_Comms.Notify("NAVIGATE_TARGET",target);
						}
						catch (std::exception &e)
						{ return MOOSFail( (string("**COLLABORATIVE ERROR WHILE CALCULATING TARGET: ") + string(e.what())).c_str() ); }
						catch (...){ printf("UNKNOWN COLLABORATIVE COMMAND ERROR.\n"); }

						last_working_zone = "target";
					}//end-if angle-zone
				}
			}
			else
				cout << "[MQTT Error]: Motion command incorrect" << endl;
		}

		if( (MOOSStrCmp(m.GetKey(),"SHUTDOWN")) && (MOOSStrCmp(m.GetString(),"true")) )
		{
			MOOSTrace("Closing Module \n");
			this->RequestQuit();
		}
	}

    UpdateMOOSVariables(NewMail);
    return true;
}


void CRobotControllerApp::SetManualMode()
{
	//Set Manual mode
	//if (Robot_control_mode == 2)
	{
		Robot_control_mode = 0;
		//! @moos_publish ROBOT_CONTROL_MODE The Giraff working mode: 0=Manual=Pilot, 2=Autonomous=OpenMORA
		m_Comms.Notify("ROBOT_CONTROL_MODE", Robot_control_mode);
		if (verbose)
			cout << "[RobotController]: Control Mode set to (0) - MANUAL" << endl;
	}

	//Clear the destiny_node
	//! @moos_publish TOPOLOGICAL_DESTINY Name of the node where the reactive navigation has been planned.
	m_Comms.Notify("TOPOLOGICAL_DESTINY","NULL");
}


void CRobotControllerApp::SetAutonomousMode()
{
	//Set Autonomous mode
	if (Robot_control_mode == 0)
	{
		Robot_control_mode = 2;
		//! @moos_publish ROBOT_CONTROL_MODE The Giraff working mode: 0=Manual=Pilot, 2=Autonomous=OpenMORA
		m_Comms.Notify("ROBOT_CONTROL_MODE", Robot_control_mode);
		if (verbose)
			cout << "[RobotController]: Control Mode set to (2) - AUTO" << endl;
	}
}

void CRobotControllerApp::GoToRecharge()
{
	if( !going_to_docking )
	{
		printf("\n[RobotController]: GOING TO RECHARGE\n");
		// 1. CANCEL ALL current tasks and movements of the robot.
		//--------------------------------------------------------
		//! @moos_publish PNAVIGATORREACTIVEPTG3D_CMD Request the reactive3D to stop current navigation
		m_Comms.Notify("PNAVIGATORREACTIVEPTG3D_CMD", "CANCEL");
		//! @moos_publish CANCEL_IN_EXECUTION Cancel current Navigation Plans (Executor)
		m_Comms.Notify("CANCEL_IN_EXECUTION", 1.0);

		//Check if random navigation was enabled
		CMOOSVariable * pVar = GetMOOSVar("RANDOM_NAVIGATOR");
		if(pVar)
		{
			double random_navigator_mode = pVar->GetDoubleVal();
			if( random_navigator_mode == 1.0)
			{
				//TEMPORARILY disable the module until battery is recharged! Then reactivate the Random Navigation
				//! @moos_publish RANDOM_NAVIGATOR Activates/Deactivates the random navigator module
				m_Comms.Notify("RANDOM_NAVIGATOR", 2.0);
			}
		}
		else
		{
			//! @moos_publish RANDOM_NAVIGATOR Activates/Deactivates the random navigator module
			m_Comms.Notify("RANDOM_NAVIGATOR", 0.0);
		}

		// 2. GO TO NODE DOCKING AND ACTIVATE THE PARKING (autodocking)
		//---------------------------------------------------------------
		going_to_docking = true;
		//! @moos_publish NEW_TASK Request a new task to the Agenda.
		m_Comms.Notify("NEW_TASK", "ROBOT_CONTROLLER 505 GO_TO_RECHARGE -1");
	}
}


void CRobotControllerApp::CheckBattery(double battery_v)
{
	if( check_battery_status )
	{
		// Display value
		//---------------
		if( mrpt::system::timeDifference(last_bettery_display_time,mrpt::system::now())>5.0 )
		{
			//cout << "[Robot_Control_Manager]: Battery voltage is: " << battery_v << endl;
			last_bettery_display_time =  mrpt::system::now();
		}


		//Is completely charged?
		//----------------------
		if( Is_Charging==1.0 )
		{
			if( battery_v > battery_threshold_charged )
			{
				//! @moos_publish RECHARGE_COMPLETED (0=not finished, 1=finished)
				//m_Comms.Notify("RECHARGE_COMPLETED", 1.0);

				//Check if random navigations was temporarily disabled
				CMOOSVariable * pVar = GetMOOSVar("RANDOM_NAVIGATOR");
				if(pVar)
				{
					double random_navigator_mode = pVar->GetDoubleVal();
					if( random_navigator_mode==2.0 )
					{
						//Battery is ok. Reactivate Random Navigation Module
						//! @moos_publish NEW_TASK Request a new task to the Agenda.
						m_Comms.Notify("NEW_TASK", "ROBOT_CONTROLLER 1 UNDOCK");
						mrpt::system::sleep(2000);
						//! @moos_publish RANDOM_NAVIGATOR Activates/Deactivates the random navigator module
						m_Comms.Notify("RANDOM_NAVIGATOR", 1.0);
					}
				}
			}
		}
		// Check if low battery
		//-----------------------
		else if( Is_Charging==0.0 && !going_to_docking )
		{
			//Should I Recharge?
			if( battery_v <= battery_threshold_recharge )
			{
				// Take Control
				if( mrpt::system::timeDifference(last_bettery_warning_time,mrpt::system::now())>60.0 )
				{
					last_bettery_warning_time =  mrpt::system::now();

					//BATTERY IS TOO LOW - EMERGENCY!
					cout << endl << "******************************************************" << endl;
					cout << "[Robot_Control_Manager]: BATTERY voltage is too low!!!" << endl;
					cout << "                         Returning to Docking station." << endl;
					cout << "******************************************************" << endl << endl;
					//! @moos_publish NEW_TASK Request a new task to the Agenda.
					m_Comms.Notify("NEW_TASK", "ROBOT_CONTROLLER 1 SAY Nivel de batería muy bajo!!!\n Regresando a la estación de carga!.");
					//! @moos_publish ERROR_MSG A string containing the description of an Error.
					m_Comms.Notify("ERROR_MSG","[Robot_Control_Manager]: BATTERY voltage too low!!!\n Returning to Docking station.");

					//Command the robot to recharge its batteries
					GoToRecharge();
				}
			}
			else if( battery_v <= battery_threshold_warning )
			{
				// Generate Warning for the user every minute
				if( mrpt::system::timeDifference(last_bettery_warning_time,mrpt::system::now())>60.0 )
				{
					//cout << "******************************************************************"	<< endl;
					//cout << "[Robot_Control_Manager]: WARNING battery voltage is getting low!!!"	<< endl;
					//cout << "                         Please proceed to recharge station."		<< endl;
					//cout << "******************************************************************"	<< endl;
					//! @moos_publish NEW_TASK Request a new task to the Agenda.
					m_Comms.Notify("NEW_TASK", "ROBOT_CONTROLLER 1 SAY ATENCIÓN. Nivel de batería bajo. Por favor, regresa a la estación de carga.");
					//! @moos_publish ERROR_MSG A string containing the description of an Error.
					m_Comms.Notify("ERROR_MSG","[Robot_Control_Manager]: WARNING battery voltage is getting low!!!\n Please proceed to recharge station.");
					last_bettery_warning_time =  mrpt::system::now();
				}
			}
		}//end-if charging
	}
}
