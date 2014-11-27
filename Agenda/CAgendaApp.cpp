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

/**  @moos_module Module that keeps a list of the high-level tasks commanded to the robot.
  *  New tasks are received via the MOOSvariable "NEW_TASK"
  *  This module implements methods to create and delete tasks, 
  *  and to publish the list of current pending tasks to be scheduled by the Planner module.
  *
  *  Requiered modules: Planner, StateCharts and Executor.
  */

//!  @moos_var NEW_TASK Variable containing the necessary parameters to create a new task in the [Agenda] module.
//!  The structure of this variable is:  NEW_TASK "UserID UserTaskID Command Arguments"
//!		UserID (string)      Unique identifier of the user
//!		UserTaskID (size_t)  The local id of the task (from the user persepective)
//!		Command (char*)	  	 Command to execute
//!		Task (char*)	 	 Arguments of the Command


/**  @moos_ToDo
  *  Clean code. There are many methods that are not used in the TaskManager internal class.
  */

#include "CAgendaApp.hpp"
#include <mrpt/system/string_utils.h>

#include <sstream>
#include <iomanip>
#include <iostream>

using namespace std;
using namespace mrpt;
using namespace mrpt::utils;


CAgendaApp::CAgendaApp()
//	: var (init_val), ...
{
}

CAgendaApp::~CAgendaApp()
{
}

bool CAgendaApp::OnStartUp()
{
	// Read parameters (if any) from the mission configuration file.
	// There is also a MRPT-like object (this->m_ini) that is a wrapper
	// to read from the module config block of the current MOOS mission file.
	// m_ini.read_int(...);

	//! @moos_param Save_logfile Wheter or not to save a logFile with debug information
	Save_logfile = m_ini.read_bool("","Save_logfile", false, false);
	tm.set_SaveLogfile(Save_logfile);
	//Tell the TaskManager about the logfile.
	return DoRegistrations();
}

bool CAgendaApp::OnCommandMsg( CMOOSMsg Msg )
{
	if(Msg.IsSkewed(MOOSTime())) return true;
	if(!Msg.IsString()) return MOOSFail("This module only accepts string command messages\n");
	const std::string sCmd = Msg.GetString();
	//MOOSTrace("COMMAND RECEIVED: %s\n",sCmd.c_str());
	// Process the command "sCmd".

	return true;
}

bool CAgendaApp::Iterate()
{
	static bool first=true;
	std::string cad;

	//Check for pending tasks
	if (tm.IsEmpty() && first)
	{
		printf("Nothing to do...\n");
		first=false;
	}

	//If there are pending tasks, publish them to be planned by the Planner module
	if (!tm.IsEmpty())
	{
		std::string out;
		tm.PrintAgenda(out,false);
		//!  @moos_publish TASK_LIST
		//!  @moos_var TASK_LIST The list of pending tasks in the Agenda
		//!  The structure of this variable is: Task1#Task2#Task3...
		//!  where each task is defined by: "UserID  TaskID  UserTaskID  Command  Task"
		m_Comms.Notify("TASK_LIST", out);
		first=true;
	}

	return true;
}

bool CAgendaApp::OnConnectToServer()
{
	DoRegistrations();
	return true;
}


bool CAgendaApp::DoRegistrations()
{
	//! @moos_subscribe	NEW_TASK
	AddMOOSVariable("NEW_TASK","NEW_TASK","NEW_TASK",0);

	//! @moos_subscribe	TASK_PLANNED
	AddMOOSVariable("TASK_PLANNED","TASK_PLANNED","TASK_PLANNED",0);

	//! @moos_subscribe SHUTDOWN
	AddMOOSVariable( "SHUTDOWN", "SHUTDOWN", "SHUTDOWN", 0 );

	RegisterMOOSVariables();
	return true;
}


bool CAgendaApp::OnNewMail(MOOSMSG_LIST &NewMail)
{
	std::string cad;
	for(MOOSMSG_LIST::iterator i=NewMail.begin();i!=NewMail.end();++i)
	{
		// Create a new task		
		if (i->GetName()=="NEW_TASK")
		{
			// Parse the incomming string to obtain all necessary fields
			// NEW_TASK: "UserID   UserTaskID   Command   Parameters"
			std::deque<std::string> lista;
			mrpt::system::tokenize(i->GetString().c_str()," ",lista);			
			std::string goal;
			if( lista.size()<3 )
			{
				printf("[Agenda]ERROR when parsing NEW_TASK. Not all fields defined. Nothing Done.\n");
				printf("[Agenda] Wrong task was: NEW_TASK %s\n",i->GetString().c_str());
				printf("[Agenda] Correct format is: NEW_TASK UserID UserTaskID Command Parameters\n");
			}
			else
			{
				for (size_t j=3;j<lista.size();j++)
					goal=goal+lista[j]+" ";
				
				// Add task to list
				// ReceiveTask(std::string userid, size_t usertaskid, const char * command, const char * task, int priority, bool parallel)
				tm.ReceiveTask(lista[0], atoi(lista[1].c_str()), lista[2].c_str(), goal.c_str(), true);		
			}			
		}


		// Delete Task. Some task has been planned (see Planner module), delete it from list of pending tasks
		if (i->GetName()=="TASK_PLANNED")
		{
			// TASK_PLANNED: "TaskID"
			printf("Task Planned [%s]\n",i->GetString().c_str());
			
			//Delete task		
			if (!tm.DeleteTask(atoi(i->GetString().c_str())))
				printf("ERROR Task Not found when deleting\n");
		}


		if( (i->GetName()=="SHUTDOWN") && (MOOSStrCmp(i->GetString(),"true")) )
		{
			// Disconnect comms:			
			MOOSTrace("Closing Module \n");
			this->RequestQuit();
		}

	}

	UpdateMOOSVariables(NewMail);
	return true;
}
