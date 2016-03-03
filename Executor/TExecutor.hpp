#ifndef __TEXECUTOR
#define __TEXECUTOR
#include <vector>
#include <set>
#include <string>

#include <COpenMORAMOOSApp.h>

using namespace mrpt;
typedef std::pair<size_t,std::string> pet_pair;
typedef std::pair<std::string,bool> signal_pair;


class TaskExecutor
{

public:

	typedef struct
	{

		int acc;							//Indica la accion(grupo-> Manipulacion, Navegacion...)
		int alg;							//Indica el algoritmo para realizar la accion
		std::string name;					//Nombre de la accion
		std::vector<std::string> param;		//Lista de parametros para la accion

		long timestamp;

		std::string asString() const
		{
			std::string s = name;
			if (param.empty()) return s;
			s+=" [";
			for (size_t i=0;i<param.size();++i)
			{
				s+=param[i];
				if (i!=param.size()-1) s+=" ";
			}
			s+="]";
			return s;
		}

	}ActionInfo;


	typedef struct
	{
		size_t timestamp;					//global an unique identificator for the plan
		std::string address;				//name of the user who asked this plan
		size_t localtaskid;					//local id of the task for the user
		std::vector<ActionInfo> actions;	//list of actions in the plan

		std::string asString() const
		{
			std::string s = address;
			s+="#";
			for (size_t i=0;i<actions.size();i++)
			{
				s+=actions[i].asString();
				if (i!=actions.size()-1) s+=" & ";
			}
			return s;
		}

	}PlanInfo;

	COpenMORAApp::CDelayedMOOSCommClient &m_Comms;
	
	TaskExecutor (COpenMORAApp::CDelayedMOOSCommClient &m_Comms);
//	TaskExecutor (CMOOSCommClient &m_Comms);

	void LoadJokes(std::vector<std::string> jokes);
	//////////Plan and Simple actions execution
	void DoPlan(PlanInfo pi);
	void DoPlan(const std::string plan);
//	void DoSimpleAction(simple_actions sa);

	~TaskExecutor();

	/////////Alert Management
	void EndNavigation();   //Finalizacion de la navegacion con exito
	void FailedNavigation();//Finalizacion de la navegacion SIN exito

	void EndSpeak();

	void EndFaceDetection();
	void EndFaceRecognition();

	void PrintStatus();

	///////Miscelaneous
	void Prueba();	//Prueba de ejecucion de tareas de navegacion


	void GetRobotStatus(long &code,char** textstatus);
	void GetSystemStatus(char **textstatus);
	void GetNavigationDestination(std::string &pt,std::string &gt); //it yields the partial and global targets of the nav.

	unsigned int action_finished;	//indica si hay una notificacion de accion terminada

	bool cancel_current_plans;		//To cancel all current plans in executions (different threads)
	bool startednavigation;
	bool errornavigation;
	bool endnavigation;
	bool endjoke;

	bool endFaceDetection;			// Indicates the end of the face detection process (either positive or negative)
	bool resultOKFaceDetection;		// Indicates the result of the face detection process

	bool endFaceRecognition;			// Indicates the end of the face detection process (either positive or negative)
	bool resultOKFaceRecognition;		// Indicates the result of the face detection process
	std::string faceRecognitionLabel;

	bool end_vision_camshift;
	bool end_speak;

	PlanInfo plan;


	std::vector<PlanInfo> lplans;					//list of all plans that PLEXAM has to execute concurrently;
	unsigned inexecution;							//number of plans currently in execution
	std::vector<PlanInfo> lplansinexecution;		//list of all plans that PLEXAM has to execute concurrently;

	

	void LastExecutedPlan(long &timestamp);    	   //yields the last plan id executed
	void IsPlanExecuted(long timestamp,bool &res);   //yields true (in vble res) if a plan with a given timestamp has been executed

	void ExecutePlan(PlanInfo &p);			//executes the plan p


	void CopyPlan(PlanInfo &in, PlanInfo &out);
	void CopyPlanfromString(const std::string plan,PlanInfo &out);
    void PrintPlan(PlanInfo &plan);

	void GetInformation(const std::string what,std::string response,double timeout=0);
	bool PendingPetitions();
	mrpt::synch::CCriticalSection pet_sem;

	std::map<size_t,std::string>  pet_map;
	std::set<size_t>	asked_petitions;
	std::map<size_t,std::string>  answer_map;

	std::map<std::string,bool> signal_map;			//list of "signals" (Actived or not) for the WAIT_FOR_SIGNAL Action

	void FileCommands(std::string path);
	void Moving();
	std::string mission_directory;

private:
	std::string errorlist;   //errors separated by \n
	
	void ReportError(std::string error,bool erase=false);  //if erase=true, the list of errors is firstly cleared before
														//error is added.
	mrpt::synch::CCriticalSection sem_error;
	//mrpt::synch::CCriticalSection sem_comms;

	std::vector<std::string> joke_list;
	std::string target;							//target of the current navigational task
	std::string partial_target;					//partial target, that is, target of the currently executed navigation

	//ExecutePlans();
	std::vector<long> executedplans;


	size_t pet_id;


	bool start;
	bool present;
	bool ready;
	bool finishedppt;

    bool domotica;
    size_t texecutor_id;


	int action_index;
	int executed_index;
	int internal_status;
	char NODE_POSITION[255];
	char systemstatus[2048];



};




#endif
