#include <vector>
#include <string>
#include <deque>
#include <time.h>
#include <CMapirMOOSApp.h>
#include <mrpt/base.h>

typedef std::vector<std::string> lista;
typedef std::vector<lista> tabla;
typedef std::vector<tabla> ltabla;

typedef std::pair<size_t,std::string> pet_pair;

typedef struct
{
	double embededtime;
	double totaltime;
	long num_calls;
	long num_nodes;
} planningcost;

typedef struct
{
	ltabla planes;
	std::vector<long> levels;
} planlist;

typedef struct
{
	std::string name;
	tabla ref;
} refpred;

typedef std::vector<refpred> listarefined;

typedef struct
{
	long timestamp;
	std::string command;

}ExecutedCommand;



#define MARCA 5
#define SPECIALMARK 8
#define INI_FILE_NAME ".\\Planning.ini"
//#define MAX_BUFFER 255

//#define OPERATIONS "d:\\Code\\Planning\\HPWA\\Release\\dominio.pddl"
//#define AHGRAPH "d:\\Code\\Planning\\HPWA\\Release\\Real.gph"

#define DETALLE 1
#define GENERALIZACION 2

//////arcs types
#define NAVTYPE 1
#define ATTYPE 2
#define INTYPE 3


///////opcode
/*#define SAY 1
#define TAKE 2
#define MOVE 3
#define BRING 4
#define TALK 5
#define PLAY 6
*/

typedef struct {std::string id;float x,y,phi;std::string descrip;void *sig;} position;


class HPWA
{

public:

	CMapirMOOSApp::CDelayedMOOSCommClient &m_Comms;

	HPWA(CMapirMOOSApp::CDelayedMOOSCommClient &m_Comms);

	void HPWAInit(std::string actions_name,std::vector<std::string> comp_list);



	bool IsCompatible(size_t,std::string command,bool parallel);
	bool SelectTasktobePlanned(std::string tasklist,std::string &command,std::string &task,size_t &taskid,std::string &address,size_t &localtaskid);
	//bool SolveTask(std::string tasklist,size_t &id,std::string &taskselected,std::string &taskowner,std::string &solution);
	bool SolveTask(std::string taskowner,size_t taskid,size_t localtaskid,std::string command,std::string task);
	bool FinishedCommand(long stamp);

	//////////////////////////////////////////////////////
	//Flat planning at a certain level
	//If level==-1 it performs FlatPlanning at the ground level of the MAHG.
	//if execute=true it send the resultant plan to be executed by RutinaSendPlantobeExecuted
	//return true if a plan was found
	bool FlatPlan(char *op,long level,tabla goal,tabla &plan,planningcost &cost,bool verbose=false,bool execute=false);



	//////////////////////////////////////////////////////
	//HPWA1 method
	// parameters:
	// op -> file with the operation definitions
	// goal -> tabla with the set of goals
	// type -> type of abstractions to be considered in the hierarchical planning
	// marca -> mark, a number, which will be used to mark subnodes of relevant elements
	// listaplanes-> resultant plans (one per level)
	// plancost -> cost of planning
	// clear -> indicates wether previous marks should be erased or not
	// level -> indicates the last level to plan (0 indicates the ground level, 1 is the first level, etc.)
	// verbose -> if true some information is displayed
	// It returns true if everything went ok. False if the goal to be plan is trivial

		bool HPWA1(char *op,tabla goal,int type,int marca,planlist &listaplanes,planningcost &cost,
		bool clear=true,long level=0,bool verbose=false);


		/////Semantic Planning ..... for testing.. it is in fact planning with more than 1 hierarchy
		void SHPWA1(char *op,tabla goal,ltabla &listaplanes,planningcost &cost,bool verbose=false);


	//////////////////////////////////////////////////////
	//Path Search at the ground level of the hierarchy of the current loaded AH-graph
	//if execute=true it send the resultant plan to be executed by RutinaSendPlantobeExecuted
		bool PathSearch(char *dest,tabla &plan, bool verbose=false, bool execute=false);




	//////////////////////////////////////////////////////
	// This method is run if DoTest = 1 in the init file
	// It is used to perform automatically planning tests
		void Test();

	//////////////////////////////////////////////////////
	//This method is automatically run when the babel module starts (it is called by a monitor service)
	//It is used typically for loading a particular graph file.
		void Init(void);

		void CreateWorld(int sizeplaces,int sizeobjects);

		void LoadGraphforTests();



	//////////////////////////////////////////////////////
	//RefinePlans method
	// parameters:
	// plan -> plan to be refined
	// listaplanes -> Refined plans obtained
	// type -> type of abstractions to be considered during the refinement
	// marca -> mark, a number, which will be used to consider subnodes (if -1 (default) all elements will be considered)
	// level -> indicates the last level to refine (0 indicates the ground level, 1 is the first level, etc.
			//within the abstraction chain of type "type")
	// verbose -> if true some information is displayed
	// It returns true if everything went ok. False if the goal to be plan is trivial
	bool RefinePlans(tabla plan, listarefined &listaref, int type,int marca=-1,long level=0,bool verbose=false);

	//////////////////////////////////////////////////////
	//ActractPlans method
	// parameters:
	// gplan -> general plan to be abstracted. It can include "meta-actions"
	// plan -> Abstract plan
	// type -> type of abstractions to be considered during the refinement
	// marca -> mark, a number, which will be used to consider subnodes (if -1 (default) all elements will be considered)
	// level -> indicates the last level to abstract (-1 indicates the universal levelground level, 1 is the first level, etc.
			//within the abstraction chain of type "type")
	// verbose -> if true some information is displayed
	// It returns true if everything went ok. False if the goal to be plan is trivial

	bool AbstractPlans(listarefined gplan,listarefined &plan,int type,int marca=-1,long level=-1,bool verbose=false);



	bool test;	//if true the Test() routine will be execute as a monitor by BABEL



	void GetInformation(const std::string what,std::string &value,double timeout=0);
	bool PendingPetitions();
	mrpt::utils::synch::CCriticalSection pet_sem;
	mrpt::utils::synch::CCriticalSection comm_sem;


	std::map<size_t,std::string>  pet_map;
	std::set<size_t>	asked_petitions;
	std::map<size_t,std::string>  answer_map;


private:
	size_t pet_id;


	std::vector<ExecutedCommand> CurrentParallelCommands;	//indicates the set of plans being executed currently.
	std::set<size_t> CurrentPlanning;					//list of tasks being planned but still not being executing
	mrpt::utils::synch::CCriticalSection plan_sem;

	tabla Compatibility;

	void GetListFromTabla(tabla t, lista &l, std::string key,bool verbose=false); //returns the set of compatible command with respect a given one (the key)

	FILE *log;
	std::string OPERATIONS; //char OPERATIONS[MAX_BUFFER];
	std::string AHGRAPH; //char AHGRAPH[MAX_BUFFER];
	std::string TASK; //char TASK[MAX_BUFFER];	//Default task to be planned


	void InquirePlanExecution(tabla plan);
	void GetPredAncestors(tabla post,ltabla &res,long type);

	void GetPredSucessorsAux(refpred pred,refpred &res, long type);
	void GetPredSucessors(refpred pred,refpred &res,long type,long level);

	void SPrintPlan(tabla t,std::string &solution);
	void PrintTabla(tabla t);
	void PrintLTabla(ltabla t);
	void PrintCost(planningcost c);
	void FPrintTabla(FILE *f,tabla t);
	void FPrintLTabla(FILE *f,ltabla t);
	void FPrintCost(FILE *f,planningcost c);

	void SumCost(planningcost &cost1,planningcost cost2); //cost1=cost1+cost2
	void AssignCost(planningcost &cost1,planningcost cost2); //cost1<-cost2
	void InitCost(planningcost &cost); //cost1<-0
	void AverageCost(planningcost &cost,int num); //cost=cost/num

	void MarcaPlan(tabla p,long level,long type,long marca);

	mrpt::synch::CCriticalSection sem_comms;

/*	void CreateRoom(char* name,int x,int y,int num,std::vector<long> &list);
	void CreateShelf(char* name,long loc,int x,int y,int num,std::vector<long> &list);
	void FillShelf(char* name, long shelf,int x,int y,int num,std::vector<long> &list,int opt=0);
	void CreateAbstraction(std::vector<long> list,long super,long type);
*/

};
