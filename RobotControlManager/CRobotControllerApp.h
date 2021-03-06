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

#ifndef CRobotControllerApp_H
#define CRobotControllerApp_H


#include <COpenMORAMOOSApp.h>

class CRobotControllerApp : public COpenMORAApp
{
public:
    CRobotControllerApp();
    virtual ~CRobotControllerApp();

protected:
    /** called at startup */
    virtual bool OnStartUp();
    /** called when new mail arrives */
    virtual bool OnNewMail(MOOSMSG_LIST & NewMail);
    /** called when work is to be done */
    virtual bool Iterate();
    /** called when app connects to DB */
    virtual bool OnConnectToServer();

	bool OnCommandMsg( CMOOSMsg Msg );

    /** performs the registration for mail */
    bool DoRegistrations();

	void SetManualMode();
	void SetAutonomousMode();
	void GoToRecharge();
	void CheckBattery(double battery_v);


	//Data
	size_t Robot_control_mode;							// The Opreation mode of the robot. 0=Manual, 2=Autonomous=(OpenMORA)
	mrpt::system::TTimeStamp last_mqtt_ack_time;		// Stores the time of the last ACK from the Pilot-MQTT client
	mrpt::system::TTimeStamp last_bettery_display_time; // Stores the time of the last battery display
	mrpt::system::TTimeStamp last_bettery_warning_time; // Stores the time of the last battery warning
	bool use_client_alive_ack;							// Indicates wheter this module should check the status of the ACK from Client (Ture/false)
	bool check_mqtt_alive;								// Indicates wheter this module should check the status of MQTT (Ture/false)
	bool check_battery_status;							// Indicates wheter this module should check the status of Battery (Ture/false)
	bool battery_base;									// Indicates true=Monitor battery from the robot base, false=monitor external battery (see BatteryManager module)
	float battery_threshold_warning, battery_threshold_recharge, battery_threshold_charged; // Battery lvl (Volts) that will cause the robot to go recharge or generate a warning
	double max_client_ack_interval;						// Max number of seconds between ACK from the Client to set it is alive
	bool verbose;
	std::string working_mode, mqtt_status, client_connected;
	float Is_Charging;
	bool going_to_docking;
	uint64_t initial_collaborative_delay;
	std::string last_working_zone;
	float collaborative_pure_angle_th;
	float collaborative_max_target_distance;
	float collaborative_max_angular_speed;
};

#endif
