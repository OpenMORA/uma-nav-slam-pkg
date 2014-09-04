#include "TExecutor.hpp"
#include <mrpt/system/threads.h>
#include <mrpt/random/RandomGenerators.h>
#include <mrpt/system/CFileSystemWatcher.h>
#include <mrpt/utils.h>
#include <time.h>
#ifdef MRPT_OS_WINDOWS
#	include <Windows.h>
#endif

using namespace mrpt;
using namespace mrpt::system;
using namespace mrpt::utils;

void TaskExecutor::Moving()
{
	using namespace mrpt::random;
	double x,y;

	//GetInformation("GET_NODE_POSITION publico",x,y);
	std::string response;
	GetInformation("GET_NODE_POSITION publico",response);
	//Get node location from response string
	std::deque<std::string> lista;
	mrpt::system::tokenize(response," ",lista);
	if (lista.size()==2)	//x y
	{
		x=atof(lista[0].c_str());
		y=atof(lista[1].c_str());		
	}
	else
	{
		printf("[TExecutor: Moving] Error while getting the NODE_POSITION of publico\n");
		return;
	}

	printf("Audience is at %f,%f\n",x,y);

    while (!domotica)
    {
		double movprob=randomGenerator.drawUniform(0,1);

		if (movprob>0.75)
		{
            double incx=randomGenerator.drawUniform(-5,5);
            double incy=randomGenerator.drawUniform(-10,10);
            printf("looking at %f %f\n",x+incx,y+incy);
            m_Comms.Notify("LOOK_AT_POINT",format("[%f %f]",x,y+incy));
            mrpt::system::sleep(2000);
		}
    }
}
void TaskExecutor::FileCommands(std::string path)
{


	CFileSystemWatcher::TFileSystemChangeList	lstChanges;
	CFileSystemWatcher::TFileSystemChangeList::iterator it;

	CFileSystemWatcher	watch(path);

    mrpt::system::sleep(3000);
	printf("Watching directory %s\n",path.c_str());

    mrpt::system::createThreadFromObjectMethod(this,&TaskExecutor::Moving);

	#ifdef MRPT_OS_LINUX


	::system("sh -c 'wmctrl -a \"Office\"'");
	::system("sh -c 'W=$(xdotool search --title \"OpenOffice.org Impress\") && xdotool windowfocus $W && xdotool key F5'");
	mrpt::system::sleep(200);
	::system("sh -c 'W=$(xdotool search --title \"OpenOffice.org Impress\") && xdotool windowfocus $W && xdotool key Right'");
	#else
		//for windows......
	#endif

	m_Comms.Notify("GUI_VISIBLE_LASER1","0");
    m_Comms.Notify("GUI_VISIBLE_LASER2","0");


	bool salir=false;
	bool publico=false;
	bool presentation=false;
	bool mostrar=false;
	bool finished=false;
	domotica=false;
	bool mapas=false;
	while (!salir)
	{
		watch.getChanges( lstChanges );

		for (it=lstChanges.begin();it!=lstChanges.end();it++)
		{
			//printf("Event %s\n",it->path.c_str());
			if (it->eventAccessed)
			{
				if (it->path==path+"/exit.wav" && !finished)
				{
				    finished=true;
				    printf("killing me softly...\n");
				    #ifdef MRPT_OS_LINUX

				    ::system("sh -c 'pkill soffice.bin'");
                    ::system("sh -c 'pkill soffice.exe'");

					//mrpt::system::launchProcess	(format("pkill soffice.bin"));
					//mrpt::system::launchProcess	(format("pkill soffice.exe"));
					#else
					mrpt::system::launchProcess	(format("pkill /F -IM soffice.bin"));
					mrpt::system::launchProcess	(format("pkill /F -IM simpress.exe"));

					#endif
					salir=true;
					finishedppt=true;

				}
				else if (it->path==path+"/mirarpublico.wav" && !publico)
				{
				    printf("mirar al publico\n");
					m_Comms.Notify("NEW_TASK", format("TEXECUTOR %d LOOK_AT publico",(int)texecutor_id));
					texecutor_id++;
					mrpt::system::sleep(1000);
					publico=true;
					presentation=false;

				}
				else if (it->path==path+"/mirarpresentacion.wav" && !presentation)
				{
				    printf("mirar a la presentacion\n");
					m_Comms.Notify("NEW_TASK", format("TEXECUTOR %d LOOK_AT presentacion",(int)texecutor_id));
					texecutor_id++;
					mrpt::system::sleep(1000);
					publico=false;
					presentation=true;

				}
				else if (it->path==path+"/mostrarsensores.wav" && !mostrar)
				{
				    mostrar=true;
                ::system("sh -c 'W=$(xdotool search --title \"OpenOffice.org Impress\") && xdotool windowfocus $W && xdotool key Escape'");
                m_Comms.Notify("GUI_VISIBLE_METRIC_MAP","0");
                m_Comms.Notify("GUI_VISIBLE_TOPOLOGICAL_MAP","0");
                m_Comms.Notify("GUI_VISIBLE_VIRTUAL_OBS","0");
                mrpt::system::sleep(500);
                ::system("sh -c 'wmctrl -a \"GUI\"'");
                m_Comms.Notify("SAY","Esto es un interfaz grafico donde represento toda la informacion de la que dispongo.");
                mrpt::system::sleep(5000);
                m_Comms.Notify("SAY","Os voy a mostrar lo que veo con mi laser delantero.");
                mrpt::system::sleep(2000);
                m_Comms.Notify("GUI_VISIBLE_LASER1","1");
                mrpt::system::sleep(7000);
                m_Comms.Notify("GUI_VISIBLE_LASER1","0");
                m_Comms.Notify("GUI_VISIBLE_LASER2","1");
                m_Comms.Notify("SAY","y ahora con el laser trasero.");
                mrpt::system::sleep(7000);
                m_Comms.Notify("SAY","y esto es lo que estoy viendo ahora mismo con mis camaras.");

                m_Comms.Notify("VISION_ENABLE_DISPLAY_IMAGE","1");
                mrpt::system::sleep(10000);
                m_Comms.Notify("VISION_ENABLE_DISPLAY_IMAGE","0");
                m_Comms.Notify("SAY","Bueno, sigamos con la presentacion.");
                m_Comms.Notify("GUI_VISIBLE_LASER2","0");

                mrpt::system::sleep(500);
                ::system("sh -c 'W=$(xdotool search --title \"OpenOffice.org Impress\") && xdotool windowfocus $W && xdotool key F5'");
               // mrpt::system::sleep(1500);
               // ::system("sh -c 'W=$(xdotool search --title \"OpenOffice.org Impress\") && xdotool windowfocus $W && xdotool key Right'");

				}
				else if (it->path==path+"/ocultarsensores.wav" && mostrar)
				{

                    mostrar=false;
				}
				else if (it->path==path+"/mapas.wav" && !mapas)
				{
				    mapas=true;
				    ::system("sh -c 'W=$(xdotool search --title \"OpenOffice.org Impress\") && xdotool windowfocus $W && xdotool key Escape'");
                mrpt::system::sleep(100);

                //activar mapa metrico
                ::system("sh -c 'wmctrl -a \"GUI\"'");
                m_Comms.Notify("SAY","Este es el mapa metrico que he creado de este entorno.");
                m_Comms.Notify("GUI_VISIBLE_METRIC_MAP","1");
                mrpt::system::sleep(4000);
                m_Comms.Notify("SAY","Las zonas blancas son los espacios que he encontrado para desplazarme.");
                mrpt::system::sleep(5000);
                m_Comms.Notify("SAY","mientras que las zonas grises son obstaculos y paredes.");
                mrpt::system::sleep(7000);
                //activar mapa topologico
                m_Comms.Notify("GUI_VISIBLE_TOPOLOGICAL_MAP","1");
                m_Comms.Notify("GUI_SET_CAMERA","DIST=40.0,ANIM=1.0");
                m_Comms.Notify("SAY","y este es el mapa topologico, que me permite definir areas y sus conexiones.");
                mrpt::system::sleep(10000);

                m_Comms.Notify("SAY","bien, ahora que sabeis como represento mi entorno, seguimos");
				::system("sh -c 'W=$(xdotool search --title \"OpenOffice.org Impress\") && xdotool windowfocus $W && xdotool key F5'");
                //mrpt::system::sleep(1500);
                //::system("sh -c 'W=$(xdotool search --title \"OpenOffice.org Impress\") && xdotool windowfocus $W && xdotool key Right'");


				}

				else if (it->path==path+"/domotica.wav" && !domotica)
				{
                    domotica=true;

				    m_Comms.Notify("DOMOTICS_OPERATION",format("ON 0"));
				    mrpt::system::sleep(3000);
				    m_Comms.Notify("SAY","y para que lo veais mejor, voy a encenderle un foco.");
                    mrpt::system::sleep(3000);
                    m_Comms.Notify("DOMOTICS_OPERATION",format("ON 1"));
				    mrpt::system::sleep(5000);
				m_Comms.Notify("VISION_ENABLE_FACEDETECTOR","1");
				    m_Comms.Notify("SAY","además puedo interactuar con la gente, hablar, responder y reconocer sus caras");


                ::system("sh -c 'wmctrl -a \"Face\"'");
                mrpt::system::sleep(10000);
                m_Comms.Notify("VISION_ENABLE_FACEDETECTOR","0");
                m_Comms.Notify("SAY","bueno ya esta bien, lo apago todo y termino con la presentacion");
				mrpt::system::sleep(1000);
				m_Comms.Notify("DOMOTICS_OPERATION",format("OFF 0"));
				m_Comms.Notify("DOMOTICS_OPERATION",format("OFF 1"));

              //  mrpt::system::sleep(500);
              //  ::system("sh -c 'W=$(xdotool search --title \"OpenOffice.org Impress\") && xdotool windowfocus $W && xdotool key F5'");
                //mrpt::system::sleep(1500);
             //   ::system("sh -c 'W=$(xdotool search --title \"OpenOffice.org 3.1\") && xdotool windowfocus $W && xdotool key Right'");
                ::system("sh -c 'wmctrl -a \"OpenOffice.org 3.1\"'");
				mrpt::system::sleep(2000);
                ::system("sh -c 'xdotool key Right'");

				}

			}
		}

		mrpt::system::sleep(500);
	}
}

//---------------------------------
// Constructor
// --------------------------------
TaskExecutor::TaskExecutor(COpenMORAApp::CDelayedMOOSCommClient &Comms):m_Comms(Comms)
{

	errorlist="";
	
	action_index=-1;
	executed_index=0;//ultima accion ejecutada con exito

	internal_status=5;

	strcpy(systemstatus,"");
	
	cancel_current_plans = false;
	startednavigation = false;
	errornavigation = false;
	endnavigation = false;

	end_vision_camshift=false;
	end_speak=false;
	endjoke=false;

	executedplans.clear();
	lplans.clear();
	lplansinexecution.clear();
	inexecution=0;

	pet_map.clear();
	asked_petitions.clear();
	pet_id=0;

	start=false;				//start the show
	present=false;				//start the presentation
	ready=false;				//start the navigation loop

	finishedppt=false;

	//size_t texecutor_id=0;
}

// Save list of jokes
void TaskExecutor::LoadJokes(std::vector<std::string> jokes)
{
	joke_list = jokes;
}


TaskExecutor::~TaskExecutor()
{
}


void TaskExecutor::ReportError(std::string error,bool erase)
{
	sem_error.enter();

	if (erase) errorlist="";
	errorlist.append(error);
	errorlist.append("\n");

	sem_error.leave();
}


void TaskExecutor::PrintPlan(PlanInfo &plan)
{
	std::vector<ActionInfo>::iterator it;
	printf("#########################\n");

	for (it=plan.actions.begin();it!=plan.actions.end();it++)
	{
		printf("-> %s\n",it->name.c_str());
	}
	printf("#########################\n");
}


void TaskExecutor::CopyPlanfromString(const std::string plan, PlanInfo &out)
{
	out.actions.clear();

//	printf("%s\n",plan.c_str());
	std::deque<std::string> lista;

	mrpt::system::tokenize(plan,"#",lista);

	out.address=lista[0];

	out.timestamp=atoi(lista[1].c_str());			//Unique Identifier (may be TaskID instead of timestamp)
	out.localtaskid=atoi(lista[2].c_str());
//	printf("%d,%d\n",out.timestamp,out.localtaskid);

	for (size_t i=3;i<lista.size();i++)
	{
			ActionInfo ai;

//			printf("---->%s\n",lista[i].c_str());
			std::deque<std::string> lista2;
			mrpt::system::tokenize(lista[i]," ",lista2);

			ai.name=lista2[0];

			for (size_t j=1;j<lista2.size();j++)
			{
				ai.param.push_back(lista2[j]);
			}

			out.actions.push_back(ai);
	}
}


void TaskExecutor::CopyPlan(PlanInfo &in, PlanInfo &out)
{
	out.actions.clear();

	std::vector<ActionInfo>::iterator it;
	ActionInfo ai;

	out.timestamp=in.timestamp;
	out.address=in.address;
	out.localtaskid=in.localtaskid;

	for (it=in.actions.begin();it!=in.actions.end();it++)
	{
		ai.acc=it->acc;
		ai.alg=it->alg;
		ai.name=it->name;

		ai.param.assign(it->param.begin(),it->param.end());

		ai.timestamp=(long)time(NULL);

		out.actions.push_back(ai);
	}
}


//-------------------------------------------------------------------
// DoPlan()
// Executes a new plan by adding it to the list of executing plans
//-------------------------------------------------------------------
void TaskExecutor::DoPlan(PlanInfo pi)
{
	internal_status = 0;
	PlanInfo aux;
	CopyPlan(pi,aux);

	//Check that plan is not empty
	if (aux.actions.size()>0)
	{
		lplans.push_back(aux);
		printf("Number of stored plans %u\n",(unsigned int)lplans.size());
	}
	else printf("Received a plan with 0 actions\n");

}


//---------------------------------------------------------------
// DoPlan()
// Executes a new plan comming from string (OpenMORA variable)
//---------------------------------------------------------------
void TaskExecutor::DoPlan(const std::string plan)
{
	PlanInfo aux;
	//Deserialize
	CopyPlanfromString(plan,aux);
	printf("Added plan %s\n",plan.c_str());
	DoPlan(aux);

}

void TaskExecutor::PrintStatus(void)
{
	printf("********************************\n");
	printf("Stored Plans:				   %u\n",(unsigned int)lplans.size());
	printf("#Plans Currently in execution: %u\n",inexecution);
	//printf information about the current plans in execution.
}


bool TaskExecutor::PendingPetitions()
{
	return (asked_petitions.size()!=0);
}



//--------------------------------------------------------------------------------------------------------
// GetInformation(what, response, timeout)
// Method to request information from other modules (trough OpenMORA variables)
// what = String with the OpenMORA variable to request.
// response = string with the variable content requested
//--------------------------------------------------------------------------------------------------------
void TaskExecutor::GetInformation(const std::string what,std::string response,double timeout)
{
	pet_sem.enter();
		pet_id++;									//increase counter of pending petitions
		pet_map.insert(pet_pair(pet_id,what));		//insert petition
		asked_petitions.insert(pet_id);
	pet_sem.leave();

	bool end = false;
	std::string value;
	std::map<size_t,std::string>::iterator  pet_it;

	//Wait till petition is answered
	while (!end)
	{
		sleep(100);
		pet_sem.enter();
			pet_it = answer_map.find(pet_id);
			if (pet_it!=answer_map.end())
			{
				//petition answered
				response = pet_it->second;
				pet_map.erase(pet_id);
				answer_map.erase(pet_it);
				end = true;
			}
		pet_sem.leave();
	}
}


//---------------------------------------------------------------
// ExecutePlan()
// Method that executes the actions of a given plan
// Its designed to be called on a different thread for each call
//----------------------------------------------------------------
void TaskExecutor::ExecutePlan(PlanInfo &p)
{
	bool error = false;
	cancel_current_plans = false;
	//bool worldchanges=false;

	//Cuidado p.actions[i].name es el nombre de la accion desde el punto de vista
	//del planificador, que no tiene por que ser igual que desde el punto de vista de
	//la agenda. Todo depende de como este definido el dominio de planificacion
	//Ej. La tarea "MOVE" se resuelve mediante acciones "GO"

	// Add this plan to the list of executing plans
	lplansinexecution.push_back(p);
	inexecution++;
	std::string errortxt="";	
	unsigned int i=0;				// Index to run all the actions in the current plan

	//Execute all actions in the Plan
	while (i<p.actions.size() && !cancel_current_plans)
	{
		int num_param = p.actions[i].param.size();		//Get num of params of the current action
		printf("-->Action %s with %d params\n",p.actions[i].name.c_str(),num_param);


		// WAIT_TIME(time_minutes)
		//---------------------------
		if (p.actions[i].name=="WAIT_TIME")
		{
			printf("Waiting %s minutes\n",p.actions[i].param[0].c_str());
			double t=atof(p.actions[i].param[0].c_str()) *60*1000; //waiting time in ms.

			//Sleep by periods of 500ms
			int iter=round(t/500);
			for (int c=0;c<iter;c++)
			{
				mrpt::system::sleep(500);
				if (fmod(double(c),120.0)==0) printf("%f minutes left\n",double(iter-c)/120);
			}
			printf("Waiting time finished\n");
		}


		// WAIT_FOR_SIGNAL(variable_name)
		// Notice that this module should be subscribed to the variable_name (see configuration params)
		//----------------------------------------------------------------------------------------------
		else if (p.actions[i].name=="WAIT_FOR_SIGNAL")
		{
			printf("Waiting for signal %s\n",p.actions[i].param[0].c_str());

			std::map<std::string,bool>::iterator pet_it;
			// Insert "flag" to listen to signal_name variable
			signal_map.insert(signal_pair(p.actions[i].param[0].c_str(),false));
			pet_it=signal_map.find(p.actions[i].param[0].c_str());

			// Wait till the just added signal becomes true (corresponding OpenMORA variable has been published with value >0)
			// See Executor::Iterate
			while (pet_it!=signal_map.end() && !pet_it->second)
			{
				mrpt::system::sleep(500);
				pet_it = signal_map.find(p.actions[i].param[0].c_str());
			}

			//Remove the "flag" to the signal once it has been detected
			signal_map.erase(pet_it);
			printf("Finishing the Waiting for %s signal\n",p.actions[i].param[0].c_str());
		}

		
		// START
		//------
		else if (p.actions[i].name=="START")
		{
			start=true;
		}


		// WAIT_TO_START
		//-----------------
		else if (p.actions[i].name=="WAIT_TO_START")
		{
			printf("Waiting to start\n");
			while (!start) mrpt::system::sleep(500);
			printf("Finishing the Waiting to start\n");
		}

		
		// END
		//------
		else if (p.actions[i].name=="END")
		{
			ready = true;
		}


		// PRESENT
		//---------
		else if (p.actions[i].name=="PRESENT")
		{
			present=true;
		}


		// WAIT_TO_PRESENT
		//----------------
		else if (p.actions[i].name=="WAIT_TO_PRESENT")
		{
			printf("Waiting to present\n");
			while (!present) mrpt::system::sleep(500);
			printf("Finishing the Waiting to present\n");
		}
		

		// WAIT_TO_GO
		//-----------
		else if (p.actions[i].name=="WAIT_TO_GO")
		{
			printf("Ready to go\n");
			while (!ready) mrpt::system::sleep(500);
			printf("GO!\n");
		}

		
		// PAUSE_MOVE
		//-----------
		else if (p.actions[i].name=="PAUSE_MOVE")
		{
			printf("Action PAUSE_MOVE\n");
			m_Comms.Notify("PNAVIGATORREACTIVEPTG_CMD","PAUSE");
		}


		// CANCEL_NAV: Cancels the reactive Navigation
		//---------------------------------------------
		else if (p.actions[i].name=="CANCEL_NAV")
		{
			printf("Action CANCEL_NAV\n");
			m_Comms.Notify("PNAVIGATORREACTIVEPTG_CMD","STOP");
		}


		// RESUME_MOVE
		//------------
		else if (p.actions[i].name=="RESUME_MOVE")
		{
			printf("Action CONTINUE\n");
			m_Comms.Notify("PNAVIGATORREACTIVEPTG_CMD","RESUME");
		}


		// PUBLISH & PUBLISHF
		//--------------------
		else if (p.actions[i].name=="PUBLISH" || p.actions[i].name=="PUBLISHF")
		{
			//Get parameters (content to publish)
			std::string cad="";
			for (size_t k=1;k<size_t(num_param);k++)
				cad=cad+p.actions[i].param[k]+" ";
			
			if (cad!="")
			{
				if (p.actions[i].name=="PUBLISH")
					m_Comms.Notify(p.actions[i].param[0].c_str(),cad);
				else
					m_Comms.Notify(p.actions[i].param[0],atof(cad.c_str()));

				printf("PUBLISH action:%s=%s\n",p.actions[i].param[0].c_str(),cad.c_str());
			}
		}


		// SAY
		//----
		else if (p.actions[i].name=="SAY")
		{
			std::string cad="";
			for (size_t k=0;k<size_t(num_param);k++)
				cad=cad+p.actions[i].param[k]+" ";
			
			printf("SAY action [%s]\n",cad.c_str());
			if (cad!="")
				m_Comms.Notify("SAY",cad);
			
			//Waits till speak event ends
			while (!end_speak) sleep(100);
		}


		// SAY_JOKES
		//----------
		else if (p.actions[i].name=="SAY_JOKES")
		{
			endjoke = false;
			while (!endjoke)
			{
				//Randomly get a Joke from list (see  mission file)
				double jokeprob = mrpt::random::randomGenerator.drawUniform(0,joke_list.size()-1);
				printf("Selected joke %f\n",jokeprob);
				int jokeid=round(jokeprob);
				printf("Selected joke %d\n",jokeid);
				//SAY joke and wait till VOICE_EVENT_END
				m_Comms.Notify("SAY",joke_list[jokeid]);
				while (!end_speak) sleep(100);
				//Randomly wait to start new joke
				int pause=mrpt::random::randomGenerator.drawUniform(10000,20000);
				printf("Waiting %d seconds for the next joke\n",pause);
				mrpt::system::sleep(pause);
			}
		}

		// END_SAY_JOKES
		//--------------
		else if (p.actions[i].name=="END_SAY_JOKES")
		{
			endjoke=true;
		}


		// PLAY
		//-------
		else if (p.actions[i].name=="PLAY")
		{			
			if (num_param>0)
			{
				printf("Executing action PLAY for %s\n",(mission_directory+"\\"+p.actions[i].param[0]).c_str());

#ifdef MRPT_OS_WINDOWS
				PlaySound((mission_directory+"\\"+p.actions[i].param[0]).c_str(), NULL, SND_ASYNC);
#endif
			}
		}

		/*OLD STUFF for showing a ppt 			//It blocks until the process finishes!

					//lanzo hebra de escucha
					std::string path=".";

					mrpt::system::createThreadFromObjectMethod(this,&TaskExecutor::FileCommands,path);


                    mrpt::system::sleep(10000);

                    	while (!finishedppt)
                        {
                            //printf(".\n");
                            mrpt::system::sleep(100);
                        }

//					mrpt::system::launchProcess	(format("simpress -show %s -nologo",p.actions[i].param[0].c_str()));
/*
#ifdef MRPT_OS_WINDOWS
					mrpt::system::launchProcess	(format("simpress -show -nologo -norestore %s",p.actions[i].param[0].c_str()));
					#else
					mrpt::system::launchProcess	(format("soffice -show -nologo -norestore %s",p.actions[i].param[0].c_str()));
#endif
  */      //            printf("after launching ppt\n");

		//		}

		//	}


		
		// LOOK_AT
		//---------
		else if (p.actions[i].name=="LOOK_AT")
		{
			double x,y;
			printf("Looking at %s\n",p.actions[i].param[0].c_str());

			//GetInformation(""GET_NODE_POSITION "+p.actions[i].param[0],x,y);
			std::string response;
			GetInformation("GET_NODE_POSITION "+p.actions[i].param[0],response);
			//Get node location from response string
			std::deque<std::string> lista;
			mrpt::system::tokenize(response," ",lista);
			if (lista.size()==2)	//x y
			{
				x=atof(lista[0].c_str());
				y=atof(lista[1].c_str());
				printf("Looking at %f,%f\n",x,y);
				m_Comms.Notify("LOOK_AT_POINT",format("[%f %f]",x,y));
				while (!endnavigation){mrpt::system::sleep(100);}
			}
			else
			{
				printf("[TExecutor: LOOK_AT] Error while getting the NODE_POSITION of %s\n",p.actions[i].param[0].c_str() );				
			}
		}


		// SWITCH_ON
		//-----------
		else if (p.actions[i].name=="SWITCH_ON")
		{
			printf("Swithching on %d\n",atoi(p.actions[i].param[0].c_str()));
			m_Comms.Notify("DOMOTICS_OPERATION",format("ON %d",atoi(p.actions[i].param[0].c_str())));
		}

		// SWITCH_OFF
		//-----------
		else if (p.actions[i].name=="SWITCH_OFF")
		{
			printf("Swithching off %d\n",atoi(p.actions[i].param[0].c_str()));
			m_Comms.Notify("DOMOTICS_OPERATION",format("OFF %d",atoi(p.actions[i].param[0].c_str())));
		}

		
		// GO (node_label x y)
		//---------------------
		else if (p.actions[i].name=="GO")
		{
			error = false;
			endnavigation = false;
			errornavigation = false;			

			if (num_param>1)
			{
				std::string param1 = p.actions[i].param[0];  //node_label
				std::string param2 = p.actions[i].param[1];  //x
				std::string param3 = p.actions[i].param[2];  //y				
				printf("GO to %s (%s %s)\n",param1.c_str(),param2.c_str(),param3.c_str());
				
				//Commands a reactive navigation to given point
				const std::string s = format("[%.03f %.03f]", atof(param2.c_str()),atof(param3.c_str()));
				m_Comms.Notify("NAVIGATE_TARGET",s);
				
				//Wait till navigation complete
				printf("Waiting for reaching %s\n",param1.c_str());
				while (!endnavigation && !errornavigation && !cancel_current_plans)
					mrpt::system::sleep(100);

				error = error || errornavigation;

				//if (!error)
				//{
				//	//Update the pose of the robot in the graph
				//	m_Comms.Notify("ROBOT_TOPOLOGICAL_PLACE",param1);
				//}
			}
		}
		
		
		//SKYPE
		//---------
		else if (p.actions[i].name=="SKYPE")
		{
			printf("Sending through Skype\n");
			std::string cad="";
				//Create phrase by concatenating parameters
				for (size_t k=0;k<size_t(num_param);k++)
					cad=cad+p.actions[i].param[k]+" ";
				
				if (cad!="")
					m_Comms.Notify("SAY",cad);				

				while (!end_speak) sleep(100);
		}

		

		//-----------------------------------------------------
		//END OF ACTIONS
		//-----------------------------------------------------

		if (error)
		{
			printf("Error while executing action %s(%d), re-trying\n",p.actions[i].name.c_str(),i);
			mrpt::system::sleep(3000);
		}
		else
		{
			i++;	//Increase counter, to execute next action in the plan
		}
	}// end while actions


	if( cancel_current_plans )
		printf("[Executor]Plan finished because All Plans were cancelled\n");

	//mrpt::utils::sleep(2000);
	printf("[Executor]Plan finished with taskID %u Owner %s LocalTaskID %u\n",(unsigned int)p.timestamp,p.address.c_str(),(unsigned int)p.localtaskid);
	executedplans.push_back(p.timestamp);

	//Sends event of plan finished
	//! @moos_publish PLAN_FINISHED  Variable that inform that a Plan (task) has been completely executed.
	//! @moos_var PLAN_FINISHED  Variable that inform that a Plan (task) has been completely executed.
	//! Format: PLAN_FINISHED TaskID UserTaskID UserID
	m_Comms.Notify("PLAN_FINISHED",format("%u %u %s",(unsigned int)p.timestamp,(unsigned int)p.localtaskid,p.address.c_str()));

	inexecution--;

	 //TODO: Removing this plan from the list of currently executing plans!!
}


void TaskExecutor::LastExecutedPlan(long &timestamp)
{
//	printf("Call LastExecutedPlan\n");

	timestamp=-1;
	if (executedplans.size()>0)
	{
		timestamp=executedplans[executedplans.size()-1];
	//	executedplans.erase(executedplans.end()-1); No borro nada!
	}
}


void TaskExecutor::IsPlanExecuted(long timestamp,bool &res)
{
	printf("Call LastExecutedPlan\n");

	res=false;
	for (unsigned int i=0;i<executedplans.size();i++)
	{
		if (executedplans[i]==timestamp)
		{
			res=true;
			break;
		}
	}
	if (res) printf("Returned plan with timestamp %f\n", mrpt::system::timestampTotime_t(timestamp));
}


void TaskExecutor::GetNavigationDestination(std::string &pt,std::string &gt)
{
	if (partial_target!="" && internal_status==0)
		pt=partial_target;
	gt=target;
}


void TaskExecutor::FailedNavigation()
{
	printf("Error event received\n");
	errornavigation=true;
	m_Comms.Notify("SAY","No soy capaz de llegar a mi destino. Vuelvo a intentarlo");
	//endnavigation=true; // por ahora cuando hay un error de navegacion, ejecuto la siguiente acción
}


//---------------------------------
// EndNavigation()
// Este metodo se ejecuta cuando se reciba el evento EndNavigationAction 1061
// Se supone que ese evento indica fin de navegacion con exito de la UNICA accion de navegacion que puede
// estar realizando el robot en cada momento
//---------------------------------
void TaskExecutor::EndNavigation()
{
	printf("Received EndNavigation Event\n");
	endnavigation=true;
}


void TaskExecutor::EndSpeak()
{
	printf("Received EndSpeak Event\n");
	end_speak=true;
}


void TaskExecutor::GetRobotStatus(long &code,char** textstatus)
{
	*textstatus=new char[255];

	switch (internal_status)
	{
	case -1: {strcpy(*textstatus,"No sé lo que me pasa....");break;}
	case 0: {strcpy(*textstatus,"Estoy navegando");break;}
	case 1: {strcpy(*textstatus,"Estoy Detenida");break;}
	case 2: {strcpy(*textstatus,"Estoy Bloqueada: No sé qué hacer");break;}
	case 3: {strcpy(*textstatus,"Estoy reanudando la navegación");break;}
	case 4: {strcpy(*textstatus,"Estoy perdida. Por favor ayúdame");break;}
	case 5: {strcpy(*textstatus,"Estoy descansando. Preparada para comenzar a navegar!!");break;}
	case 6: {strcpy(*textstatus,"Tú me estas guiando... ¿Dónde me llevas?");break;}
	case 7: {strcpy(*textstatus,"Retomando el control");break;}
	case 8: {strcpy(*textstatus,"Estoy girando");break;}
	}
	code=internal_status;
}


void TaskExecutor::GetSystemStatus(char **textstatus)
{
	*textstatus=new char[2048];
	strcpy(*textstatus,systemstatus);

}