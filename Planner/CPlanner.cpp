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

/**  @moos_module A module that schedules complex (high level) tasks into a sequence of small, simple routines.
  *  This module obtain a list of pending tasks from the Agenda module and tries to schedule them according to compatibility
  *  and priority values. Each task is planned in a new thread which is closed when the task finished.
  *  Each task is decomposed ina list of actions (called plan), and send to the Executor module.
  *
  *  Requiered modules: Agenda, StateCharts and Executor.
  */

/** @moos_ToDo  
  *  Consider changin the way "Actions" are defined. ACTION = compatibility.
  *  Method IsCompatible() Is not implemented!! It alwasy return TRUE!!
  *  Method PathSearch(): Why arguments "plan" and "execute"? Also, this module MOVES the robot, so a better name would be "GoToNodeLabel"
  *  HPWA line 1059: 		ec.timestamp = taskid;		//JGMonroy: timestamp = taskID ????
  */

#include "CPlannerApp.hpp"
#include <sstream>
#include <iomanip>
#include <iostream>

using namespace std;
using namespace mrpt;
using namespace mrpt::utils;



CPlannerApp::CPlannerApp():planner(m_Comms_delayed)
//	: var (init_val), ...
{
}

CPlannerApp::~CPlannerApp()
{
}

bool CPlannerApp::OnStartUp()
{
	// Read parameters (if any) from the mission configuration file.
	
	//! @moos_param   num_actions
	m_MissionReader.GetConfigurationParam("num_actions",num_actions);
	
	//! @moos_param   actions_names The names of the Actions to load
	m_MissionReader.GetConfigurationParam("actions_names",actions_names);

	// Get compatibility list between actions
	std::vector<std::string> comp_list;

    printf("Loading %d actions from %s\n",num_actions,actions_names.c_str());
	for (size_t i=0;i<num_actions;i++)
	{
		std::string name;
		name=format("%s%u","action",(unsigned int)i);
		printf("Loading action %s\n",name.c_str());
		std::string value;
		if (!m_MissionReader.GetConfigurationParam(name,value)) printf("Error while retrieving %s\n",name.c_str());
		comp_list.push_back(value);
	}

	//Initialize Planner
	planner.HPWAInit(actions_names,comp_list);
	
	return DoRegistrations();
}

bool CPlannerApp::OnCommandMsg( CMOOSMsg Msg )
{
	if(Msg.IsSkewed(MOOSTime())) return true;
	if(!Msg.IsString()) return MOOSFail("This module only accepts string command messages\n");
	const std::string sCmd = Msg.GetString();
	//MOOSTrace("COMMAND RECEIVED: %s\n",sCmd.c_str());
	// Process the command "sCmd".

	return true;
}

bool CPlannerApp::Iterate()
{
	IterateBase();

	// Are there	
	CMOOSVariable *PLAN_FINISHED = GetMOOSVar("PLAN_FINISHED");
	if (PLAN_FINISHED && PLAN_FINISHED->IsFresh())
	{
			PLAN_FINISHED->SetFresh(false);
			planner.FinishedCommand(atoi(PLAN_FINISHED->GetStringVal().c_str()));
	}


	// PETITIONS
	// Check for pending petitions: "asked_petitions" list.
	// Petitions are requests of information to other modules via OpenMORA variables.
	//-----------------------------------------------------------------------------------
	if (planner.PendingPetitions())
	{
		planner.pet_sem.enter();

		//Get first pending petition (petID, what)
		std::string petition = planner.pet_map.begin()->second;		//The string with the "what" to perform. ej "GET_PATH nX"		
		std::deque<std::string> cad;
		mrpt::system::tokenize(petition," ",cad);

		//printf("notifying %s %d %s\n",cad[0].c_str(),planner.pet_map.begin()->first,cad[1].c_str());

		//! @moos_publish <OpenMORA_VAR> This variable could be any OpenMORA variable. It is the result of a scheduled task. The first parameter is always the petitionID.
		// ej. Notify(GET_PATH,"petitionID nX")
		// ej. Notify(GET_PERSON_LOCATION, "petitionID userID")
		m_Comms.Notify(cad[0], format("%d %s", (int)planner.pet_map.begin()->first, cad[1].c_str()));

		//Remove petition from pending list.
		planner.asked_petitions.erase(planner.pet_map.begin()->first);
		
		planner.pet_sem.leave();
	}


	// PATH
	// Check if any PATH petition has been answered by the WorldModel	
	// Path = "petitionID NOTFOUND"  (If node or path not found in the topology)
	// Path = "petitionID (nodeID1 nodeLabel1 nodeX1 nodeY1) (nodeID2 nodeLanel2 nodeX2 nodeY2)..."	
	//-------------------------------------------------------------------------------------------
	CMOOSVariable *path = GetMOOSVar("PATH");
	if (path && path->IsFresh())
	{
		path->SetFresh(false);
		printf("[%s]\n",path->GetStringVal().c_str());

		planner.pet_sem.enter();			
			std::deque<std::string> cad;
			mrpt::system::tokenize(path->GetStringVal()," ",cad);
			// remove the petitionID identifier from response
			std::string response = path->GetStringVal().substr((size_t)1 + path->GetStringVal().find(" "));			
			// answer_map(petitionID, response)
			planner.answer_map.insert( pet_pair(atoi(cad[0].c_str()),response) );
		planner.pet_sem.leave();
	}


	// PERSON_LOCATION
	// Check if any PERSON_LOCATION request has been answered by WorldModel
	// PERSON_LOCATION "petitionID NOTFOUND"
	// PERSON_LOCATION "petitoinID location_node_label"
	//-----------------------------------------------------------------------
	CMOOSVariable *person_location = GetMOOSVar("PERSON_LOCATION");
	if (person_location && person_location->IsFresh())
	{
		person_location->SetFresh(false);
		printf("%s\n",person_location->GetStringVal().c_str());

		planner.pet_sem.enter();
			std::deque<std::string> cad;
			mrpt::system::tokenize(person_location->GetStringVal()," ",cad);
			// remove the petitionID identifier from response
			std::string response = person_location->GetStringVal().substr((size_t)1 + person_location->GetStringVal().find(" "));
			// answer_map(petitionID, response)
			planner.answer_map.insert( pet_pair(atoi(cad[0].c_str()),response) );
		planner.pet_sem.leave();
	}

	return true;
}

bool CPlannerApp::OnConnectToServer()
{
	DoRegistrations();
	return true;
}


bool CPlannerApp::DoRegistrations()
{
	//! @moos_subscribe TASK_LIST
	AddMOOSVariable("TASK_LIST","TASK_LIST","TASK_LIST",0);
	
	//! @moos_subscribe PATH
	AddMOOSVariable("PATH","PATH","PATH",0);
	
	//! @moos_subscribe PERSON_LOCATION
	AddMOOSVariable("PERSON_LOCATION","PERSON_LOCATION","PERSON_LOCATION",0);
	
	//! @moos_subscribe PLAN_FINISHED
	AddMOOSVariable("PLAN_FINISHED","PLAN_FINISHED","PLAN_FINISHED",0);

	//! @moos_subscribe SHUTDOWN
	AddMOOSVariable( "SHUTDOWN", "SHUTDOWN", "SHUTDOWN", 0 );

	RegisterMOOSVariables();
	return true;
}

//----------------------------------------------------------------
// Plan()
// This method Schedule(Plan) the task paased in the string goal
// Goal = "userid taskid localtaskid command task"
// This method should run in a different Thread for each call
//-----------------------------------------------------------------
void CPlannerApp::Plan(std::string goal)
{
	//Parse String and get Task parameters
	std::deque<std::string> cad;
	mrpt::system::tokenize(goal," ",cad);
	printf("Planning thread %s\n",goal.c_str());

	// Check the Task parameters
	if (cad.size()>=5)			//Task has parameters
	{
		std::string params;	
		for (size_t j=4;j<cad.size();j++)
		{	
			params.append(cad[j]);
			params.append(" ");		
		}			
		planner.SolveTask(cad[0], atoi(cad[1].c_str()), atoi(cad[2].c_str()), cad[3], params);
	}
	else if (cad.size()==4)		//No parameters
		planner.SolveTask(cad[0], atoi(cad[1].c_str()), atoi(cad[2].c_str()), cad[3], "");

    else printf("[Planner:Plan]Error while parsing task\n");
}



bool CPlannerApp::OnNewMail(MOOSMSG_LIST &NewMail)
{
	UpdateMOOSVariables(NewMail);

	for(MOOSMSG_LIST::iterator i=NewMail.begin();i!=NewMail.end();++i)
	{
		// Get list of tasks in the Agenda and try scheduling them.
		if (i->GetName()=="TASK_LIST")
		{
			// TASK_LIST : Task1#Task2#Task3...
			// where each task is defined by: "UserID  TaskID  UserTaskID  Command  Task"
			
			//printf("Received from task_list [%s]\n",i->GetAsString().c_str());
			std::deque<std::string> lista;
			mrpt::system::tokenize(i->GetString().c_str(),"#",lista);

			//Run the list of pending tasks and if possible Schedule them
			//-------------------------------------------------------------
			std::string command,task,address;
			size_t taskid,localtaskid;
			//Run it N times, where N is the number of pending tasks
			for (size_t j=0;j<lista.size();j++)
			{
				//Check Compatibility of ALL pending tasks. If any task is compatible then Schedule it.
				bool tasktosolve = planner.SelectTasktobePlanned(i->GetAsString(), command, task, taskid, address, localtaskid);

				//IF any task has been scheduled
				if (taskid!=size_t(-1) && tasktosolve)
				{
					//Notify the taskID that has been planned
					std::string goal = address+" "+format ("%d",(int)taskid)+" "+format ("%d",(int)localtaskid)+" "+command+" "+task;
					printf("Planning %s\n",goal.c_str());

					//Notify the Agenda that some task have been planned
					//!  @moos_publish TASK_PLANNED The Id of the task that has just been planned (to inform the Agenda)
					m_Comms.Notify("TASK_PLANNED", format("%d",(int)taskid));

					//Schedule the task in a new thread!
					mrpt::system::createThreadFromObjectMethod(this,&CPlannerApp::Plan,goal);
				}
				else if (taskid==size_t(-1))
				{
					printf("Esa tarea no la puedo hacer ahora mismo...\n");
				}
			}
		}


		if( (i->GetName()=="SHUTDOWN") && (MOOSStrCmp(i->GetString(),"true")) )
		{
			// Disconnect comms:
			MOOSTrace("Closing Module \n");
			this->RequestQuit();
		}

	}
	return true;
}
