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
  *  The module controls when the robot is to be teleoperated (external user commands via Pilot, WebRTC, etc.),
  *  or when the OpenMORA modules (ReactiveNavigation, NavigatorTopological, etc.) are in control.
  *  This module also implements some security procedures in case of errors or malfunctions, sucha as:
  *  - Stopping the robot and returning the control to manual mode
  *  - Returning to Docking when low batery is detected.
  *  - Returning to Docking when Client connection is lost.  
  */

#include "CRobotControllerApp.h"

#include <sstream>
#include <iomanip>
#include <iostream>
#include <mrpt/slam/CObservationBatteryState.h>
#include <mrpt/system.h>
#include <stdlib.h>     /* abs */


using namespace std;
using namespace mrpt;
using namespace mrpt::slam;
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
		printf("Loading Options from file...\n");
		std::string section = "";
		verbose = true;

		//! @moos_param working_mode Indicates (auto/onlyOpenMORA) if the robot controller should listen to the commands sent by an external client (pilot, webrtc, etc.), or not.
		working_mode = m_ini.read_string(section,"working_mode","auto",true);
		cout << "working_mode = " << working_mode << endl;

		if( MOOSStrCmp(working_mode,"onlyOpenMORA") )
		{
			//Start robot in autonomous mode (0 = Manual=Pilot, 2=Autonomous=OpenMORA)			
			Robot_control_mode = 2;			
		}
		else
		{
			//Allow changing from Pilot to OpenMORA as requested (0 = Manual=Pilot, 2=Autonomous=OpenMORA)
			//By default, start in Manual Mode 
			Robot_control_mode = 0;
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

		//! @moos_param battery_threshold_warning Battery level that once is reached will generate a user warning
		battery_threshold_warning = m_ini.read_float(section,"battery_threshold_warning",26.0,false);

		//! @moos_param battery_threshold_recharge Battery level that once is reached will cause the robot to go Recharge
		battery_threshold_recharge = m_ini.read_float(section,"battery_threshold_recharge",25.0,false);

		//! @moos_param battery_threshold_charged Battery level corresponding to a complete charge
		battery_threshold_charged = m_ini.read_float(section,"battery_threshold_charged",29.0,false);

		mqtt_status = "";
		Is_Charging = 1.0;		//Start as charging (default behaviour)
		last_bettery_display_time = mrpt::system::now();
		last_bettery_warning_time = mrpt::system::now();
		going_to_docking = false;
		last_working_zone = "NULL";
		printf("[RobotController]: Initial configuration done.\n");
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
		// Check Batery status
		//---------------------
		CMOOSVariable * pVarBattery = GetMOOSVar( "BATTERY_V" );
		if( pVarBattery && pVarBattery->IsFresh() )
		{
			pVarBattery->SetFresh(false);
			CSerializablePtr obj;
			mrpt::utils::RawStringToObject(pVarBattery->GetStringVal(),obj);
				
			if( IS_CLASS(obj, CObservationBatteryState) )
			{
				mrpt::slam::CObservationBatteryStatePtr battery_obs = mrpt::slam::CObservationBatteryStatePtr(obj);
				
				// Display value
				if( mrpt::system::timeDifference(last_bettery_display_time,mrpt::system::now())>5.0 )
				{
					//cout << "[Robot_Control_Manager]: Battery voltage is: " << battery_obs->voltageMainRobotBattery << endl;
					last_bettery_display_time =  mrpt::system::now();
				}

				//Is completely charged?
				if( Is_Charging==1.0 )
				{
					if (battery_obs->voltageMainRobotBattery > battery_threshold_charged)
					{
						//! @moos_publish RECHARGE_COMPLETED (0=not finished, 1=finished)
						m_Comms.Notify("RECHARGE_COMPLETED", 1.0);
						//printf("[Robot_Control_Manager]: Notified Recharge_Completed!\n");
					}					
				}

				// Check if low battery
				else if( Is_Charging==0.0 && !going_to_docking && check_battery_status )
				{
					//Recharge?
					if( battery_obs->voltageMainRobotBattery <= battery_threshold_recharge )
					{
						// Take Control
						if( mrpt::system::timeDifference(last_bettery_warning_time,mrpt::system::now())>60.0 )
						{
							last_bettery_warning_time =  mrpt::system::now();

							//BATTERY IS TOO LOW - EMERGENCY!
							cout << "******************************************************" << endl;
							cout << "[Robot_Control_Manager]: BATTERY voltage too low!!!"	 << endl;
							cout << "                         Returning to Docking station." << endl;
							cout << "******************************************************" << endl;
							//! @moos_publish NEW_TASK Request a new task to the Agenda.
							m_Comms.Notify("NEW_TASK", "ROBOT_CONTROLLER 1 SAY BATTERY voltage too low!!!\n Returning to Docking station.");
							//! @moos_publish ERROR_MSG A string containing the description of an Error.
							m_Comms.Notify("ERROR_MSG","[Robot_Control_Manager]: BATTERY voltage too low!!!\n Returning to Docking station.");

							//Command the robot to recharge its batteries
							GoToRecharge();							
						}
						//else
						//	 cout << "tdif = "<< mrpt::system::timeDifference(last_bettery_warning_time,mrpt::system::now()) << endl;
						
					}
					else if( battery_obs->voltageMainRobotBattery <= battery_threshold_warning )
					{
						// Generate Warning for the user every minute
						if( mrpt::system::timeDifference(last_bettery_warning_time,mrpt::system::now())>60.0 )
						{
							cout << "******************************************************************"	<< endl;
							cout << "[Robot_Control_Manager]: WARNING battery voltage is getting low!!!"	<< endl;
							cout << "                         Please proceed to recharge station."			<< endl;
							cout << "******************************************************************"	<< endl;
							//! @moos_publish NEW_TASK Request a new task to the Agenda.
							m_Comms.Notify("NEW_TASK", "ROBOT_CONTROLLER 1 SAY Warning. Battery is getting low. Please take me to the docking station.");
							//! @moos_publish ERROR_MSG A string containing the description of an Error.
							m_Comms.Notify("ERROR_MSG","[Robot_Control_Manager]: WARNING battery voltage is getting low!!!\n Please proceed to recharge station.");
							last_bettery_warning_time =  mrpt::system::now();
						}
						//else
						//	 cout << "tdif = "<< mrpt::system::timeDifference(last_bettery_warning_time,mrpt::system::now()) << endl;
					}	
				}
			}
			else
				cout << "[Robot_Control_Manager]: ERROR: MOOS topic: BATERY_V is not CObservationBatteryStatePtr" << endl;
		}
		

		//Check that MQTT is alive (related to Wifi connectivity) and that Client is connected
		if (Is_Charging==0.0)
		{			
			CMOOSVariable * pVarMQTT = GetMOOSVar( "MQTT_CONNECT_STATUS" );		
			if( pVarMQTT )
			{
				//Cout any change in the MQTT status
				if( !MOOSStrCmp(mqtt_status,pVarMQTT->GetStringVal()) )
				{
					mqtt_status = pVarMQTT->GetStringVal();
					cout << "[Robot_Control_Manager]: MQTT status is: " << pVarMQTT->GetStringVal() << endl;
				}
			
				if( use_client_alive_ack && mrpt::system::timeDifference(last_mqtt_ack_time,mrpt::system::now()) > max_client_ack_interval)
				{				
					if( Robot_control_mode == 2 )
					{	
						//Finish current navigation
						//Do nothing, just wait navigation to end --> control_mode = 0
					}	
					else
					{
						//Problem detected! Communications not working or client disconnected
						if ( MOOSStrCmp(pVarMQTT->GetStringVal(),"OFFLINE"))
							cout << "[Robot_Control_Manager]: Error detected in the communications with MQTT.\n Returning to Docking station." << endl;
						else						
							cout << "[Robot_Control_Manager]: Error detected in the communications with Client.\n ACK not received.\n Returning to Docking station." << endl;

						//! @moos_publish PNAVIGATORREACTIVEPTG3D_CMD Request the reactive3D to stop current navigation plan
						m_Comms.Notify("PNAVIGATORREACTIVEPTG3D_CMD", "CANCEL");
						//Command a Navigation to Docking node.
						//! @moos_publish GET_PATH Request a navigation to especified node
						m_Comms.Notify("GET_PATH", "Docking" );
					}
				}
			}
			else
				cout << "[Robot_Control_Manager]: ERROR - MQTT status UNKNOWN" << endl;			
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
	
	//! @moos_subscribe PILOT_MQTT_ACK
    AddMOOSVariable( "PILOT_MQTT_ACK", "PILOT_MQTT_ACK", "PILOT_MQTT_ACK", 0);
	
	//! @moos_subscribe MQTT_CONNECT_STATUS
	AddMOOSVariable( "MQTT_CONNECT_STATUS", "MQTT_CONNECT_STATUS", "MQTT_CONNECT_STATUS", 0);

	//! @moos_subscribe Is_Charging
	AddMOOSVariable( "Is_Charging", "Is_Charging", "Is_Charging", 0);

	//! @moos_subscribe GO_TO_RECHARGE
	AddMOOSVariable( "GO_TO_RECHARGE", "GO_TO_RECHARGE", "GO_TO_RECHARGE", 0);
	
	//! @moos_subscribe BATTERY_V
	AddMOOSVariable( "BATTERY_V", "BATTERY_V", "BATTERY_V", 0);

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
			printf("[Robot_Control_Manager] PLAN_FINISHED event received: %s\n", m.GetString().c_str());
			
			//End of reactive navigation
			if( cad[2]=="ROBOT_CONTROLLER" && atoi(cad[1].c_str())==0)
			{
				//Navigation eneded: Set Manual mode
				SetManualMode();
			}

			//end of GO_TO_RECHARGE (battery is now fully recharged)
			else if( cad[2]=="ROBOT_CONTROLLER" && atoi(cad[1].c_str())==505)
			{
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

				//Navigation to Docking node ended: Activate AutoDocking module!
				//! @moos_publish PARKING Variable that indicates when the robot should start the Docking process.
				//m_Comms.Notify("PARKING", 1.0);
				//if (verbose)
				//	cout << "Activating AutoDocking Module" << endl;				
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
			if (Robot_control_mode == 2 && !MOOSStrCmp(working_mode,"onlyOpenMORA") )
			{
				//STOP THE ROBOT & Set Manual mode
				//---------------------------------				
				//! @moos_publish PNAVIGATORREACTIVEPTG3D_CMD Request the reactive3D to stop current navigation
				m_Comms.Notify("PNAVIGATORREACTIVEPTG3D_CMD", "CANCEL");
				//! @moos_publish RANDOM_NAVIGATOR Activates/Deactivates the random navigator module
				m_Comms.Notify("RANDOM_NAVIGATOR", 0.0);
				//! @moos_publish CANCEL_IN_EXECUTION Cancel current Navigation Plans (Executor)
				m_Comms.Notify("CANCEL_IN_EXECUTION", 1.0);
				//! @moos_publish PARKING Variable that indicates when the robot should start the Docking process.
				m_Comms.Notify("PARKING", 0.0);
				going_to_docking = false;

				SetManualMode();
				cout << "[Robot_Control_Manager]: NAVIGATION CANCELLED" << endl;				
			}            
		}


		if( MOOSStrCmp(m.GetKey(),"PILOT_MQTT_ACK") )
		{
			//Update time of last ACK
			last_mqtt_ack_time = mrpt::system::now();
		}
		

		if( MOOSStrCmp(m.GetKey(),"Is_Charging") )
		{
			//Update charging status
			Is_Charging = m.GetDouble();
			cout << "[Robot_Control_Manager]: Giraff Charging status changed to: " << Is_Charging << endl;
			if( Is_Charging == 1.0 )
				going_to_docking = false; //Already there
		}
		

		if( MOOSStrCmp(m.GetKey(),"PARKING") )
		{
			//Someone requested to de/activate the Autodocking assistant
			double parking_status = m.GetDouble();
			if (parking_status == 1.0)
			{
				printf("[Robot_Control_Manager]: Request to activate Autodocking assistant!\n");
				SetAutonomousMode();				
			}
			else
			{
				printf("[Robot_Control_Manager]: Request to deactivate Autodocking assistant!\n");
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
				cout << "[Robot_Control_Manager] ERROR: Motion command incorrect" << endl;
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
						initial_collaborative_delay = ref - static_cast<unsigned __int64>(_atoi64(list.at(1).c_str())) ;
						printf("[Robot_Control_Manager]: Collaborative control ACTIVATED.\n");
					}
					catch (std::exception &e) {	return MOOSFail( (string("**COLLABORATIVE START ERROR: ") + string(e.what())).c_str() ); }
					catch (...)	{ printf("UNKNOWN COLLABORATIVE START ERROR.\n"); }
				}
				else
				{
					//! @moos_publish COLLABORATIVE Indicates if collaborative control is requested by the user
					m_Comms.Notify("COLLABORATIVE", 0.0);
					printf("[Robot_Control_Manager]: Collaborative control DEACTIVATED.\n");
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
					current_delay = ref - static_cast<unsigned __int64>(_atoi64(list.at(2).c_str()));
				}
				catch (std::exception &e){ return MOOSFail( (string("**COLLABORATIVE COMMAND ERROR**") + string(e.what())).c_str() ); }				
				catch (...){ printf("UNKNOWN COLLABORATIVE COMMAND ERROR.\n"); }

				if (current_delay > initial_collaborative_delay+ 200)
				{
					printf("[MQTTMosquitto:Collaborative]:ERROR Collaborative command TO OLD. Discarding...\n");
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
							m_Comms.Notify("CANCEL_NAVIGATION", 1.0);
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
							m_Comms.Notify("CANCEL_NAVIGATION", 1.0);
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
			this->RequestQuit();			
		}		
	}

    UpdateMOOSVariables(NewMail);
    return true;
}


void CRobotControllerApp::SetManualMode()
{
	//Set Manual mode
	if (Robot_control_mode == 2)
	{
		Robot_control_mode = 0;
		//! @moos_publish ROBOT_CONTROL_MODE The Giraff working mode: 0=Manual=Pilot, 2=Autonomous=OpenMORA		
		m_Comms.Notify("ROBOT_CONTROL_MODE", Robot_control_mode);
		if (verbose)
			cout << "Control Mode set to (0) - MANUAL" << endl;
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
			cout << "Control Mode set to (2) - AUTO" << endl;
	}
}

void CRobotControllerApp::GoToRecharge()
{
	if (going_to_docking = false)
	{
		printf("[RobotController]: \n\nRequested GoToRecharge\n\n\n");
		// 1. CANCEL ALL current tasks and movements of the robot.
		//--------------------------------------------------------
		//! @moos_publish PNAVIGATORREACTIVEPTG3D_CMD Request the reactive3D to stop current navigation
		m_Comms.Notify("PNAVIGATORREACTIVEPTG3D_CMD", "CANCEL");							
		//! @moos_publish CANCEL_IN_EXECUTION Cancel current Navigation Plans (Executor)
		m_Comms.Notify("CANCEL_IN_EXECUTION", 1.0);

		//Check if random navigations was enabled
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
		//m_Comms.Notify("NEW_TASK", "ROBOT_CONTROLLER 505 MOVE Docking");
		m_Comms.Notify("NEW_TASK", "ROBOT_CONTROLLER 505 GO_TO_RECHARGE -1");
		//printf("[RobotController]: NEW_TASK GO_TO_RECHARGE\n");
	}
	else
		printf("[RobotController]: Requested GoToRecharge, but already on the process.\n");
}