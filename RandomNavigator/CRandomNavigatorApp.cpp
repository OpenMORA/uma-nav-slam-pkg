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

/**  @moos_module This module request continous reactive navigations by randomly selecting nodes from the graph topology.
  *  This module can be used to make a robot to randomly patrol an environment.
  */

/**  @moos_TODO
  *
  */

#include "CRandomNavigatorApp.h"
#include <mrpt/random.h>
#include <sstream>
#include <iomanip>
#include <iostream>

using namespace std;

using namespace mrpt;
using namespace mrpt::utils;

CRandomNavigatorApp::CRandomNavigatorApp()
//	: var (init_val), ...
{
}

CRandomNavigatorApp::~CRandomNavigatorApp()
{
}

bool CRandomNavigatorApp::OnStartUp()
{
	// Read parameters (if any) from the mission configuration file.
	//! @moos_param target_only_labeled_nodes Indicates if all nodes are legible as targets, or only those labeled (!=NODE...)
	std::string section = "";
	target_only_labeled_nodes = m_ini.read_bool(section,"target_only_labeled_nodes",true,false);	

	active = false;
	verbose = true;
	counter = 0;

	return DoRegistrations();
}

bool CRandomNavigatorApp::OnCommandMsg( CMOOSMsg Msg )
{
	if(Msg.IsSkewed(MOOSTime())) return true;
	if(!Msg.IsString()) return MOOSFail("This module only accepts string command messages\n");
	const std::string sCmd = Msg.GetString();
	//MOOSTrace("COMMAND RECEIVED: %s\n",sCmd.c_str());
	// Process the command "sCmd".

	return true;
}

bool CRandomNavigatorApp::Iterate()
{
	return true;
}


bool CRandomNavigatorApp::OnConnectToServer()
{
	DoRegistrations();
	return true;
}


bool CRandomNavigatorApp::DoRegistrations()
{
	//! @moos_subscribe SHUTDOWN
	AddMOOSVariable( "SHUTDOWN", "SHUTDOWN", "SHUTDOWN", 0 );

	//! @moos_subscribe RANDOM_NAVIGATOR
	AddMOOSVariable( "RANDOM_NAVIGATOR", "RANDOM_NAVIGATOR", "RANDOM_NAVIGATOR", 0 );

	//! @moos_subscribe PLAN_FINISHED
	AddMOOSVariable( "PLAN_FINISHED", "PLAN_FINISHED", "PLAN_FINISHED", 0 );

	//! @moos_subscribe GRAPH
	AddMOOSVariable( "GRAPH", "GRAPH", "GRAPH", 0 );

	//! @moos_subscribe ROBOT_TOPOLOGICAL_PLACE
	AddMOOSVariable( "ROBOT_TOPOLOGICAL_PLACE", "ROBOT_TOPOLOGICAL_PLACE", "ROBOT_TOPOLOGICAL_PLACE", 0 );	

	RegisterMOOSVariables();
	return true;
}


bool CRandomNavigatorApp::OnNewMail(MOOSMSG_LIST &NewMail)
{
	std::string cad;
	for(MOOSMSG_LIST::iterator i=NewMail.begin();i!=NewMail.end();++i)
	{				
		
		//RANDOM_NAVIGATOR
		if( i->GetName() == "RANDOM_NAVIGATOR" )
		{
			if (verbose) cout << "New RANDOM_NAVIGATOR msg" << endl;

			//Enable?
			if( i->GetDouble() == 1.0 )
			{
				if( active == false)
				{
					if( verbose ) cout << "[RandomNavigator]: Activating Random Navigations." << endl;
					
					// GRAPH contains the list of all nodes and arcs from the world model
					//  	 {"list of nodes" & "list of arcs"}:
					//        Each node is separated by #.
					//        node_name node_posx node_posy
					CMOOSVariable *GRAPH = GetMOOSVar("GRAPH");
					if( GRAPH )
					{
						std::deque<std::string> lista;
						mrpt::system::tokenize(GRAPH->GetStringVal(),"&",lista);
						if( lista.size() == 2)
						{
							std::deque<std::string> lista_nodes;
							mrpt::system::tokenize(lista[0],"#",lista_nodes);

							//Fill list of node names
							for( std::deque<std::string>::iterator it=lista_nodes.begin(); it!=lista_nodes.end(); it++ )
							{
								std::deque<std::string> node_params;
								mrpt::system::tokenize(it->c_str()," ",node_params);
								if( (node_params[0]!= "Robot") && (node_params[0]!= "Docking") )
								{
									if (target_only_labeled_nodes)
									{
										if( node_params[0].compare(0,4,"NODE") != 0)								
											node_list.push_back(node_params[0]);	//Keep only the node names (except node Robot)
									}
									else
										node_list.push_back(node_params[0]);	//Keep only the node names (except node Robot)
								}
							}

							//
							printf("[RandomNavigator]: Number of nodes detected is %u\n",node_list.size());
							active = true;
							Request_Random_Navigation();
						}
						else
						{
							cout << "[RandomNavigator]: Error. GRAPH variable incorrect!!" << endl;
							active = false;
						}
					}
					else
					{
						cout << "[RandomNavigator]: Error. No GRAPH available." << endl;
						active = false;
					}
				}
				else
				{
					cout << "[RandomNavigator]: Module already Active. Nothing to be done." << endl;
				}			
			}
			//Disable?
			else if( i->GetDouble() == 0.0 )
			{
				cout << "[RandomNavigator]: Stopping Random Navigations." << endl;
				active = false;
			}
			//Temporarily Disable?
			else if( i->GetDouble() == 2.0 )
			{
				cout << "[RandomNavigator]: Stopping Random Navigations Temporarily." << endl;
				active = false;
			}
			else
				cout << "[RandomNavigator]: Error in the RANDOM_NAVIGATOR varible. Nothing Done" << endl;
		}

		
		
		// PLAN_FINISHED
		if( i->GetName()=="PLAN_FINISHED" )
		{
			if( active )
			{
				//check if the finished plan was originated by us.
				// PLAN_FINISHED TaskID UserTaskID UserID
				std::deque<std::string> plan_params;
				mrpt::system::tokenize(i->GetString()," ",plan_params);
				if( plan_params.size() == 3)
				{
					if( plan_params[2] == "RANDOM_NAVIGATOR" )
					{
						//current navigation eneded, request a new navigation
						Request_Random_Navigation();
						counter++;
					}
				}
				else
					cout << "[RandomNavigator]: Error in the PLAN_FINISHED varible. Nothing Done" << endl;
			}
		}

		
		// SHUTDOWN
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


void CRandomNavigatorApp::Request_Random_Navigation()
{
	if( active )
	{
		//Select a node to navigate to from list of active nodes.
		bool found = false;
		std::string	current_node;
		//Avoid selecting the current topological place
		CMOOSVariable * pVar = GetMOOSVar("ROBOT_TOPOLOGICAL_PLACE");
		if(pVar)
			current_node = pVar->GetStringVal();
		else
			current_node = "";

		while (!found)
		{
			//Get random node
			mrpt::random::randomGenerator.randomize();
			size_t selected = (size_t) floor( mrpt::random::randomGenerator.drawUniform(0,node_list.size()) );

			if( (selected>=0) && (selected<node_list.size()) )
			{
				//Check its not the current node
				if (node_list[selected].c_str() != current_node)
				{
					found = true;
					if( verbose ) 
						cout << "Selected node is: " << selected << " from a total of " << node_list.size() << " nodes." << endl;

					//Request a Reactive navigation to such node
					std::string selected_node_label = node_list[selected];
					printf("[RandomNavigator]: Navigating to node: %s\n", selected_node_label.c_str() );

					//! @moos_publish NEW_TASK Launch a new task in the Agenda Module
					//  format: NEW_TASK UserID UserTaskID Command Arguments
					m_Comms.Notify("NEW_TASK", format("RANDOM_NAVIGATOR %u MOVE %s",counter,selected_node_label.c_str()) );
					if( verbose )
						cout << "NEW_TASK: RANDOM_NAVIGATOR " << counter << " MOVE " << selected_node_label << endl;
				}
			}
			else
			{
				cout << "[RandomNavigator]: Error in the Random Selection!. Disabling RandomNavigator!" << endl;
				//! @moos_publish RANDOM_NAVIGATOR
				m_Comms.Notify("RANDOM_NAVIGATOR", 0.0);
			}
		}
	}//end-if active
}