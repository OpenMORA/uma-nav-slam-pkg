#include "HPWA.hpp"
#include <iostream>

#include <mrpt/system.h>


// Dialoguing functions.. here by now
void Hora(std::string &horatext)
{
	printf("Hora\n");

/* Display operating system-style date and time. */

                mrpt::system::TTimeParts timParts;

                mrpt::system::timestampToParts(mrpt::system::getCurrentLocalTime(), timParts, true);

				char text[255];
				char coletilla[50];
				int num_hora = timParts.hour+2;

				int num_min  = timParts.minute;

				int hora12;


				if (num_hora>12) hora12=num_hora-12;
				else hora12=num_hora;


				if (num_hora>12 && num_hora<16) sprintf(coletilla,"del mediodia ");
				else if (num_hora>16 && num_hora<21) sprintf(coletilla,"de la tarde ");
				else if (num_hora>21 && num_hora<5) sprintf(coletilla,"de la noche ");
				else sprintf(coletilla,"de la mañana ");

				int cont=rand()%3;
				if (num_hora!=13)
				{
					switch (cont)
					{
								case 0: sprintf(text,"Uy que tarde!, son las %d %s y %d minutos\n",hora12,coletilla,num_min);break;
							case 1: sprintf(text,"Cómo pasa el tiempo, ya son las  %d y %d \n",hora12,num_min);break;
							case 2: sprintf(text,"Son las %d %s y %d minutos\n",hora12,coletilla,num_min);break;
					}
				}
				else
					{
					switch (cont)
					{
							case 0: sprintf(text,"Uy que tarde!, es la %d %s y %d minutos\n",hora12,coletilla,num_min);break;
							case 1: sprintf(text,"Cómo pasa el tiempo, ya es la  %d y %d \n",hora12,num_min);break;
							case 2: sprintf(text,"Es la %d %s y %d minutos\n",hora12,coletilla,num_min);break;
					}
				}

//				printf("Time: %s\n",text);
				horatext=text;

}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////Aux Functions///////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////

void HPWA::GetListFromTabla(tabla t, lista &l, std::string key,bool verbose)
{

	l.clear();

	for (unsigned int i=0;i<t.size();i++)
	{
		if (key==t[i][0])
		{
			if (verbose) printf("==================\n");
			for (unsigned int j=1;j<t[i].size();j++)
			{
				l.push_back(t[i][j]);
			if (verbose) 	printf("%s ",t[i][j].c_str());
			}
			if (verbose) printf("\n==================\n");

		}

	}

}


//------------------------------------------------------------------------------
// PrintTabla()
// Prints the current plan, that is, the list of actions
//-------------------------------------------------------------------------------
void HPWA::PrintTabla(tabla t)
{
	for (unsigned int i=0;i<t.size();i++)
	{
		for (unsigned int j=0;j<t[i].size();j++)
		{
			printf("%s ",t[i][j].c_str());
		}
		printf("\n");
	}
}

//------------------------------------------------------------------------------
// SPrintPlan()
// Gets the current plan, that is, the list of actions as a string
//-------------------------------------------------------------------------------
void HPWA::SPrintPlan(tabla t, std::string &solution)
{
	solution="";
	//For each action in the plan
	for (unsigned int i=0;i<t.size();i++)
	{
		//For each field of the action
		for (unsigned int j=0;j<t[i].size();j++)
		{			
			solution=solution+t[i][j]+" ";
		}
		solution=solution+"#";
	}
}


void HPWA::PrintLTabla(ltabla t)
{
	for (unsigned int i=0;i<t.size();i++)
	{
		PrintTabla(t[i]);
		printf("----------------\n");
	}
}
void HPWA::PrintCost(planningcost c)
{
	printf("$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$\n");
	printf("Results from planning\n");
	printf("Total Time %f\nEmbeded Time %f\n#Calls %ld\n#Nodes expanded %ld\n",
							c.totaltime,c.embededtime,c.num_calls,c.num_nodes);

	printf("$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$\n");

}
void HPWA::FPrintTabla(FILE *f,tabla t)
{
	for (unsigned int i=0;i<t.size();i++)
	{
		for (unsigned int j=0;j<t[i].size();j++)
		{
			fprintf(f,"%s ",t[i][j].c_str());


		}
		fprintf(f,"\n");
	}


}
void HPWA::FPrintLTabla(FILE *f,ltabla t)
{
	for (unsigned int i=0;i<t.size();i++)
	{
		FPrintTabla(f,t[i]);
		fprintf(f,"----------------\n");
	}

}
void HPWA::FPrintCost(FILE *f,planningcost c)
{
	fprintf(f,"\nResults from planning\n");

		fprintf(f,"Total Time %f\nEmbeded Time %f\n#Calls %ld\n#Nodes expanded %ld\n",
							c.totaltime,c.embededtime,c.num_calls,c.num_nodes);
	fprintf(f,"\n");

}

void HPWA::SumCost(planningcost &cost1,planningcost cost2) //cost1=cost1+cost2
{
	cost1.totaltime+=cost2.totaltime;
	cost1.embededtime+=cost2.embededtime;
	cost1.num_calls+=cost2.num_calls;
	cost1.num_nodes+=cost2.num_nodes;
}
void HPWA::AssignCost(planningcost &cost1,planningcost cost2) //cost1<-cost2
{
	cost1.totaltime=cost2.totaltime;
	cost1.embededtime=cost2.embededtime;
	cost1.num_calls=cost2.num_calls;
	cost1.num_nodes=cost2.num_nodes;
}
void HPWA::InitCost(planningcost &cost) //cost1<-0
{
	cost.totaltime=0;
	cost.embededtime=0;
	cost.num_calls=0;
	cost.num_nodes=0;
}
void HPWA::AverageCost(planningcost &cost,int num) //cost=cost/num
{
	cost.totaltime=cost.totaltime/num;
	cost.embededtime=cost.embededtime/num;
	cost.num_calls=cost.num_calls/num;
	cost.num_nodes=cost.num_nodes/num;


}
void HPWA::MarcaPlan(tabla p,long level,long type,long marca)
{
	//printf("MarcandoPlan\n");
	long id=0;
	for (unsigned int i=0;i<p.size();i++)
	{
		for (unsigned int j=1;j<p[i].size();j++)   //j=0 es el predicado!!
		{

			//rc->RutinaGetNodeId(p[i][j],level,id);
			if (id!=0)
			{
				//rc->RutinaMarkNode(id,marca);
				//std::string name;
				////rc->RutinaReadNode(id,name);
				//printf("Marking %s\n",name.c_str());
				////rc->RutinaMarkAllSubnodes(id,type,marca);
			}

		}
	}

	//Se marca el nodo Robot (y sus subelementos de haberlos) porque siempre esta involucrado en el plan
	////rc->RutinaGetNodeId("Robot",level,id);
	if (id!=0)
	{	//rc->RutinaMarkNode(id,marca);
		//rc->RutinaMarkAllSubnodes(id,type,marca);
	}
//	printf("FinMarcadoPlan\n");


}
void HPWA::GetPredAncestors(tabla post,ltabla &res,long type)
{

	std::vector<long> levels;
	//rc->RutinaAllLevels(levels,type);

	res.push_back(post);

	bool salir=false;
	for (unsigned int n=0;n<levels.size()-1;n++)
	{
		//printf("->%d\n",n);
		tabla aux;

		for (unsigned int j=0;j<res[n].size();j++)
		{

		//	printf("--->%d\n",n);
			lista laux;
			for (unsigned int k=0;k<res[n][j].size();k++)
			{
				if (k==0 /*|| k==1*/) laux.push_back(res[n][j][k]);
				else
				{
					long id=0,super=0;
					//rc->RutinaGetNodeId(res[n][j][k],levels[n],id);
					//rc->RutinaGetSupernode(id,type,super);
			//		printf("---->%s,%d,%d\n",res[n][j][k].c_str(),id,super);
					if (super!=0 && id!=0)
					{
						std::string name;
						//rc->RutinaReadNode(super,name);
						laux.push_back(name);
					}
					else
					{

						salir=true;
						break;
					}

				}
			}
			if (!salir) aux.push_back(laux);

		}
		if (!salir) res.push_back(aux);
		else break;
	}

}

void HPWA::GetPredSucessorsAux(refpred pred,refpred &res, long type)
{
	lista laux;
	tabla taux;
	std::vector<long> subs;
	res.ref.clear();
	res.name=pred.name;

//	printf("GetPredSucessors Aux\n");

	for (unsigned int i=0;i<pred.ref.size();i++)
		{
			laux.clear();
			for (unsigned int j=0;j<pred.ref[i].size();j++)
			{
//				long id;
				//rc->RutinaGetNodeId(pred.ref[i][j],0,id);
				//printf("%s\n",pred.ref[i][j].c_str());
				//rc->RutinaGetSubnodes(id,type,subs);
				//printf("p2\n");
				for (unsigned int j=0;j<subs.size();j++)
				{
					//printf("Getting subnodes of id %d\n",id);
					std::string name;
					//rc->RutinaReadNode(subs[j],name);
				//	printf("-->%s\n",name.c_str());
					laux.push_back(name);
				}
			}
			res.ref.push_back(laux);
		}

}

void HPWA::GetPredSucessors(refpred pred,refpred &res, long type,long level)
{
/*
	long l;


	//rc->RutinaGetLevelofNode(pred.ref[0][0],l);
	if (l!=level)
	{
		GetPredSucessorsAux(pred,res,type);
		//printf("------\n");
		//printf("%s\n",res.name.c_str());
		//PrintTabla(res.ref);
		//printf("------\n");


		//rc->RutinaGetLevelofNode(res.ref[0][0],l);
		while (l!=level)
		{
			GetPredSucessorsAux(res,res,type);
			//rc->RutinaGetLevelofNode(res.ref[0][0],l);
		}
	}

*/

}
HPWA::HPWA(COpenMORAApp::CDelayedMOOSCommClient &Comms):m_Comms(Comms)
{


}

//----------------------------------------------------------------------------
// HPWAInit() Initialization of available actions and their compatibility
//----------------------------------------------------------------------------
void HPWA::HPWAInit(std::string actions_names,std::vector<std::string> comp_list)
{
	//Check that all actions have compatibility definitions
	std::deque<std::string> list_names;
	mrpt::system::tokenize(actions_names,"@",list_names);
	if (comp_list.size()!=list_names.size())
	{
	    std::cout
			<< comp_list.size() << "," << list_names.size() << std::endl
			<< "[HPWA] Error while initializing compatibility table (#actions names differs from #actions)\n";
		throw ("[HPWA] Error while initializing compatibility table");
	}

	//Save actions and compatibilities into HPWA table
	for (size_t i=0;i<comp_list.size();i++)
	{
		lista comp;
		printf("%s,%s\n",list_names[i].c_str(),comp_list[i].c_str());
		comp.push_back(list_names[i]);
		comp.push_back(comp_list[i]);
		Compatibility.push_back(comp);
	}

	//Print 
	printf("Action Compatibility Table\n==========================\n[");
	PrintTabla(Compatibility);
	printf("]\n");

	//Initialize parameters
	pet_map.clear();			//Clear information petitions
	pet_id=0;					//Reset Nº of information petitions.
	asked_petitions.clear();	//Clear list of ending petitions
	answer_map.clear();	
	CurrentPlanning.clear();
}


void HPWA::Init(void)
{

}


void HPWA::LoadGraphforTests()
{
//	unsigned error;
	printf("Loading Graph [%s]\n",AHGRAPH.c_str());
	//rc->RutinaLoadAHGraph(AHGRAPH.c_str(),error);



	lista l2;

	tabla goal;
	planningcost cost;
	planlist p;

	l2.push_back("at");
	l2.push_back("Robot");
	l2.push_back("n5");

	goal.push_back(l2);

	HPWA1("",goal,DETALLE,MARCA,p,cost,true,0,true);

	printf("---------Refining plan\n");

	listarefined ref;
	unsigned int i;
	for (i=1;i<p.planes.size();i++)
	{

		RefinePlans(p.planes[i], ref, DETALLE,-1,p.levels[p.planes.size()-1]);



		for (unsigned int i=0;i<ref[i].ref.size();i++)
		{
			printf("==================\n");
			printf("%s ",ref[i].name.c_str());
			PrintTabla(ref[i].ref);
			printf("==================\n");
		}

	}


}

void HPWA::Test()
{

	printf("Testing\n");

}


//////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////MAIN METHODS//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool HPWA::FinishedCommand(long stamp)
{

	//comm_sem.enter();
	bool res=false;
	for (unsigned int i=0;i<CurrentParallelCommands.size();i++)
	{
		if (CurrentParallelCommands[i].timestamp==stamp)
		{
			CurrentParallelCommands.erase(CurrentParallelCommands.begin()+i);
			res=true;
			break;
		}
	}
//	comm_sem.leave();
	return res;
}

//-----------------------------------------------------------
// IsCompatible()
// Chek if provided task is compatible with current planning
//------------------------------------------------------------
bool HPWA::IsCompatible(size_t id,std::string command,bool parallel)
{
	bool executable=false;
	bool compatible=false;
	lista lcomp;
	std::string current;

	//comm_sem.enter();

	//Is task already planned?
	if (CurrentPlanning.find(id)!=CurrentPlanning.end()) return false;

    
	return true;    // JGMonroy: Is this a temporal fix??
	//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    

	for (size_t i=0;i<CurrentParallelCommands.size();i++)
	{
		if (id==size_t(CurrentParallelCommands[i].timestamp) )
		{
			printf("Task %d already pending to be executed\n",(int)id);
			return false;
		}
	}


	if (CurrentParallelCommands.size()==0) executable=true;

	else if (parallel)
	{
		for (unsigned int i=0;i<CurrentParallelCommands.size();i++)
		{
			current=CurrentParallelCommands[i].command;

			GetListFromTabla(Compatibility,lcomp,current);

			//	printf("Checking compatibility between %s and %s\n",command.c_str(),current.c_str());
			//	printf("Testing %s\n",lcomp[0].c_str());


				if (strstr(lcomp[0].c_str(),command.c_str()) !=NULL)
				{
					compatible=true;
					executable=true;
					printf("%s is compatible with %s\n",command.c_str(),current.c_str());

				}
				else
				{
					printf("%s is NOT compatible with %s\n",command.c_str(),current.c_str());
					executable=false;
					break;
				}
/*
			if (strstr(lcomp[0].c_str(),command.c_str()) !=NULL)
			{
				//printf("%s is compatible with %s\n",command.c_str(),current.c_str());
				compatible=true; break;
			}
			else {
					//printf("%s NONONO is compatible with %s\n",command.c_str(),current.c_str());
				executable=false;
			}
*/
		}
	}
	else printf("%s es NO compatible since it is not PARALELIZABLE!!!\n",command.c_str());
	//comm_sem.leave();
	return executable;
}

//-----------------------------------
// PendingPetitions()
// Returns the number of pending petitions
//-----------------------------------------
bool HPWA::PendingPetitions()
{
	return (asked_petitions.size()>0);
}


//----------------------------------------------------------------------------
// GetInformation(what, response, timeout)
// Method to request information from other modules (trough OpenMORA variables)
// what = String with the OpenMORA variable to request.
// ej: GET_PATH node
// ej: GET_PERSON_LOCATION userID
//------------------------------------------------------------------------------
void HPWA::GetInformation(const std::string what, std::string &value, double timeout)
{	
	pet_sem.enter();	
		pet_id++;												//Increase the counter of information petitions (unique identificator)
		pet_map.insert(pet_pair(pet_id,what));					//Add the new petition to the stack (pet_ID, what)
		asked_petitions.insert(pet_id);							//Add petition to list of pending questions (controled by the Iterate process of the Planer)
		printf("[HPWA:GetInformation] New petition request id=%d: '%s'\n",pet_id, what.c_str());
	pet_sem.leave();

	//Wait until petition is answered. Search by petitionID
	bool end = false;
	std::map<size_t,std::string>::iterator  pet_it;
	while (!end)
	{
		mrpt::system::sleep(100);
		pet_sem.enter();
			pet_it = answer_map.find(pet_id);	//check if petition is solved.
			if (pet_it != answer_map.end())
			{
				//Petition was solved.
				value = pet_it->second;			//Response to petition = "params of response"
				printf("[HPWA:GetInformation] Petition id=%d solved: Response = %s\n",pet_id,value.c_str() );
				
				pet_map.erase(pet_id);			//Remove petition
				answer_map.erase(pet_it);
				end=true;
			}
		pet_sem.leave();
	}
}

//--------------------------------------------------------------------------------
// SelectTasktobePlanned()
// This method takes a list of pending tasks TASK_LIST = Task1#Task2#Task3... 
// where each "Task_i" is defined by: "UserID  TaskID  UserTaskID  Command  Task"
// and Schedule the one with highest priority
//--------------------------------------------------------------------------------
bool HPWA::SelectTasktobePlanned(std::string tasklist, std::string &command, std::string &task, size_t &taskid, std::string &address, size_t &localtaskid)
{

	//Define parameters
	std::vector<size_t> listid;
	std::vector<size_t> listlocalid;
	std::vector<int> listprio;
	lista listaddress;
	lista listcommands;
	lista listargs;
	std::vector<bool> listparallel;

	// Check all pending tasks (from the Agenda)
	//-------------------------------------------
	std::deque<std::string> lista,lista2;
	mrpt::system::tokenize(tasklist,"#",lista);
	for (size_t i=0;i<lista.size();i++)			
	{
		//Get the parameters of each Task: "UserID  TaskID  UserTaskID  Command  Task"
		mrpt::system::tokenize(lista[i]," ",lista2);

		//if (parallel || CurrentParallelCommands.size()==0)	//i.e. if the i-th task is parallel....
		listaddress.push_back(lista2[0]);						//address = UserID
		listid.push_back(atoi(lista2[1].c_str()));				//taskid = TaskID
		listlocalid.push_back(atoi(lista2[2].c_str()));			//localtaskid = UserTaskID
		listcommands.push_back(lista2[3]);						//command = Command

		//Unify the Task parameter
		std::string arg="";
		for (size_t k=4;k<lista2.size();k++)
		{
			arg=arg+lista2[k]+" ";
		}
		listargs.push_back(arg);								//arguments = Task
		//missing by now parallel and priority;
		listprio.push_back(1);									//priority = 1
		listparallel.push_back(true);							//parallel = true
	}


	// Search for the task with highest priority and compatible
	//-----------------------------------------------------------
	int maxprio=-1;
	int pos=-1;
	for (unsigned int i=0;i<listprio.size();i++)
	{
		//Chek if task is compatible
		bool comp = IsCompatible(listid[i],listcommands[i],listparallel[i]);
		//If yes...
		if (listprio[i]>=maxprio && comp)
		{
			maxprio = listprio[i];
			pos = i;
		}
	}

	// Schedule the task with highest priority
	//------------------------------------------
	if (pos!=-1)
	{
		address = listaddress[pos];
		taskid = listid[pos];
		command = listcommands[pos];
		task = listargs[pos];
		localtaskid = listlocalid[pos];
		printf("Adding to currentplanning %d from %s %d\n",(int)taskid,address.c_str(),(int)localtaskid);
		CurrentPlanning.insert(taskid);
		return true;
	}
	return false;
}


//------------------------------------------------------------------------------
// SolveTask ()
// This method is called on a different thread for each planned task
// It implements the task itself, attending to the Command parameter, 
// generating a list of "actions" called "plan" to be executed in the "Executor" module
// It returns true if the task could be executed, false if not implemented.
//-------------------------------------------------------------------------------
bool HPWA::SolveTask(std::string taskowner, size_t taskid, size_t localtaskid, std::string command, std::string task)
{
	tabla plan;			//List of actions to perform according to the Commanded task
	tabla goal;
	lista action,param;
	bool solved=false;
	action.clear();
	param.clear();
	plan.clear();		//Clear plan to populate with new actions
	goal.clear();

	printf("[Planner:SolveTask] Task to be planned (%ld) [%s] [%s]\n",taskid, command.c_str(), task.c_str());

	//Chek the parameters (arguments) of the given task
	char auxtask[255];
	strcpy(auxtask, task.c_str());
	char* token = strtok((char*)auxtask," ");
	while (token!=NULL)
	{
		param.push_back(token);
		token=strtok(NULL," ");
	}
	int num_param = param.size();


	//---------------------------------------------------
	// TASKS WITH ARGUMENTS
	//---------------------------------------------------
	if (num_param>0)
	{
		// PRESENCE_ALERT(node_label):
		// Moves to node_label and alert of unautorized presence
		if (command=="PRESENCE_ALERT")
		{
			// Check if there exists a path in the WorlModel topology to the given node_label
			// If YES, move to it
			solved = PathSearch((char*)param[0].c_str(), plan);
			
			action.push_back("SAY");
			action.push_back("Detectada presencia en posicion " + param[0]);
			plan.insert(plan.begin(),action);
			
			action.clear();
			action.push_back("PUBLISH");
			action.push_back("STOP_STATECHART");
			plan.insert(plan.begin(),action);
			
			action.clear();
			action.push_back("SAY");
			action.push_back("Buscando persona no autorizada");
			plan.push_back(action);

			action.clear();
			action.push_back("PUBLISH");
			action.push_back("RESUME_STATECHART");
			plan.push_back(action);
		}

		// GAS_ALERT(position): 
		// Moves to node_label and alert of gas contamination
		else if (command=="GAS_ALERT")
		{
			// Check if there exists a path in the WorlModel topology to the given node_label
			// If YES, move to it
			solved = PathSearch((char*)param[0].c_str(),plan);
			action.push_back("SAY");
			action.push_back("Detectada fuga de gas en posicion " + param[0]);			
			plan.insert(plan.begin(),action);
			
			action.clear();
			action.push_back("PUBLISH");
			action.push_back("STOP_STATECHART");
			plan.insert(plan.begin(),action);
			
			action.clear();
			action.push_back("SAY");
			action.push_back("Buscando origen de la fuga, por favor, aléjense de esta zona como medida de seguridad");
			plan.push_back(action);
			
			action.clear();
			action.push_back("PUBLISH");
			action.push_back("RESUME_STATECHART");
			plan.push_back(action);
		}

		// PUBLISH(Variable Params) : Publish the OpenMORA variable
		else if (command=="PUBLISH")
		{
				action.push_back("PUBLISH");
				action.push_back(task);
				plan.push_back(action);
				solved=true;
		}

		// SAY(phrase): Say something
		else if (command=="SAY")
		{
				action.push_back("SAY");
				action.push_back(task);
				plan.push_back(action);
				solved=true;
		}

		// SAY_TO(person phrase)
		else if (command=="SAY_TO" && num_param>=2)
		{
			std::string result="";
			//Get Location of person to which speak. Result can be "NOTFOUND" or "node_label"
			GetInformation(mrpt::format ("GET_PERSON_LOCATION %s",param[0].c_str()),result);
			
			if (result!="NOTFOUND")
			{
				//If user found, navigate to its location
				solved = PathSearch((char*)result.c_str(),plan);
				
				if (solved)
				{					
					//Get phrase to SAY
					std::string text = task.substr((size_t)1 + task.find(" "));
					printf("Say [%s] to [%s]\n",text.c_str(),param[0].c_str());

					action.clear();
					action.push_back("LOOK_AT");
					action.push_back(param[0]);
					plan.push_back(action);

					action.clear();
					action.push_back("SAY");
					action.push_back(text);
					plan.push_back(action);
				}
				else
					printf("[HPWA:SolveTask:SAY_TO]Error: No path found to %s in the topology.\n",param[0].c_str());				
			}
			else printf("[HPWA:SolveTask:SAY_TO]Error: %s is %s in the topology.\n",param[0].c_str(),result.c_str());				
		}

		// PLAY_ITEM(item)
		else if (command=="PLAY_ITEM")
		{
				action.push_back("PLAY");
				action.push_back(param[0]);
				plan.push_back(action);
				solved=true;
		}

		//LOOK_AT(point)
		else if (command=="LOOK_AT")
		{
				action.push_back("LOOK_AT");
				action.push_back(param[0]);
				plan.push_back(action);
				solved=true;
		}

		//SWITCH_ON()
		else if (command=="SWITCH_ON")
		{
				action.push_back("SWITCH_ON");
				action.push_back(param[0]);
				plan.push_back(action);
				solved=true;
		}

		//SWITCH_OFF()
		else if (command=="SWITCH_OFF")
		{
				action.push_back("SWITCH_OFF");
				action.push_back(param[0]);
				plan.push_back(action);
				solved=true;
		}

		//WAIT_TIME()
		else if (command=="WAIT_TIME")
		{
				action.push_back("WAIT_TIME");
				action.push_back(param[0]);
				plan.push_back(action);
				solved=true;
		}

		//WAIT_FOR_SIGNAL()
		else if (command=="WAIT_FOR_SIGNAL")
		{
				action.push_back("WAIT_FOR_SIGNAL");
				action.push_back(param[0]);
				plan.push_back(action);
				solved=true;
		}

		//WAIT_FOR_VBLE_VALUE()
		else if (command=="WAIT_FOR_VBLE_VALUE" && num_param==2)
		{
				action.push_back("WAIT_FOR_VBLE_VALUE");
				action.push_back(param[0]);
				action.push_back(param[1]);
				plan.push_back(action);
				solved=true;
		}
		
		//GO_TO_RECHARGE()
		else if (command=="GO_TO_RECHARGE")
			{
				//Go to Docking node!
				solved = PathSearch("Docking",plan);

				if (solved)
				{
					action.clear();
					action.push_back("PUBLISH");
					action.push_back("PARKING 1");
					plan.push_back(action);
					
					action.clear();
					if (atof(param[0].c_str())>0)
					{
						action.push_back("WAIT_TIME");
						action.push_back(param[0]);
					}
					else
					{
						action.push_back("WAIT_FOR_SIGNAL");
						action.push_back("Complete_Recharge");
					}					
					plan.push_back(action);
				}
				else
					printf("[HPWA:SolveTask:GO_TO_RECHARGE]Error: No path found to 'Docking' in the topology.\n");
			}

		//MOVE(node_label)
		else if (command=="MOVE")
			{
				//Check if node_label is a user_label
				std::string result;
				GetInformation(mrpt::format ("GET_PERSON_LOCATION %s",param[0].c_str()),result);
				
				if (result!="NOTFOUND")
				{  
					//The node_label is a user, try navigating to its location
					std::cout << "[Planner] Navigating to USER node" << std::endl;
					solved = PathSearch((char*)result.c_str(),plan);

					action.clear();
					action.push_back("LOOK_AT");
					action.push_back(param[0]);
					plan.push_back(action);				
				}
				else 
				{
					//The node_label is not a user, try to navigate to it
					std::cout << "[Planner] Navigating to TOPOLOGY node" << std::endl;
					solved = PathSearch((char*)param[0].c_str(),plan);
				}
			}

		// Command not recognized
		else
		{
			solved=false;
			printf("[HPWA:SolveTask] Command described in Task (%u) not implemented!", taskid);
		}
	}//end num_param>0

	//---------------------------------------------------
	// TASKS WITH NO ARGUMENTS
	//---------------------------------------------------
	else
	{
		if (command=="UNDOCK")
		{
			action.clear();
			action.push_back("PUBLISH");
			action.push_back("PARKING 0");
			plan.push_back(action);

			action.clear();
			action.push_back("PUBLISH");
			action.push_back("ENABLE_MOTORS 1");
			plan.push_back(action);

			action.clear();
			action.push_back("PUBLISHF");
			action.push_back("MOTION_CMD_V -0.1");
			plan.push_back(action);

			action.clear();
			action.push_back("WAIT_TIME");
			action.push_back("0.07");
			plan.push_back(action);
				
			action.clear();
			action.push_back("PUBLISHF");
			action.push_back("MOTION_CMD_V 0.0");
			plan.push_back(action);

			/*
			action.clear();
			action.push_back("PUBLISHF");
			action.push_back("MOTION_CMD_W 0.53");
			plan.push_back(action);

			action.clear();
			action.push_back("WAIT_TIME");
			action.push_back("0.15");
			plan.push_back(action);
					
			action.clear();
			action.push_back("PUBLISHF");
			action.push_back("MOTION_CMD_W 0.0");
			plan.push_back(action);
			*/
			solved=true;			
		}

		else if (command=="SAY_TIME")
		{
			std::string horatext;
			Hora(horatext);
			action.push_back("SAY");
			action.push_back(horatext);
			plan.push_back(action);
			solved=true;
		}

		else if (command=="PLAY_ITEM")
		{
			action.push_back("PLAY");
			//action.push_back("test.odp");
			plan.push_back(action);
			solved=true;
		}

		else if (command=="PAUSE_MOVE" 
			  || command=="RESUME_MOVE" 
			  || command=="SAY_WEATHER" 
			  || command=="SAY_DATE" 
			  || command=="WAIT_TO_START" 
			  || command=="START" 
			  || command=="END"
			  || command=="WAIT_TO_PRESENT"
			  || command=="PRESENT" 
			  || command=="SAY_JOKES" 
			  || command=="END_SAY_JOKES" 
			  || command=="WAIT_TO_GO")
		{
			action.push_back(command);
			plan.push_back(action);
			solved=true;
		}
		
		//Command not implemented!
		else
		{
			printf("Task command NOT recognized\n");
			solved=false;
		}
	}//end Commands


	// If task command found, execute the corresponding actions in the "plan"
	//-----------------------------------------------------------------------
	if(solved)
	{
		// Print Plan (list of actions)
		printf("----------\n");
		PrintTabla(plan);
		printf("----------\n");

		//Get the current Plan (list of actions to perform) as a string
		std::string solution;
		SPrintPlan(plan,solution);

		comm_sem.enter();
			// Add Command to list of "in execution"
			ExecutedCommand ec;
			ec.timestamp = taskid;		//JGMonroy: timestamp = taskID ????
			ec.command = command;
			CurrentParallelCommands.push_back(ec);

			// Send plan (list of actions) to the Executor module.
			//!  @moos_publish IN_EXECUTION The parameters (taskowner, taskid, localtaskid, solution) of a task that is being executed.
			//!  @moos_var IN_EXECUTION Variable published each time a new task is executed by the planner, to inform the Executor module.
			//!  It contains all the necessary parameters defining the task itself and the corresponding actions.
			//!  FORMAT: IN_EXECUTION taskowner#taskid#localtaskid#listOfActions
			//!  where listOfActions = ACTION_NAME_1 param1_1 param1_2 .. param1_i#ACTION_NAME_2 param2_1 param2_2 .. param2_i
			m_Comms.Notify("IN_EXECUTION", mrpt::format("%s#%d#%d#%s",taskowner.c_str(),(int)taskid,(int)localtaskid,solution.c_str()));
		comm_sem.leave();
	}
	else if (!solved)
	{
		printf("[Planner: HPWA]FAILURE:Task Command not found or imposible to solve.\n");

		//Inform that the task has finished
		//! @moos_publish PLAN_FINISHED  Variable that inform that a Plan (task) has finished.
		//! @moos_var PLAN_FINISHED  Variable that inform that a Plan (task) has been completely executed.
		//! Format: PLAN_FINISHED TaskID UserTaskID UserID
		m_Comms.Notify("PLAN_FINISHED",mrpt::format("%u %u %s",(int)taskid,(int)localtaskid,taskowner.c_str() ));
	}

	//mrpt::system::sleep(1000);
	//CurrentPlanning.erase(taskid);

	return solved;
}


void HPWA::SHPWA1(char *op,tabla goal,ltabla &listaplanes,planningcost &cost,bool verbose)
{

	planlist p;
	planningcost cost1,cost2;
	bool res;

	if (verbose) printf("\n-----------------SHPWA1-----------------\n");

	res=HPWA1(op,goal,GENERALIZACION,MARCA,p,cost1,true,1,verbose);

	if (res)
	{
		//rc->RutinaDeleteAllNodeMarks();
		//PrintLTabla(p.planes);
		for (unsigned int i=0;i<p.planes.size();i++)
		{
			MarcaPlan(p.planes[i],p.levels[i],GENERALIZACION,SPECIALMARK);
		//Here only those nodes from the detail hierarchy which are subnodes of nodes involved in the
		//semantic plan are marked with the "special mark"

		}
	}

	HPWA1(op,goal,DETALLE,SPECIALMARK,p,cost2,!res,0,verbose);

	//PrintLTabla(p.planes);
	SumCost(cost1,cost2);
	AssignCost(cost,cost1);

}

//----------------------------------------------------
// PathSearch()
// Checks if there exists a path between the node robot and the given "dest" in
// the WorlModel: GET_PATH dest_node_label
//----------------------------------------------------
bool HPWA::PathSearch(char *dest, tabla &plan, bool verbose, bool execute)
{
	lista path;
	bool solved = false;	
	if (verbose) 
		printf("---------SEARCHING PATH AT GROUND LEVEL -------------\n");
	
	//Ask the WorldModel module if such path exists: GET_PATH request
	std::string result;
	GetInformation(mrpt::format ("GET_PATH %s",dest), result);		//result = "PATH response"
	printf("[HPWA:PathSearch]: path is \n[%s]\n",result.c_str());
	
	std::deque<std::string> tok;
	mrpt::system::tokenize(result," ",tok);

	// result = OpenMORA Variable PATH after removing the petitionID
	// Path = "NOTFOUND"  (If node or path not found in the topology)
	// Path = "(nodeID1 nodeLabel1 nodeX1 nodeY1) (nodeID2 nodeLanel2 nodeX2 nodeY2)..."	
	
	if (tok.size()==1 && tok[0]=="NOTFOUND")
		solved = false;
	else
	{
		solved = true;
		//Generate sequence of actions, to run the path
		for (size_t i=0;i<tok.size();i+=4)
		{
			lista action;
			action.push_back("GO");		//GO nodel_label X Y
			action.push_back(tok[i+1]);
			action.push_back(tok[i+2]);
			action.push_back(tok[i+3]);
			//	printf("go to %s %s %s %s\n",tok[i].c_str(),tok[i+1].c_str(),tok[i+2].c_str(),tok[i+3].c_str());
			plan.push_back(action);
		}
	}

	printf ("plan is solved\n");
	//	PrintTabla(plan);
	return solved;
}



bool HPWA::FlatPlan(char *op,long level,tabla goal,tabla &plan,planningcost &cost,bool verbose,bool execute)
{
	if (verbose) printf("---------PLANNING AT GROUND LEVEL (WITHOUT HPWA) -------------\n");

	clock_t inicio,tfin;
	//clock_t inicio2,tfin2;
	inicio=clock();

	if (level==-1)
	{
		printf("Get Ground Level\n");
		//rc->RutinaGetGroundLevel(level);
	}


	printf("Load Ops\n");
	//rc->RutinaLoadOp(OPERATIONS.c_str());



	double tiempo=0;
	int num_nodos=0;
	ltabla mundo;

	//inicio2=clock();
	printf("Get Predicates\n");
	//rc->RutinaGetPredicados(level,false,MARCA,mundo);
	//tfin2=clock();
	//printf("Tiempo total GetPredicados %f\n", (double)(tfin2-inicio2)/CLOCKS_PER_SEC);

	mundo.push_back(goal);
//	int error;


		if (verbose)
		{
			printf("%%%%%%%%%%%%%%%%%%%%%%%%%%==================\n");
			PrintLTabla(mundo);
			printf("%%%%%%%%%%%%%%%%%%%%%%%%%%==================\n");
			//FPrintLTabla(log,mundo);

		}


	//rc->RutinaLoadProblem(mundo,error);

	//rc->RutinaSolveProblem(plan,tiempo,num_nodos);

	tfin=clock();

	if (verbose)
	{
		PrintTabla(plan);
		printf("===============================\n");
		printf("Tiempo total del algoritmo %f\n", (double)(tfin-inicio)/CLOCKS_PER_SEC);
		printf("Tiempo del planificador: %f\n",tiempo);
		printf("Numero de nodos creados: %d\n",num_nodos);
	}

	cost.embededtime=tiempo;
	cost.num_calls=1;
	cost.num_nodes=num_nodos;
	cost.totaltime=(double)(tfin-inicio)/CLOCKS_PER_SEC;

	/////Inquiring the execution of the resultant plan
	if (execute && plan.size()>0)
	{
		if (verbose) printf("Sending Plan to PLEXAM\n");
		//rc->RutinaSendPlantobeExecuted("",plan,0);
	}
	else if (execute && plan.size()==0)
	{
		//rc->RutinaSpeech("Lo siento, plan no encontrado");

	}
	return (plan.size()>0);
	printf("End of FlatPlanning\n");

}




bool HPWA::HPWA1(char *op,tabla goal,int type,int marca,planlist &listaplanes,planningcost &cost,bool clear,long min_level,bool verbose)
{
	clock_t inicio,tfin;
//	clock_t inicio2,tfin2;
	inicio=clock();
	bool res=true;

	if (verbose && clear) printf("---------PLANNING WITH HPWA-1 -------------\n");

	ltabla pred_anc;
	std::vector<long> levels;

	//printf("Loading Operations\n");
	//rc->RutinaLoadOp(OPERATIONS.c_str());

	//printf("Getting Ancestors\n");
	GetPredAncestors(goal,pred_anc,type);

	if (verbose)
	{
		PrintLTabla(pred_anc);

	}
	//rc->RutinaAllLevels(levels,type);
	int cnivel=pred_anc.size();
	bool usamarca=!clear;
	double actiempo=0;
	int acnum_nodos=0;
	int num_veces=0;
	bool trivial;
	listaplanes.planes.clear();
	listaplanes.levels.clear();


	if (clear)
		//rc->RutinaDeleteAllNodeMarks();

	for (int nivel=cnivel-1;nivel>=min_level;nivel--)
	{
		ltabla mundo;
		//printf("Getting predicates from level %d\n",levels[nivel]);
//		inicio2=clock();
		//rc->RutinaGetPredicados(levels[nivel],usamarca,marca,mundo);
//		tfin2=clock();
//		printf("Tiempo total GetPredicados %f\n", (double)(tfin2-inicio2)/CLOCKS_PER_SEC);
		//if (nivel!=0) //rc->RutinaClearMarks(levels[nivel-1]);
		mundo.push_back(pred_anc[nivel]);

		if (verbose)
		{
			printf("%%%%%%%%%%%%%%%%%%%%%%%%%%==================\n");
			PrintLTabla(mundo);
			printf("%%%%%%%%%%%%%%%%%%%%%%%%%%==================\n");
			//FPrintLTabla(log,mundo);

		}

		trivial=false;
		//rc->RutinaPlanIsTrivial(mundo,trivial);
		if (!trivial)
		{
			double tiempo=0;
			int num_nodos=0;
//			int error;
			tabla plan;
			//rc->RutinaLoadProblem(mundo,error);
			num_veces++;
			//rc->RutinaSolveProblem(plan,tiempo,num_nodos);

			actiempo=actiempo+tiempo;
			acnum_nodos=acnum_nodos+num_nodos;

			if (plan.size()!=0) usamarca=true;
			else usamarca=false;

			if (verbose)
			{
				printf("Plan at level %d (%d)\n ",nivel,num_nodos);
				PrintTabla(plan);
			}


			listaplanes.planes.push_back(plan);

			listaplanes.levels.push_back(levels[nivel]);

			if (clear) MarcaPlan(plan,levels[nivel],type,marca);


		}
		else
		{
		//	printf("Plan is trivial\n\n");
			//fprintf(log,"Plan is trivial\n");
			res=false;
			usamarca=false;
		}

	}
	tfin=clock();

	if (verbose)
	{
		printf("===============================\n");
		printf("Tiempo total del algoritomo %f\n", (double)(tfin-inicio)/CLOCKS_PER_SEC);
		printf("Tiempo del planificador: %f\n",actiempo);
		printf("Numero de llamadas al planificador: %d\n",num_veces);
		printf("Tiempo acotado del planificador: %f\n",actiempo+(num_veces*0.001));
		printf("Numero de nodos creados: %d\n",acnum_nodos);
		printf("-----------\n");
//		fflush(stdin);
//		char c[1];
//		gets(c);
	}


	cost.embededtime=actiempo;
	cost.num_calls=num_veces;
	cost.num_nodes=acnum_nodos;
	cost.totaltime=(double)(tfin-inicio)/CLOCKS_PER_SEC;
	return res;
}



bool HPWA::RefinePlans(tabla plan, listarefined &listaref,int type,int marca,long level,bool verbose)
{
	bool res=true;
	listaref.clear();
	refpred refined;


	printf("Refining Plan\n");
	PrintTabla(plan);
	for (unsigned int i=0;i<plan.size();i++)
	{

		refpred refaction;
		refaction.ref.clear();
		refaction.name=plan[i][0];

		lista l;
		for (unsigned int j=1;j<plan[i].size();j++)
		{
			l.clear();
			l.push_back(plan[i][j]);
			refaction.ref.push_back(l);
		}



		GetPredSucessors(refaction,refined,type,level);
		listaref.push_back(refined);

	}

	return res;
}



bool HPWA::AbstractPlans(listarefined gplan,listarefined &plan,int type,int marca,long level,bool verbose)
{
	bool res=true;
	plan.clear();
	refpred abs;

	for (unsigned int i=0;i<gplan.size();i++)
	{
		refpred absaction;
		ltabla ancestor;
		absaction.ref.clear();
		absaction.name=gplan[i].name;

		GetPredAncestors(gplan[i].ref,ancestor,type);
		PrintLTabla(ancestor);



	}

	return res;
}



void HPWA::InquirePlanExecution(tabla plan)
{
	//rc->RutinaSendPlantobeExecuted("",plan,0);

}












