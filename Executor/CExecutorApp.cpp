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

/**  @moos_module Module that executes "plans", that is, sequence of actions as result of a scheduled task.
  *  New plans are received through the OpenMORA variable IN_EXECUTION.
  *  
  *  Requiered modules: Agenda, StateCharts and Planner.
  */

//!  @moos_var IN_EXECUTION Variable published each time a new task is scheduled by the planner, to inform the Executor module.
//!  It contains all the necessary parameters defining the task itself and the corresponding actions.
//!  FORMAT: IN_EXECUTION taskowner#taskid#localtaskid#listOfActions
//!  where listOfActions = ACTION_NAME_1 param1_1 param1_2 .. param1_i#ACTION_NAME_2 param2_1 param2_2 .. param2_i

/** @moos_ToDo
  *  Change the way "flags" of subscribed signals are activated. 
  *  Currently, each iteration checks all subscribed vbles even if there are no flags waiting for a response (see Iterate).
  *  It would be more efficient to check only subscribed vbles when there are "flags" waiting
  *
  *  Remove executed plan from the list of currently executing plans!!
  */


#include "CExecutorApp.hpp"
#include <mrpt/system/threads.h>

#include <sstream>
#include <iomanip>
#include <iostream>

using namespace std;
using namespace mrpt;
using namespace mrpt::utils;



CExecutorApp::CExecutorApp():te(m_Comms_delayed)
{
}

CExecutorApp::~CExecutorApp()
{
}

bool CExecutorApp::OnStartUp()
{
	// LOAD JOKES
	//--------------
	//! @moos_param   num_jokes   The number of "jokes" or phrases tha can be played with the Task SAY_JOKES
	const uint64_t num_jokes   = m_ini.read_uint64_t("pExecutor","num_jokes",0);

	//! @moos_param   jokes[i]   Each of the "jokes" or phrases tha can be played with the Task SAY_JOKES
	std::vector<std::string> joke_list;
    printf("Loading %d jokes\n",(int)num_jokes);
	for (unsigned int i=1;i<=num_jokes;i++)
	{
		std::string name;
		name = format("%s%u","joke",i);		
		std::string value;
		if (!m_MissionReader.GetConfigurationParam(name,value)) printf("Error while retrieving %s\n",name.c_str());
		joke_list.push_back(value);
	}
	te.LoadJokes(joke_list);
	
	// LOAD VARIABLE NAMES TO SUBSCRIBE
	//--------------------------------------
	//! @moos_param   num_vbles   The number of variables that this module should subscribe to
	const uint64_t num_vbles   = m_ini.read_uint64_t("pExecutor","num_vbles",0);

	//! @moos_param   vble[i]   Each of the "variable names" that this module should subscribe to
	//! This variables are requested by actions such as WAIT_FOR_SIGNAL
	printf("Loading %d vbles\n",(int)num_vbles);
	for (unsigned int i=1;i<=num_vbles;i++)
	{
		std::string name;
		name=format("%s%u","vble",i);
		
		std::string value;
		if (!m_MissionReader.GetConfigurationParam(name,value)) printf("Error while retrieving %s\n",name.c_str());
		printf("Loading %s=%s\n",name.c_str(),value.c_str());
		vble_list.push_back(value);
	}

	//! @moos_param   dir   The mission directory. Used to play sounds with the PLAY Action
	m_MissionReader.GetConfigurationParam("dir",te.mission_directory);
	cout << "Mission directory " << te.mission_directory << endl;

	return DoRegistrations();
}

bool CExecutorApp::OnCommandMsg( CMOOSMsg Msg )
{
	if(Msg.IsSkewed(MOOSTime())) return true;
	if(!Msg.IsString()) return MOOSFail("This module only accepts string command messages\n");
	const std::string sCmd = Msg.GetString();
	//MOOSTrace("COMMAND RECEIVED: %s\n",sCmd.c_str());
	// Process the command "sCmd".

	return true;
}

bool CExecutorApp::Iterate()
{
	IterateBase();

	// Action WAIT_FOR_SIGNAL(signal_name)
	// Check the "flags" of subscribed variables. 
	// If value>0, then active its corresponding "signal" 
	for (int i=0;i<vble_list.size();i++)
	{
		CMOOSVariable *v = GetMOOSVar(vble_list[i].c_str());

		if (v && v->IsFresh())
		{
			//printf("Checking signal %s(%f)\n",v->GetName().c_str(),v->GetDoubleVal());
			v->SetFresh(false);
			std::map<std::string,bool>::iterator pet_it;
			pet_it = te.signal_map.find(vble_list[i].c_str());
			//If we are waiting for a signal of this vble, then activate/deactivate it
			if (pet_it!=te.signal_map.end())
			{
				if (atof(v->GetAsString().c_str())>0.0) 
				{
					printf("Activating signal %s\n",v->GetName().c_str());
					pet_it->second=true;
				}
				else 
					pet_it->second=false;
			}
		}
	}


	// Check if current NAVIGATION has ended successfully (ReactiveNavigation, LookAtPoint)
	// Only one navigation action can be carried out simultaniously
	CMOOSVariable *navigate_end = GetMOOSVar("NAV_EVENT_END");
	if (navigate_end && navigate_end->IsFresh())
	{
		navigate_end->SetFresh(false);
		te.EndNavigation();
	}

	
	// Check if current NAVIGATION has ended with error (ReactiveNavigation, LookAtPoint)
	// Only one navigation action can be carried out simultaniously
	CMOOSVariable *navigate_error = GetMOOSVar("NAV_EVENT_ERROR");
	if (navigate_error && navigate_error->IsFresh())
	{
		navigate_error->SetFresh(false);
		te.FailedNavigation();
	}


	// Check if Voice action has finished.
	// Only one "talk" action can be carried out simultaniously
	CMOOSVariable *speak_end = GetMOOSVar("VOICE_EVENT_DONE");
	if (speak_end && speak_end->IsFresh())
	{
		speak_end->SetFresh(false);
		te.EndSpeak();
	}


	// Check if response to GET_NODE_POSITION has been received
	CMOOSVariable *NODE_POSITION = GetMOOSVar("NODE_POSITION");
	if (NODE_POSITION && NODE_POSITION->IsFresh())
	{
		NODE_POSITION->SetFresh(false);
		te.pet_sem.enter();
			std::deque<std::string> lista;
			//NODE_POSITION petitionID node_label x y
			//NODE_POSITION petitionID NOTFOUND
			mrpt::system::tokenize(NODE_POSITION->GetStringVal()," ",lista);
			// remove the petitionID identifier from response
			std::string response = NODE_POSITION->GetStringVal().substr((size_t)1 + NODE_POSITION->GetStringVal().find(" "));
			// answer_map(petitionID, response)
			te.answer_map.insert( pet_pair(atoi(lista[0].c_str()),  response) );
		te.pet_sem.leave();
	}


	// PETITIONS
	// Check for pending petitions
	// Petitions are requests of information to other modules via OpenMORA variables.
	//-----------------------------------------------------------------------------------
	if (te.PendingPetitions())
	{
		te.pet_sem.enter();
			//Get first pending petition (petID, what)
			std::string petition = te.pet_map.begin()->second;
			printf("[Excutor:Iterate]Pending petition: %s\n",petition.c_str());
			std::deque<std::string> lista;
			mrpt::system::tokenize(petition," ",lista);
			//Notify VBLE_NAME petitionID parameter
			m_Comms.Notify( lista[0], format("%lu %s",te.pet_map.begin()->first,lista[1].c_str()) );
			te.asked_petitions.erase(te.pet_map.begin()->first);
		te.pet_sem.leave();
	}


	//PLANS
	//Check if there are plans not yet executed
	if (te.lplans.size()>0)
	{
		//Execute the first pending plan (queue)
		printf("[Executing Plan] %u \n",(unsigned int)te.lplans[0].timestamp);
		TaskExecutor::PlanInfo plan;

		sem_execution.enter();
			te.CopyPlan(te.lplans[0],plan);			//copy task to execute (plan)
			te.lplans.erase(te.lplans.begin());		//remove from list of pending plans
		sem_execution.leave();

		// Notify the plan so it's displayed in the GUI:
		const std::string planDesc = plan.asString();
		this->SendSystemLogString(planDesc);
		printf("[Plan details] %s\n",planDesc.c_str());

		//Execute each plan in a different thread
		mrpt::system::createThreadFromObjectMethod(this,&CExecutorApp::Execute,plan);
	}

	return true;
}


//---------------------------------------------------------------
// Execute()
// Method that executes the actions of a given plan
// Its designed to be called on a different thread for each call
//----------------------------------------------------------------
void CExecutorApp::Execute(TaskExecutor::PlanInfo plan)
{
	te.ExecutePlan(plan);
}


bool CExecutorApp::OnConnectToServer()
{
	DoRegistrations();
	return true;
}


bool CExecutorApp::DoRegistrations()
{
	//! @moos_subscribe  NAV_EVENT_END, NAV_EVENT_ERROR, NAV_EVENT_NOWAY, NAV_EVENT_START
	AddMOOSVariable("NAV_EVENT_END","NAV_EVENT_END","NAV_EVENT_END",0);
	AddMOOSVariable("NAV_EVENT_ERROR","NAV_EVENT_ERROR","NAV_EVENT_ERROR",0);
	AddMOOSVariable("NAV_EVENT_NOWAY","NAV_EVENT_NOWAY","NAV_EVENT_NOWAY",0);
	AddMOOSVariable("NAV_EVENT_START","NAV_EVENT_START","NAV_EVENT_START",0);

	//! @moos_subscribe  IN_EXECUTION, NODE_POSITION, VOICE_EVENT_DONE
	AddMOOSVariable("IN_EXECUTION","IN_EXECUTION","IN_EXECUTION",0);
	AddMOOSVariable("NODE_POSITION","NODE_POSITION","NODE_POSITION",0);
	AddMOOSVariable("VOICE_EVENT_DONE","VOICE_EVENT_DONE","VOICE_EVENT_DONE",0);

	//! @moos_subscribe SHUTDOWN
	AddMOOSVariable( "SHUTDOWN", "SHUTDOWN", "SHUTDOWN", 0 );

	//Subscription to the list of vbles in the moos file.
	for (int i=0;i<vble_list.size();i++)
		AddMOOSVariable( vble_list[i].c_str(),vble_list[i].c_str(),vble_list[i].c_str(),0);

	RegisterMOOSVariables();
	return true;
}


bool CExecutorApp::OnNewMail(MOOSMSG_LIST &NewMail)
{
	std::string cad;
	for(MOOSMSG_LIST::iterator i=NewMail.begin();i!=NewMail.end();++i)
	{
		//New plan (list of actions) as result of new task
		if (i->GetName()=="IN_EXECUTION")
				te.DoPlan(i->GetString());


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
