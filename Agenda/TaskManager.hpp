//(c) 2005 CGA
// Task Manager for ACHRIN


#include <string>
#include <vector>
#include <time.h>
#include <mrpt/system/datetime.h>
#include <mrpt/synch.h>


//Definition of tasks
//It includes:
//timestamp: system time that univocaly identifies the task.
//userid: identifier of the user who commands the task
//command: a string that identifies the type of task
//task: a string that represents the task
//priority: a priority value (0 non-priorized, 1 priorized) TODO
//parallel: indicates if the task can be executed in paralled with others (only if it is possible)

typedef struct
{
	mrpt::system::TTimeStamp timestamp;
	std::string userid;	//name of the client of the task
	size_t taskid;		//unique id for tasks independently of the user
	size_t usertaskid;  //particular id of the task given by the user
	std::string command;
	std::string task;
	int priority;
	bool parallel;
}tasktype;

typedef struct
{
	//void(*Rutina_NotifyNewTask)();
	//void(*Rutina_GetRandomDestination)(char *&dest,int &error);

}rcConfig;


extern "C"
{
//	unsigned ValorEnteroUniforme(unsigned v0, unsigned v1);
//	void IniciaSemillaAleatoria (long semilla);
//	float ValorAleatorioUniforme();
}


//#define TASKPLANNER 0
//#define TOPOLOGICINFO 1
//#define PLEXAM 2
//#define SEARCH 3

class TaskManager
{
	public:

		TaskManager();

		//Creates and returns a Task
		void * CreateTask(std::string userid,const char *command, const char *task, int &id, int priority=0,bool parallel=false);

		//Inserts a task into the agenda
		bool ReceiveTask(std::string userid,size_t usertaskid,const char *command,const char * goal,int priority=0,bool parallel=false);
		bool NewTask(void *task);

		//Returns the first task, first priorized tasks and then the rest in order of arrival
		//opcode is the code of the task,
		//task is the sequence of parameters for the opcode task
		bool GetLastTask(char *&opcode,char *&task,bool erase=true);  //obsolete function

		bool GetNextTask(std::string &userid,long &timestamp, char *&opcode, char *&task, int &prio, bool &parallel,bool reset=false);
		bool DeleteTask(size_t taskidtoerase);
		bool SearchTaskById(size_t taskid);
		bool GetTaskbytimestamp(long timestamp,std::string &userid, char *&opcode, char *&task, int &prio, bool &parallel);


		//Blocks the agenda to prevent the addition of new tasks
		void BlockAgenda();
		void UnBlockAgenda();

		//Eliminates all tasks stored in the agenda with priority less than prio. If prio=-1, eliminates all tasks
		void FlushAgenda(int prio=-1);

		//State of the Agenda
		bool IsEmpty();

		void PrintAgenda(std::string &out,bool verbose=true);


		void FPrintTask(tasktype task,bool added=true); //if added (default) a note is include in the log to indicate
		//that the task was added into the agenda.


		///////Geting and releasing user id's
		void GetUserLicense(int &t);
		void ReleaseUserLicense(int t);


		void SendEvent();

		void AgendaStatus(std::string &cad);


	private:

	 std::vector<tasktype> tasklist;
	 bool blocked;
	 int ticket;
	 int num_users;
	 std::vector<int> releasedtickets;
	 std::vector<std::string> errors;
	 FILE *log;
	 char fileLog[80];
	 long taskid;	//unique identifier for tasks

	 mrpt::synch::CCriticalSection  sem;

};
