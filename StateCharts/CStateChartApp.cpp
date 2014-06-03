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

/**  @moos_module This module processes .scxml files that contain state charts. 
  *  It will read the file given by OBJECTIVE_FILE and execute the defined state charts.
  *  Each state is composed by different TASKS that are executed by sending the NEW_TASK to the Agenda module.
  *  After all the task of a state are finished, the following state is processed.
  *
  *  Note: This module will accept the .scxml files given by the GUI. 
  *  This involves that the StateChart can be updated online.
  */

/** @moos_ToDo
  *  Check that NEW_TASK is correctly set. See NotifyNewTask method.
  */

#include "CStateChartApp.hpp"

#include <sstream>
#include <iomanip>
#include <iostream>

using namespace std;
using namespace mrpt;
using namespace mrpt::utils;


CStateChartApp* CStateChartApp::meagain = NULL;

CStateChartApp::CStateChartApp()
{
	rut = new Rutinas;
	rut->log = &CStateChartApp::NotifyNewTask;
	rut->check = &CStateChartApp::Check4Vble;
	
	meagain = this;	
	meagain->handler = new ManejadorSCXML(rut);
	active=true;
}

CStateChartApp::~CStateChartApp()
{
}

bool CStateChartApp::OnStartUp()
{
	//! @moos_param OBJECTIVE_FILE File containing the objectives chart that will be executed. This line can be omitted safely
	if(m_MissionReader.GetConfigurationParam("OBJECTIVE_FILE",OBJECTIVE_FILE))
		SCThread = mrpt::system::createThread( ExecuteStateChart );			//Crear en una nueva hebra	
	else
		MOOSTrace("[StateChart:OnStartUp]Warning: No initial SCXML file provided.\n");
		
	return DoRegistrations();
}

bool CStateChartApp::OnCommandMsg( CMOOSMsg Msg )
{
	if(Msg.IsSkewed(MOOSTime())) return true;
	if(!Msg.IsString()) return MOOSFail("This module only accepts string command messages\n");
	const std::string sCmd = Msg.GetString();
	//MOOSTrace("COMMAND RECEIVED: %s\n",sCmd.c_str());
	// Process the command "sCmd".

	return true;
}

bool CStateChartApp::Iterate()
{
	//Nothing to do
	return true;
}


double CStateChartApp::Check4Vble(const char *name)
{
	double res=-1;
/*	meagain->AddMOOSVariable(name,name,name,0);
	
	meagain->RegisterMOOSVariables();
	*/
	CMOOSVariable *v = meagain->GetMOOSVar(name);
	if (v)
	{
			res=atof(v->GetAsString().c_str());
			printf("Checking vlbe %f\n",res);
	}
//	printf("but var 1 is %f\n",*var1);
	//res=var1;
	return res;
}


void CStateChartApp::NotifyNewTask(void *p,const char *label,const int uid,const char *comando,const int pri,const bool parallel)
{	
	//! @moos_publish NEW_TASK Launch a new task from a given state in the StateCharts
	//  format: NEW_TASK UserID TaskID UserTaskID Command
	meagain->m_Comms.Notify("NEW_TASK", format("STATECHART %i %s",uid,comando) );
	printf("[%s] TaskId [%i]\n",comando,uid);
	mrpt::system::sleep(100);	
}


//----------------------------------------------------
// ExecuteStateChart()
// Designed to run in a new thread for each call.
// Executes a new state chart provided the SCXML file.
//------------------------------------------------------
void CStateChartApp::ExecuteStateChart()
{
	//load SCXML file
	meagain->handler->lee_fichero(meagain->OBJECTIVE_FILE);
}


bool CStateChartApp::OnConnectToServer()
{	
	DoRegistrations();
	return true;
}


bool CStateChartApp::DoRegistrations()
{
	//! @moos_subscribe  OBJECTIVE_FILE, PLAN_FINISHED
	AddMOOSVariable("OBJECTIVE_FILE","OBJECTIVE_FILE","OBJECTIVE_FILE",0);
	AddMOOSVariable("PLAN_FINISHED","PLAN_FINISHED","PLAN_FINISHED",0);

	//! @moos_subscribe SHUTDOWN
	AddMOOSVariable( "SHUTDOWN", "SHUTDOWN", "SHUTDOWN", 0 );
	
	//! @moos_subscribe STOP_STATECHART
	AddMOOSVariable( "STOP_STATECHART", "STOP_STATECHART", "STOP_STATECHART", 0 );

	//! @moos_subscribe RESUME_STATECHART
	AddMOOSVariable( "RESUME_STATECHART", "RESUME_STATECHART", "RESUME_STATECHART", 0 );

	//! @moos_subscribe Battery_Level, Mota_Alert, Gesture_Id, Person_Found
	AddMOOSVariable( "Battery_Level", "Battery_Level", "Battery_Level", 0 );
	AddMOOSVariable( "Mota_Alert", "Mota_Alert", "Mota_Alert", 0 );
	AddMOOSVariable( "Gesture_Id", "Gesture_Id", "Gesture_Id", 0 );
	AddMOOSVariable( "Person_Found", "Person_Found", "Person_Found", 0 );
	
	RegisterMOOSVariables();

	return true;
}



bool CStateChartApp::OnNewMail(MOOSMSG_LIST &NewMail)
{
	
	for(MOOSMSG_LIST::iterator i=NewMail.begin();i!=NewMail.end();++i)
	{
		
		// PLAN_FINISHED TaskID UserTaskID UserID
		if (i->GetName()=="PLAN_FINISHED" && active)
		{
			std::deque<std::string> cad;
			mrpt::system::tokenize(i->GetString()," +",cad);
			printf("PlanFinished Event received for local[global] id %d[%d]%s\n",atoi(cad[1].c_str()),atoi(cad[0].c_str()),cad[2].c_str());

			//Was task created by StateChart?
			if (cad[2]=="STATECHART")
			{
				//unsigned char enc='\0';
				list<TareaPendiente *> *lista = (list<TareaPendiente *> *)meagain->handler->pendientes;

				//printf("%u\n",(unsigned)lista->size());
				for (list<TareaPendiente *>::iterator i=lista->begin();i!=lista->end();i++)
				{
					TareaPendiente *t=*i;

					if ( (atoi(cad[1].c_str()))==t->id)
					{
						//printf("%d,%d,%s\n",atoi(planfinished->GetStringVal().c_str()),t->id,t->label);

						printf("Se ha detectado la terminaci\242n del evento %s.\n",t->label);

						EventoExterno *e=new EventoExterno;
						char *tmp=new char[strlen(t->label)+6];
						strcpy(tmp,t->label);
						strcat(tmp,".done");
						e->nombre=strdup(tmp);
						delete[] tmp;
						e->target=NULL;
						e->tstamp=mrpt::system::now();
						e->_eventData=NULL;
						e->procesados=0;
						meagain->handler->anadirEventoExterno(e);
						lista->erase(i);
						break;
					}
				}//end-if UserID=StateChart				
			}
		}


		if( (i->GetName()=="SHUTDOWN") && (MOOSStrCmp(i->GetString(),"true")))
		{
			// Disconnect comms:			
			MOOSTrace("Closing Module \n");
			this->RequestQuit();
		}


		if (i->GetName()=="STOP_STATECHART")
		{
			printf("STATECHART stopped\n");
			active = false;			
		}

		if (i->GetName()=="RESUME_STATECHART")
		{
			printf("STATECHART resumed\n");
			//Flush the current state to go to the next one
			
			list<TareaPendiente *> *lista=(list<TareaPendiente *> *)meagain->handler->pendientes;

			printf("Flushing %u actions\n",(unsigned)lista->size());
			for (list<TareaPendiente *>::iterator i=lista->begin();i!=lista->end();i++)
			{
				TareaPendiente *t=*i;

				printf("%d,%s\n",t->id,t->label);

				printf("Forcing finish event for %s\n",t->label);

				EventoExterno *e=new EventoExterno;
				char *tmp=new char[strlen(t->label)+6];
				strcpy(tmp,t->label);
				strcat(tmp,".done");
				e->nombre=strdup(tmp);
				delete[] tmp;
				e->target=NULL;
				e->tstamp=mrpt::system::now();
				e->_eventData=NULL;
				e->procesados=0;
				meagain->handler->anadirEventoExterno(e);
			}	
			lista->clear();

			active=true;
		}


		if (i->GetName()=="OBJECTIVE_FILE")
		{
		    // A new state chart is to be loaded. The ongoing state chart will be interrupted, and the new one launched.
		    
			// First, stop the current process by killing its thread		    
            meagain->handler->matar();
            mrpt::system::sleep(3000);      // Wait one second so the previous state chart has time to exit

            // Set the name of the SCXML file
            OBJECTIVE_FILE = i->GetString();

            // Execute it
            SCThread = mrpt::system::createThread(ExecuteStateChart);
		}
	}

	UpdateMOOSVariables(NewMail);
	return true;
}
