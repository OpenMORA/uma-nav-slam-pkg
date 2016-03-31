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

#ifndef CStateChartApp_H
#define CStateChartApp_H

//#include <MOOS/libMOOS/App/MOOSApp.h>
#include <COpenMORAMOOSApp.h>
#include "ManejadorSCXML.h"



class CStateChartApp : public COpenMORAApp
{
public:
    CStateChartApp();
	virtual ~CStateChartApp();

	static void NotifyNewTask(void *p,const char *label,const int uid,const char *comando,const int pri,const bool parallel);
	static double Check4Vble(const char *vble);
	static void ExecuteStateChart();
	static double* var1;
protected:
	
	/** called at startup */
	virtual bool OnStartUp();
	/** called when new mail arrives */
	virtual bool OnNewMail(MOOSMSG_LIST & NewMail);
	/** called when work is to be done */
	virtual bool Iterate();
	/** called when app connects to DB */
	virtual bool OnConnectToServer();

	bool OnCommandMsg( CMOOSMsg Msg );

	bool DoRegistrations();

	// DATA. Your local variables here...

	Rutinas *rut;
	ManejadorSCXML* handler;			//Pointer to the SCXML class handler
	std::string OBJECTIVE_FILE;			//Name of the SCXML file with the State Charts
	bool active;						//boolean to indicate the status of the StateChart module

	static CStateChartApp* meagain;		//pointer of the same class

	mrpt::system::TThreadHandle SCThread;

};
#endif
