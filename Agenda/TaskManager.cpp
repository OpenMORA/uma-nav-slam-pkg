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

#include "TaskManager.hpp"
#include <time.h>

using namespace mrpt;
using namespace mrpt::system;


TaskManager::TaskManager()
{
	//Initialize list of tasks
	tasklist.clear();
	errors.clear();

	blocked=false;
	ticket=0;
	num_users=0;
	releasedtickets.clear();

	Save_logfile = false;
	taskid=0;
}

void TaskManager::GetUserLicense(int &t)
{
	if (releasedtickets.size()==0)
	{
		t=ticket;
		ticket++;
		num_users++;
	}
	else
	{
		t=releasedtickets[0];
		releasedtickets.erase(releasedtickets.begin());
		num_users--;
	}
}


void TaskManager::ReleaseUserLicense(int t)
{

	releasedtickets.push_back(t);
}


//------------------------------------------------------------
// CreateTask()  Creates a new task and returns pointer to it
//-------------------------------------------------------------
void *TaskManager::CreateTask(std::string userid, const char *command, const char *task, int &id, int priority, bool parallel)
{
	sem.enter();
		tasktype *tt=new tasktype();
		tt->timestamp=getCurrentTime();
		tt->userid=userid;
		tt->taskid=taskid;
		tt->command=command;
		tt->task=task;
		tt->priority=priority;
		tt->parallel=parallel;

		//printf("Created Task %s (%ld)\n",tt->task.c_str(),tt->timestamp);

		id = taskid;
		taskid++;
	sem.leave();
		return (void*)tt;
}


//-------------------------------------------------------------------------
// ReceiveTask()  Creates a new task and add it to the list of pending tasks
//--------------------------------------------------------------------------
bool TaskManager::ReceiveTask(std::string userid, size_t usertaskid, const char * command, const char * task, int priority, bool parallel)
{
	sem.enter();
		//Create new task
		tasktype tt;

		//Popullate task
		tt.timestamp = getCurrentTime();
		tt.userid = userid;
		tt.taskid = taskid;
		tt.usertaskid = usertaskid;
		tt.command = command;
		tt.task = task;
		tt.priority = priority;
		tt.parallel = parallel;

		//Add task to list of pending tasks
		tasklist.push_back(tt);
		printf("Added Task [(%d) %s) %s %s]\n", (int)taskid, userid.c_str(), command,task);

		//Increase the task counter.
		taskid++;
	sem.leave();
		return true;
}


bool TaskManager::NewTask(void *task)
{
	if (!blocked)
	{
		tasktype tt;
		tt.timestamp=((tasktype*)task)->timestamp;
		tt.userid=((tasktype*)task)->userid;
		tt.command=((tasktype*)task)->command;
		tt.task=((tasktype*)task)->task;
		tt.priority=((tasktype*)task)->priority;
		tt.parallel=((tasktype*)task)->parallel;

		tasklist.push_back(tt);
	//	printf("Added Task %s (%f)\n",tt.task.c_str(), mrpt::system::timestampTotime_t(tt.timestamp) );

		FPrintTask(tt);

		return true;
	}
	return false;

}

bool TaskManager::SearchTaskById(size_t searchtaskid)
{
	size_t i;
	for (i=0;i<tasklist.size();i++)
	{
		if (tasklist[i].taskid==searchtaskid)
		{
			break;
		}

	}
	return (i!=tasklist.size());


}

//----------------------------------------------------------------
// DeleteTask()   Delete a task from the list of pending tasks
//----------------------------------------------------------------
bool TaskManager::DeleteTask(size_t taskidtoerase)
{
	size_t i;
	bool found=false;
	sem.enter();
	printf("deleting task %d\n",(int)taskidtoerase);
	for (i=0;i<tasklist.size();i++)
	{
		if (tasklist[i].taskid==taskidtoerase)
		{
			//FPrintTask((void*)(tasklist.begin()+i),false);
			tasklist.erase(tasklist.begin()+i);
			found=true;
			printf("deleted!\n");
			break;
		}

	}
	sem.leave();
	return found;

}


bool TaskManager::GetTaskbytimestamp(long timestamp,std::string &userid, char *&command, char *&task, int &prio, bool &parallel)
{
/*	size_t i;
	sem.enter();
	for (i=0;i<tasklist.size();i++)
	{
		if (tasklist[i].timestamp==timestamp)
		{
			userid=tasklist[i].userid;
			strcpy(command,tasklist[i].command.c_str());
			strcpy (task,tasklist[i].task.c_str());
			prio=tasklist[i].priority;
			parallel=tasklist[i].parallel;
			break;
		}

	}
	sem.leave();
	return (i!=tasklist.size());
*/
	return 0;


}
bool TaskManager::GetNextTask(std::string &userid,long &timestamp, char *&command, char *&task, int &prio, bool &parallel,bool reset)
{
	static int index=0;

	if (reset) index=0;

	size_t pos=tasklist.size()-index-1;

	if (pos<tasklist.size())
	{

		timestamp=tasklist[pos].timestamp;
		userid=tasklist[pos].userid;
		strcpy(command,tasklist[pos].command.c_str());
		strcpy (task,tasklist[pos].task.c_str());
		prio=tasklist[pos].priority;
		parallel=tasklist[pos].parallel;
		index++;
//		printf("%d\n",timestamp);
		return true;
	}
	else
	{
		//printf("Ya no hay mas tareas\n");
		index=0;
		return false;
	}




}
bool TaskManager::GetLastTask(char *&command,char *&task,bool erase)
{

	int last=tasklist.size()-1;
	if (last>=0)
	{
		strcpy (task,tasklist[last].task.c_str());
		strcpy(command,tasklist[last].command.c_str());
		if (erase)
		{
			tasktype removedt=tasklist[tasklist.size()];
			tasklist.pop_back();
			//FPrintTask((void*)&removedt,false);
		}
		return true;
	}
	else return false;

}



void TaskManager::BlockAgenda()
{
	blocked=true;
}
void TaskManager::UnBlockAgenda()
{
	blocked=false;
}

void TaskManager::FlushAgenda(int prio)
{
	if (prio!=-1)
	{
		for (int i=tasklist.size()-1;i>=0;i--)
		{
			if (tasklist[i].priority<prio)
				tasklist.erase(tasklist.begin()+i);
		}
	}
	else tasklist.clear();

}


void TaskManager::FPrintTask(tasktype task,bool added)
{
	if( Save_logfile )
	{
		log = fopen(fileLog,"a+");
		if (added) 
			fprintf(log,"Added a New Task:\t");
		else 
			fprintf(log,"Remove a Task from the Agenda:\t");
		//fprintf(log,"[%f]\t %s\t %s\t %s\t \n",mrpt::system::timestampTotime_t(task.timestamp),     task.userid.c_str(),     task.command.c_str(),    task.task.c_str());
		fclose(log);
	}
}


//--------------------------------------------------------
// PrintAgenda()
// Prints the parameters of all tasks into a std:string
//--------------------------------------------------------
void TaskManager::PrintAgenda(std::string &out,bool verbose)
{
	sem.enter();
		if (verbose) printf("***************************\n");
		out="";
		for (size_t i=0;i<tasklist.size();i++)
		{
			if (verbose) printf("[%u]\t%s\t%s\n",(unsigned int)i,tasklist[i].command.c_str(),tasklist[i].task.c_str());
			//Name of the user, global task id, local task id, command, params
			out=out+format("%s %d %d %s %s#",tasklist[i].userid.c_str(),(int)tasklist[i].taskid,(int)tasklist[i].usertaskid,tasklist[i].command.c_str(),tasklist[i].task.c_str());
		}
		if (verbose) printf("***************************\n");
	sem.leave();
}


bool TaskManager::IsEmpty()
{


	return (tasklist.size()==0);
}


void TaskManager::AgendaStatus(std::string &cad)
{

	char d[255];
	std::string aux="";

	sprintf(d,"NUM_PLANS=%u\n",(unsigned)tasklist.size());
	aux.append(d);

	sprintf(d,"NUM_USERS=%d\n",num_users);
	aux.append(d);

	for (size_t i=0;i<tasklist.size();i++)
	{
		//sprintf(d,"TASK_%u=%s %s %f\n",(unsigned)i,tasklist[i].command.c_str(),tasklist[i].task.c_str(), mrpt::system::timestampTotime_t(tasklist[i].timestamp) );
		aux.append(d);
	}
	for (size_t i=0;i<errors.size();i++)
	{
		aux.append("ERROR_%u=",(unsigned)i);
		aux.append(errors[i]);
	}

	printf("%s",aux.c_str());
	cad=aux;

}


/** Indicates wheter or not save a log file */
void TaskManager::set_SaveLogfile(bool logfile_option)
{
	Save_logfile = logfile_option;

	//Generate log file
	if( Save_logfile)
	{
		sprintf(fileLog,"agenda_log.txt");
		log=fopen(fileLog,"w+");
		fprintf(log,"-------------------------------------------------\n");
		TTimeStamp stamp= getCurrentTime();
		std::string date= dateTimeToString(stamp);
		fprintf(log,"------Starting Agenda at %s----\n",date.c_str());
		fprintf(log,"-------------------------------------------------\n");
		fclose(log);
	}
}