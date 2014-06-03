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

/**  @moos_module A module that plans complex tasks into a sequence of small, simple routines.   */

#include "CPlannerApp.hpp"

#include <sstream>
#include <iomanip>
#include <iostream>

using namespace std;

using namespace mrpt;
using namespace mrpt::slam;
using namespace mrpt::utils;
//using namespace ....


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
	//! @moos_param PARAM_NAME  PARAM DESCRIPTION IN ONE LINE
	m_MissionReader.GetConfigurationParam("world_model",world_model_file);
	m_MissionReader.GetConfigurationParam("planning_domain",planning_domain_file);

	m_MissionReader.GetConfigurationParam("num_actions",num_actions);
	m_MissionReader.GetConfigurationParam("pCommon","actions_names",actions_names);

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

	planner.HPWAInit(actions_names,comp_list);
	// There is also a MRPT-like object (this->m_ini) that is a wrapper
	//  to read from the module config block of the current MOOS mission file.
	// m_ini.read_int(...);
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



	CMOOSVariable *path = GetMOOSVar("PATH");
	CMOOSVariable *PLAN_FINISHED = GetMOOSVar("PLAN_FINISHED");

	if (PLAN_FINISHED && PLAN_FINISHED->IsFresh())
	{
			PLAN_FINISHED->SetFresh(false);
			planner.FinishedCommand(atoi(PLAN_FINISHED->GetStringVal().c_str()));
	}

	if (planner.PendingPetitions())
	{

		planner.pet_sem.enter();

		std::string petition=planner.pet_map.begin()->second;

		//printf("Pending %d,%s\n",planner.pet_map.begin()->first,petition.c_str());

		std::deque<std::string> cad;
		mrpt::utils::tokenize(petition," ",cad);

		//printf("notifying %s %d %s\n",cad[0].c_str(),planner.pet_map.begin()->first,cad[1].c_str());
		m_Comms.Notify(cad[0],format("%d %s",(int)planner.pet_map.begin()->first,cad[1].c_str()));

		planner.asked_petitions.erase(planner.pet_map.begin()->first);
		//printf("borre %d y quedan %d\n",planner.pet_map.begin()->first,planner.asked_petitions.size());

		planner.pet_sem.leave();
	}


	if (path && path->IsFresh())
	{
		path->SetFresh(false);
	//	printf("%s\n",path->GetStringVal().c_str());

		std::deque<std::string> lista;
		mrpt::utils::tokenize(path->GetStringVal()," ",lista);

		planner.pet_sem.enter();
		std::deque<std::string> cad;
		mrpt::utils::tokenize(path->GetStringVal()," ",cad);
		planner.answer_map.insert(pet_pair(atoi(cad[0].c_str()),path->GetStringVal()));

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
	//! @moos_subscribe  TASK_LIST, PATH, PLAN_FINISHED
	AddMOOSVariable("TASK_LIST","TASK_LIST","TASK_LIST",0);
	AddMOOSVariable("PATH","PATH","PATH",0);
	AddMOOSVariable("PLAN_FINISHED","PLAN_FINISHED","PLAN_FINISHED",0);

	//! @moos_subscribe SHUTDOWN
	AddMOOSVariable( "SHUTDOWN", "SHUTDOWN", "SHUTDOWN", 0 );

	RegisterMOOSVariables();
	return true;
}

void CPlannerApp::Plan(std::string goal)
{
	std::deque<std::string> cad;
	mrpt::utils::tokenize(goal," ",cad);


	//std::string goal=address+" "+format ("%d",taskid)+" "+format ("%d",localtaskid+command+" "+task;

	printf("Plan thread %s\n",goal.c_str());

	printf("(%d)\n",(int)cad.size());


	//m_Comms.Notify("TASK_PLANNED", format("%s#%d",cad[0].c_str(),atoi(cad[1].c_str())));

	if (cad.size()>=5)
		//by now only 1 parameter is considered cad[4], so if (cad.size()==5) should be enough...
		{
			std::string params;
			printf("creating params\n");
			for (size_t j=4;j<cad.size();j++)
			{	params.append(cad[j]);params.append(" ");
				printf("[%s]\n",params.c_str());
			}
			planner.SolveTask(cad[0],atoi(cad[1].c_str()),atoi(cad[2].c_str()),cad[3],params);
		}
	else if (cad.size()==4)
		planner.SolveTask(cad[0],atoi(cad[1].c_str()),atoi(cad[2].c_str()),cad[3],"");

    else printf("Error while parsing task\n");

}
bool CPlannerApp::OnNewMail(MOOSMSG_LIST &NewMail)
{
	UpdateMOOSVariables(NewMail);

	for(MOOSMSG_LIST::iterator i=NewMail.begin();i!=NewMail.end();++i)
	{
		if (i->GetName()=="TASK_LIST")
		{
			//printf("Received from task_list [%s]\n",i->GetAsString().c_str());
			std::deque<std::string> lista;
			mrpt::utils::tokenize(i->GetString().c_str(),"#",lista);

			std::string command,task,address;
			size_t taskid,localtaskid;
			for (size_t j=0;j<lista.size();j++)
			{
				//CheckCompatibility

				bool tasktosolve=planner.SelectTasktobePlanned(i->GetAsString(),command,task,taskid,address,localtaskid);
				if (taskid!=size_t(-1) && tasktosolve)
				{
					std::string goal=address+" "+format ("%d",(int)taskid)+" "+format ("%d",(int)localtaskid)+" "+command+" "+task;

					printf("Planning %s\n",goal.c_str());
					m_Comms.Notify("TASK_PLANNED", format("%s#%d",address.c_str(),(int)taskid));
					mrpt::utils::createThreadFromObjectMethod(this,&CPlannerApp::Plan,goal);
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
