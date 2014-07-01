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

		// CLIENT ACK
		//! @moos_param use_client_alive_ack Indicates (true/false) if the robot controller should periodically check the ACK sent by the MQTT client to cancel Navigation on error.
		use_client_alive_ack = m_ini.read_bool(section,"use_client_alive_ack","true",true);
		//! @moos_param max_client_ack_interval The maximum interval (seconds) between Client's ACK if use_client_alive_ack=true
		max_client_ack_interval = m_ini.read_double(section,"max_client_ack_interval",2,true);

		// BATTERY STATUS
		//! @moos_param check_battery_status Indicates (true/false) if the robot controller should periodically check the battery status and return to Docking when low.
		check_battery_status = m_ini.read_bool(section,"check_battery_status","true",true);

		mqtt_status = "";
		Is_Charging = 1.0;		//Start as charging (default behaviour)
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
		//If not charging check stuff
		if (Is_Charging==0.0)
		{
			//Check that MQTT is alive (related to Wifi connectivity) and that Client is connected
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

			//Check Batery status
			CMOOSVariable * pVarBattery = GetMOOSVar( "BATTERY_V" );
			if( check_battery_status && pVarBattery && pVarBattery->IsFresh() )
			{
				pVarBattery->SetFresh(false);
				CSerializablePtr obj;				
				mrpt::utils::RawStringToObject(pVarBattery->GetStringVal(),obj);
				
				if ( IS_CLASS(obj, CObservationBatteryState ))
				{
					mrpt::slam::CObservationBatteryStatePtr battery_obs = mrpt::slam::CObservationBatteryStatePtr(obj);
					if( battery_obs->voltageMainRobotBattery < 25.4)	//20% battery aprox
					{
						//BATTERY IS LOW -> Cancel and Recharge!
						cout << "[Robot_Control_Manager]: BATTERY voltage too low!!!\n Returning to Docking station." << endl;
						//! @moos_publish PNAVIGATORREACTIVEPTG3D_CMD Request the reactive3D to stop current navigation plan
						m_Comms.Notify("PNAVIGATORREACTIVEPTG3D_CMD", "CANCEL");
						//Command a Navigation to Docking node.
						//! @moos_publish GET_PATH Request a navigation to especified node
						m_Comms.Notify("GET_PATH", "Docking" );
					}					
				}
				else
					cout << "[Robot_Control_Manager]: ERROR: MOOS topic: BATERY_V is not CObservationBatteryStatePtr" << endl;
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
	
	//! @moos_subscribe PILOT_MQTT_ACK
    AddMOOSVariable( "PILOT_MQTT_ACK", "PILOT_MQTT_ACK", "PILOT_MQTT_ACK", 0);
	
	//! @moos_subscribe MQTT_CONNECT_STATUS
	AddMOOSVariable( "MQTT_CONNECT_STATUS", "MQTT_CONNECT_STATUS", "MQTT_CONNECT_STATUS", 0);

	//! @moos_subscribe Is_Charging
	AddMOOSVariable( "Is_Charging", "Is_Charging", "Is_Charging", 0);
	
	//! @moos_subscribe BATTERY_V
	AddMOOSVariable( "BATTERY_V", "BATTERY_V", "BATTERY_V", 0);

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


		if( MOOSStrCmp(m.GetKey(),"GO_TO_NODE") )
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

			//! @moos_publish NEW_TASK Request a new task to the Agenda.
			m_Comms.Notify("NEW_TASK", "ROBOT_CONTROLLER 0 MOVE "+m.GetString() );
		}

		// PLAN_FINISHED TaskID UserTaskID UserID
		if( MOOSStrCmp(m.GetKey(),"PLAN_FINISHED") )
		{
			std::deque<std::string> cad;
			mrpt::system::tokenize(m.GetString()," ",cad);			
			printf("[Robot_Control_Manager] PLAN_FINISHED event received: %s\n", m.GetString().c_str());
			//Was task created by the Controller?
			if( cad[2]=="ROBOT_CONTROLLER" && atoi(cad[1].c_str())==0)
			{
				//Navigation eneded: Set Manual mode
				if (Robot_control_mode == 2)
				{				
					Robot_control_mode = 0;
					//! @moos_publish ROBOT_CONTROL_MODE The Giraff working mode: 0=Manual=Pilot, 2=Autonomous=OpenMORA		
					m_Comms.Notify("ROBOT_CONTROL_MODE", Robot_control_mode);
					if (verbose)
						cout << "Control Mode set to (0) - MANUAL" << endl;
				}		
			}
		}
		

	    if (MOOSStrCmp(m.GetKey(),"NAVIGATE_TARGET"))
		{
			//New target set
			if (Robot_control_mode == 0)
			{
				//Set Autonomous mode
				Robot_control_mode = 2;
				//! @moos_publish ROBOT_CONTROL_MODE The Giraff working mode: 0=Manual=Pilot, 2=Autonomous=OpenMORA		
				m_Comms.Notify("ROBOT_CONTROL_MODE", Robot_control_mode);
				if (verbose)
					cout << "Control Mode set to (2) - AUTO" << endl;
			}
		}
		

		if (MOOSStrCmp(m.GetKey(),"CANCEL_NAVIGATION"))
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

				Robot_control_mode = 0;
				//! @moos_publish ROBOT_CONTROL_MODE The Giraff working mode: 0=Manual=Pilot, 2=Autonomous=OpenMORA		
				m_Comms.Notify("ROBOT_CONTROL_MODE", Robot_control_mode);

				cout << "[Robot_Control_Manager]: NAVIGATION CANCELLED" << endl;
				if (verbose)
						cout << "Control Mode set to (0) - MANUAL" << endl;
			}            
		}


		if (MOOSStrCmp(m.GetKey(),"PILOT_MQTT_ACK"))
		{
			//Update time of last ACK
			last_mqtt_ack_time = mrpt::system::now();
		}
		

		if (MOOSStrCmp(m.GetKey(),"Is_Charging"))
		{
			//Update charging status
			Is_Charging = m.GetDouble();
			cout << "[Robot_Control_Manager]: Giraff Charging status = " << Is_Charging << endl;
		}
		


		if( (MOOSStrCmp(m.GetKey(),"SHUTDOWN")) && (MOOSStrCmp(m.GetString(),"true")) )
		{
			this->RequestQuit();			
		}		
	}

    UpdateMOOSVariables(NewMail);
    return true;
}