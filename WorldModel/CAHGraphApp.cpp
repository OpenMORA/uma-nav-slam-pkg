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

/**  @moos_module  This module keeps a topological representation of the world
  *	 The module implements functions to create, modify and delete both components of the topology :nodes and arcs
  *  It also provides methods for finding paths from one node to another.
  */

#include "CAHGraphApp.h"

#include <sstream>
#include <iomanip>
#include <iostream>

using namespace std;

using namespace mrpt;
using namespace mrpt::slam;
using namespace mrpt::utils;
//using namespace ....


CAHGraphApp::CAHGraphApp()
//	: var (init_val), ...
{
}

CAHGraphApp::~CAHGraphApp()
{
}

bool CAHGraphApp::OnStartUp()
{
	// Read parameters (if any) from the mission configuration file.
	//! @moos_param world_model_file Path to a XML file contaning the topologic structure of the world
	bool ok=m_MissionReader.GetConfigurationParam("world_model_file",world_model_file);
	if (ok){
		// There is also a MRPT-like object (this->m_ini) that is a wrapper
		//  to read from the module config block of the current MOOS mission file.
		// m_ini.read_int(...);
		graph.LoadGraph(world_model_file);
	}

	RefreshGraph();

	//! @moos_publish GUI_VISIBLE_TOPOLOGICAL_MAP  A boolean value to indicate the availability of the Topological Map in variable GRAPH.
	 m_Comms.Notify("GUI_VISIBLE_TOPOLOGICAL_MAP","1");

	return DoRegistrations();
}

bool CAHGraphApp::OnCommandMsg( CMOOSMsg Msg )
{
	if(Msg.IsSkewed(MOOSTime())) return true;
	if(!Msg.IsString()) return MOOSFail("This module only accepts string command messages\n");
	const std::string sCmd = Msg.GetString();
	//MOOSTrace("COMMAND RECEIVED: %s\n",sCmd.c_str());
	// Process the command "sCmd".

	return true;
}

bool CAHGraphApp::Iterate()
{
	
	/** Set the ID of the closest node to the current robot position. 
		It updates the arcs between the graph and the "robot" node
		which indicates the node closest to the robot*/
	CMOOSVariable *ROBOT_TOPOLOGICAL_PLACE = GetMOOSVar("ROBOT_TOPOLOGICAL_PLACE");		// Set the ID of the closest node to the current robot position

	if (ROBOT_TOPOLOGICAL_PLACE && ROBOT_TOPOLOGICAL_PLACE->IsFresh())
	{
		ROBOT_TOPOLOGICAL_PLACE->SetFresh(false);

		size_t id = 0;
		size_t idrobot = graph.GetNodeId("Robot");

		if( graph.ExistsNodeLabel(ROBOT_TOPOLOGICAL_PLACE->GetStringVal()) )
		{
			size_t idtarget = graph.GetNodeId(ROBOT_TOPOLOGICAL_PLACE->GetStringVal());

			//Did Robot_Topological_Place changed?
			std::vector<size_t> neighbors;
			graph.GetNodeNeighbors(idrobot,"Location",neighbors);
			if (neighbors[0] != idtarget )
			{
				graph.DeleteOutcommingArcs(idrobot,"Location");
				printf("Changing robot pose to %s\n",ROBOT_TOPOLOGICAL_PLACE->GetStringVal().c_str());
				graph.AddArc(idrobot,idtarget,"robotpose","Location",id);
				RefreshGraph();
				//graph.SaveGraph("result.xml");
			}
		}
	}
	
	return true;
}


void CAHGraphApp::RefreshGraph()
{
	std::string nodes;
	std::string arcs;
	std::string entire_graph;
	//printf("Getting all nodes\n");
	graph.GetAllNodes(nodes);
	//printf("%s\n",nodes.c_str());
	//printf("Getting all arcs\n");
	graph.GetAllArcs(arcs);
	//printf("%s\n",arcs.c_str());

	//! @moos_publish GRAPH contains the list of all nodes and arcs from the world model
	//!       <br> The format of the string is: <br>
	//!		  {"list of nodes" & "list of arcs"}: <br>
	//!       Each node is separated by #.
	//!       node_name node_posx node_posy
	//!       Each arc is separated by #.
	//!       node_origin_name node_destination_name arc_type	
	m_Comms.Notify("GRAPH",nodes+"&"+arcs);	
}

bool CAHGraphApp::OnConnectToServer()
{
	DoRegistrations();
	return true;
}


bool CAHGraphApp::DoRegistrations()
{
	//! @moos_subscribe	GET_NODE_POSITION
	AddMOOSVariable("GET_NODE_POSITION", "GET_NODE_POSITION", "GET_NODE_POSITION",0);

	//! @moos_subscribe	GET_PATH
	AddMOOSVariable("GET_PATH", "GET_PATH", "GET_PATH",0);

	//! @moos_subscribe	ROBOT_TOPOLOGICAL_PLACE
	AddMOOSVariable("ROBOT_TOPOLOGICAL_PLACE","ROBOT_TOPOLOGICAL_PLACE","ROBOT_TOPOLOGICAL_PLACE",0);

	//! @moos_subscribe	ADD_NODE
	AddMOOSVariable("ADD_NODE", "ADD_NODE", "ADD_NODE",0);

	//! @moos_subscribe	ADD_ARC
	AddMOOSVariable("ADD_ARC", "ADD_ARC", "ADD_ARC",0);

	//! @moos_subscribe	MOVE_NODE
	AddMOOSVariable("MOVE_NODE", "MOVE_NODE", "MOVE_NODE",0);

	//! @moos_subscribe	CHANGE_NODE_LABEL
	AddMOOSVariable("CHANGE_NODE_LABEL", "CHANGE_NODE_LABEL", "CHANGE_NODE_LABEL",0);
	
	//! @moos_subscribe	REMOVE_ARC
	AddMOOSVariable("REMOVE_ARC", "REMOVE_ARC", "REMOVE_ARC",0);

	//! @moos_subscribe	REMOVE_NODE
	AddMOOSVariable("REMOVE_NODE", "REMOVE_NODE", "REMOVE_NODE",0);

	//! @moos_subscribe	SAVE_GRAPH
	AddMOOSVariable("SAVE_GRAPH", "SAVE_GRAPH", "SAVE_GRAPH",0);

	//! @moos_subscribe	LOAD_GRAPH
	AddMOOSVariable("LOAD_GRAPH", "LOAD_GRAPH", "LOAD_GRAPH",0);

	//! @moos_subscribe	GET_PERSON_LOCATION
	AddMOOSVariable("GET_PERSON_LOCATION", "GET_PERSON_LOCATION", "GET_PERSON_LOCATION",0);	

	//! @moos_subscribe SHUTDOWN
	AddMOOSVariable( "SHUTDOWN", "SHUTDOWN", "SHUTDOWN", 0 );

	RegisterMOOSVariables();
	return true;
}


bool CAHGraphApp::OnNewMail(MOOSMSG_LIST &NewMail)
{
	std::string cad;
	for(MOOSMSG_LIST::iterator i=NewMail.begin();i!=NewMail.end();++i)
	{				
		
		/*  Get path from node "Robot" to desired node 			
		 *	GET_PATH "petitionID nX" (The petitionID is a unique identifier used by the Planer module)
		 */
		if( i->GetName() == "GET_PATH" ) 
		{			
			std::deque<std::string> lista;
			mrpt::system::tokenize(i->GetString()," ",lista);			
			printf( "\n\nSearching a path to %s\n",lista.back().c_str());			
			size_t idtarget = graph.GetNodeId(lista.back());
			size_t idrobot = graph.GetNodeId("Robot");

			//! @moos_var PATH Sequence of nodes from "robot" to provided destiny in response to the "GET_PATH" var.
			//! If node NOT found --> PATH "petitionID NOTFOUND"
			//! If node found     --> PATH "petitionID (nodeID1 node_label1 x1 y1) (nodeID2 node_label2 x2 y2) ..."

			if (idtarget == (size_t)-1)		//Node not found, cannot get path to it
			{
				cout << "Node Not Found!" << endl;				
				//! @moos_publish PATH Sequence of nodes from "robot" to provided destiny in response to the "GET_PATH" var.
				m_Comms.Notify("PATH", format("%s NOTFOUND",lista[0].c_str()));
			}
			else
			{
				//Since node Robot is only connected to the closest node in the graph (via a location arc), 
				//avoid node "robot" in the search by getting its node location
				std::vector<size_t> neighbors;
				graph.GetNodeNeighbors(idrobot,"Location",neighbors);
				printf("Searching a path from id:%u to id:%u\n",(unsigned int)neighbors[0],(unsigned int)idtarget);

				//Search path between two nodes of the graph
				std::string path;
				bool path_found = graph.FindPath(neighbors[0], idtarget, path);

				if (path_found)
				{
					printf( "Path found!! - Publishing new PATH variable.");
					//! @moos_publish PATH Sequence of nodes from "robot" to provided destiny in response to the "GET_PATH" var.
					m_Comms.Notify("PATH", format("%s %s",lista.back().c_str(),path.c_str()));
				}
				else
				{
					MOOSTrace("No path found to node %u\n",idtarget);					
					//! @moos_publish PATH Sequence of nodes from "robot" to provided destiny in response to the "GET_PATH" var.
					m_Comms.Notify("PATH", format("%s NOTFOUND", lista[0].c_str()));
				}
					//// Trivial Path (n_start == n_end)
					//if (size_t(neighbors[0]) == idtarget)
					//{
					//	//[deprecated]
					//	//comand reactive navigation directly, and do not publish any PATH
					//	//get node coordinates
					//	printf( "Trivial Path found - Requesting Navigation to Target Point (x,y)\n");
					//	double x,y;
					//	bool res = graph.GetNodeLocation(lista.back(),x,y);
					//	const string only_target = format("[%.03f %.03f]",x,y);

					//	//! @moos_publish NAVIGATE_TARGET The position [x,y] to navigate to.
					//	m_Comms.Notify("NAVIGATE_TARGET",only_target);
					//}				
			}//end-if node found

		}
		

		// GET_PERSON_LOCATION "petitionID user_node_label" (requested by planner)
		// Returns the node_label from the topology where the user is located
		// A User(node) is defined in a location(another node) by a "location" arc between them.
		if(  i->GetName()=="GET_PERSON_LOCATION" )
		{
			std::deque<std::string> lista;
			mrpt::system::tokenize(i->GetString()," ",lista);
			
			//! @moos_var PERSON_LOCATION The result of finding a Person(node_label) in the Topological Graph
			//! If NOT found = PERSON_LOCATION "petitionID NOTFOUND"
			//! IF found     = PERSON_LOCATION "petitoinID location_node_label"

			//check that (user)node_label exists in the Topological Graph
			if ( graph.ExistsNodeLabel(lista[1]) )
			{
				size_t idperson = graph.GetNodeId(lista[1]);
				printf("Searching for %s (id: %d)\n",lista[1].c_str(),idperson);
				std::vector<size_t> dest;
				//Get list of neighbord nodes of tipe location
				graph.GetNodeNeighbors(idperson,"Location",dest);		
				if (dest.size()!=0)
				{
					// Return the label of the node where the user is
					std::string label="";			
					graph.GetNodeLabel(dest[0],label);
					//! @moos_publish PERSON_LOCATION The result of finding a Person(node_label) in the Topological Graph
					m_Comms.Notify("PERSON_LOCATION", format("%s %s",lista[0].c_str(),label.c_str()));
				}
				else
				{
					//! @moos_publish PERSON_LOCATION The result of finding a Person(node_label) in the Topological Graph
					m_Comms.Notify("PERSON_LOCATION", format("%s NOTFOUND",lista[0].c_str()));
				}
			}
			else			
			{
				cout << "[WorldModel: GET_PERSON_LOCATION]: EROR - Node label not found." << endl;
				//! @moos_publish PERSON_LOCATION The result of finding a Person(node_label) in the Topological Graph
				m_Comms.Notify("PERSON_LOCATION", format("%s NOTFOUND",lista[0].c_str()));
			}
		}

		// GET_NODE_POSITION "petitionID node_label"
		// Returns the location (x,y) of a node given its label
		if( i->GetName()=="GET_NODE_POSITION" )		
		{	
			std::deque<std::string> lista;
			mrpt::system::tokenize(i->GetString()," ",lista);
			
			//!  @moos_variable NODE_POSITION  geometric position of a node in response to the "GET_NODE_POSITION"
			//! If NOT found = NODE_POSITION "petitionID NOTFOUND"
			//! IF found     = NODE_POSITION "petitoinID x y"

			//check that node exists.
			if ( graph.ExistsNodeLabel(lista[0]) )
			{
				double x,y;
				bool res = graph.GetNodeLocation(lista[0],x,y);

				//!  @moos_publish NODE_POSITION  geometric position of a node in response to the "GET_NODE_POSITION" var.
				if (res) 
					m_Comms.Notify("NODE_POSITION", format("%s %f %f",lista[0].c_str(),x,y));
				else 
					m_Comms.Notify("NODE_POSITION", format("%s %f %f notfound",lista[0].c_str(),x,y));
			}
			else			
			{
				cout << "[WorldModel: GET_NODE_POSITION]EROR: Node label not found." << endl;				
				//m_Comms.Notify("NODE_POSITION", format("%s %f %f notfound",lista[0].c_str(),x,y));
				//!  @moos_publish NODE_POSITION  geometric position of a node in response to the "GET_NODE_POSITION" var.
				m_Comms.Notify( "NODE_POSITION", format("%s NOTFOUND",lista[0].c_str()) );
				//! @moos_publish ERROR_MSG A string containing the description of an Error.
				m_Comms.Notify("ERROR_MSG","pWorldModel: Node label not found. Node position not published.");
			}
		}



		//Add new node to topology -->  ADD_NODE label type x y
		if( (i->GetName()=="ADD_NODE") )
		{	
			std::deque<std::string> lista;
			mrpt::system::tokenize(i->GetString()," ",lista);

			//Check that label doesn't exist in topology
			if (!graph.ExistsNodeLabel(lista[0]))
			{
				cout << "Adding new node to Topology" << endl;
				size_t id;
				graph.AddNode(lista[0],lista[1],id,atof(lista[2].c_str()),atof(lista[3].c_str()));
				RefreshGraph();
			}
			else
			{
				cout << "EROR: Node label already exists" << endl;
				//! @moos_publish ERROR_MSG A string containing the description of an Error.
				m_Comms.Notify("ERROR_MSG","pWorldModel: Node label already exists. Node not Added.");
			}
		}
		
		// Add new arc to topology --> ADD_ARC label_nodeA label_nodeB type
		if( (i->GetName()=="ADD_ARC") )
		{
			std::deque<std::string> lista;
			mrpt::system::tokenize(i->GetString()," ",lista);

			//check that both nodes exists.
			if (graph.ExistsNodeLabel(lista[0]) && graph.ExistsNodeLabel(lista[1]) )
			{
				size_t id;
				// Add arc as Bidirectional: a->b + b->a
				graph.AddArcbyLabel(lista[0].c_str(),lista[1].c_str(),"",lista[2].c_str(), true, id);
				RefreshGraph();
			}
			else
				cout << "EROR: Nodes not found. Arc not created " << endl;
				//! @moos_publish ERROR_MSG A string containing the description of an Error.
				m_Comms.Notify("ERROR_MSG","pWorldModel: Nodes not found. Arc not created.");

		}

		// Change location of existing node --> MOVE_NODE label x y
		if( (i->GetName()=="MOVE_NODE") )
		{
			std::deque<std::string> lista;
			mrpt::system::tokenize(i->GetString()," ",lista);

			//check that node exists.
			if ( graph.ExistsNodeLabel(lista[0]) )
			{
				graph.SetNodeLocation(lista[0],atof(lista[1].c_str()),atof(lista[2].c_str()));
				RefreshGraph();
			}
			else
			{
				cout << "EROR: Node not found. Position not changed. " << endl;
				//! @moos_publish ERROR_MSG A string containing the description of an Error.
				m_Comms.Notify("ERROR_MSG","pWorldModel: Node not found. Position not changed.");
			}
		}

		// Change the label of existing node --> CHANGE_NODE_LABEL old_label new_label
		if( (i->GetName()=="CHANGE_NODE_LABEL") )
		{
			bool error = false;
			std::deque<std::string> lista;
			mrpt::system::tokenize(i->GetString()," ",lista);

			if (graph.ExistsNodeLabel(lista[0]))
			{
				//get ID of node
				size_t node_id = graph.GetNodeId(lista[0]);
				if (!graph.SetNodeLabel(node_id,lista[1]))
					error = true;
				else
					RefreshGraph();
			}
			else
				error = true;
			
			if (error)
			{
				cout << "EROR: Node label not found or new label already exists. Cannot Rename it." << endl;
				//! @moos_publish ERROR_MSG A string containing the description of an Error.
				m_Comms.Notify("ERROR_MSG","pWorldModel: Node label not changed because node not found or new_label already exists.");
			}
		}
				

		// Remove Arcs between two given nodes: REMOVE_ARC nodeFrom nodeTo Arc_type 
		if( i->GetName()=="REMOVE_ARC" )
		{
			bool error = false;
			std::deque<std::string> lista;
			mrpt::system::tokenize(i->GetString()," ",lista);
			
			//check that both nodes exists.
			if (!graph.ExistsNodeLabel(lista[0]) || !graph.ExistsNodeLabel(lista[1]) )
				error = true;
			else
			{
				size_t idFrom = graph.GetNodeId(lista[0]);
				size_t idTo = graph.GetNodeId(lista[1]);
			
				if( graph.DeleteArcsBetweenNodes(idFrom, idTo, lista[2]) )
					RefreshGraph();
				else
					error = true;
			}

			if (error)
			{
				cout << "EROR: Problem found when deleting arcs between two given nodes. Nodes not found." << endl;
				//! @moos_publish ERROR_MSG A string containing the description of an Error.
				m_Comms.Notify("ERROR_MSG","pWorldModel: Problem found when deleting arcs between two given nodes. Nodes not found.");
			}
		}

		// Remove Node from topological graph: REMOVE_NODE nodeLabel
		if( i->GetName()=="REMOVE_NODE" )
		{
			if( graph.DeleteNode(i->GetString().c_str()) )
				RefreshGraph();
			else
			{
				cout << "EROR: Problem deleting Node. Node may not exists." << endl;
				//! @moos_publish ERROR_MSG A string containing the description of an Error.
				m_Comms.Notify("ERROR_MSG","pWorldModel: Problem deleting Node. Node may not exists.");
			}
		}


		// Load Graph from file: LOAD_GRAPH file
		if( i->GetName()=="LOAD_GRAPH" )
		{
			printf("Loading Topology graph from: [%s]\n",i->GetString().c_str());
			graph.LoadGraph(i->GetString());
			RefreshGraph();
		}

		// Save Graph to file: SAVE_GRAPH file
		if( i->GetName()=="SAVE_GRAPH" )
		{
			printf("Saving Topology graph to: [%s]\n",i->GetString().c_str());
			graph.SaveGraph(i->GetString());
			RefreshGraph();
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
