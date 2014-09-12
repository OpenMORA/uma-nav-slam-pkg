/* +---------------------------------------------------------------------------+
   |          The Mobile Robot Programming Toolkit (MRPT) C++ library          |
   |                                                                           |
   |                   http://mrpt.sourceforge.net/                            |
   |                                                                           |
   |   Copyright (C) 2005-2008  University of Malaga                           |
   |                                                                           |
   |    This software was written by the Machine Perception and Intelligent    |
   |      Robotics Lab, University of Malaga (Spain).                          |
   |    Contact: Jose-Luis Blanco  <jlblanco@ctima.uma.es>                     |
   |                                                                           |
   |  This file is part of the MRPT project.                                   |
   |                                                                           |
   |     MRPT is free software: you can redistribute it and/or modify          |
   |     it under the terms of the GNU General Public License as published by  |
   |     the Free Software Foundation, either version 3 of the License, or     |
   |     (at your option) any later version.                                   |
   |                                                                           |
   |   MRPT is distributed in the hope that it will be useful,                 |
   |     but WITHOUT ANY WARRANTY; without even the implied warranty of        |
   |     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         |
   |     GNU General Public License for more details.                          |
   |                                                                           |
   |     You should have received a copy of the GNU General Public License     |
   |     along with MRPT.  If not, see <http://www.gnu.org/licenses/>.         |
   |                                                                           |
   +---------------------------------------------------------------------------+ */




#include <mrpt/utils.h>
#include "ahgraph.h"

CAHGraph::CAHGraph()
{
}

CAHGraph::~CAHGraph()
{
}

bool CAHGraph::LoadGraph(const std::string file)
{
	grafo.clear();
	grafo.loadFromXMLfile(file);
	return true;
}

bool CAHGraph::SaveGraph(const std::string file)
{
	grafo.dumpAsXMLfile(file);
	return true;
}

/** Add new node to Topology */
bool CAHGraph::AddNode(const std::string label,const std::string type,size_t &id,double x,double y)
{
	CHMHMapNodePtr node;
	node = CHMHMapNode::Create( &grafo);
	node->m_label=label;
	node->m_nodeType.setType(type);
	node->m_hypotheses.insert( COMMON_TOPOLOG_HYP);


	CPoint2DPtr o=CPoint2D::Create();
	std::string s=format("[%.03f %.03f]",x,y);
	o->fromString(s);

	node->m_annotations.set(NODE_ANNOTATION_PLACE_POSE,o,COMMON_TOPOLOG_HYP);
	id=(size_t)node->getID();
	printf("Added node: %s %s %f %f\n",label.c_str(),type.c_str(),x,y);

	return true;
}

/** Get ID of a node given its label */
size_t CAHGraph::GetNodeId (const std::string label)
{
	size_t id=-1;
	CHMHMapNodePtr node;
	node=grafo.getNodeByLabel(label,COMMON_TOPOLOG_HYP);
	if (!node.null()) id=(size_t)node->getID();
	else printf("Error, node %s not found\n",label.c_str());
	return id;
}

/** Get the label of a node given its ID */
void CAHGraph::GetNodeLabel (const size_t id,std::string &label)
{
	label="";
	CHMHMapNodePtr node;
	node=grafo.getNodeByID(id);
	label=node->m_label;

}

/** Set a new label of an existing node ID */
bool CAHGraph::SetNodeLabel (const size_t id,std::string label)
{	
	//check that new label does not exists.
	if(!ExistsNodeLabel(label) )
	{
		CHMHMapNodePtr node;
		node=grafo.getNodeByID(id);
		printf("Renaming node from: %s To: %s\n",node->m_label.c_str(),label.c_str());
		node->m_label = label;
		return true;
	}
	else
	{
		cout << " Error when changing Label to node. Label already exists." << endl;
		return false;
	}
}


/** Get a list of neighbourd nodes IDs */
void CAHGraph::GetNodeNeighbors(const size_t idnode,const std::string arc_type,std::vector<size_t> &neighbors)
{
	CHMHMapNodePtr node;
	node=grafo.getNodeByID(idnode);
	if (node)
	{
		mrpt::hmtslam::TArcList arcs;
		node->getArcs(arcs);
		TArcList::const_iterator it;
		for (it=arcs.begin();it!=arcs.end();++it)
		{
			if ((*it)->m_arcType.getType()==arc_type)
				neighbors.push_back((size_t)(*it)->getNodeTo());
		}
	}
}

/** Get a list containing all the nodes in the Topology. 
  * List is compose of "nodelabel1 posx1 posy1#nodelabel2 posx2 posy2#... 
  */
void CAHGraph::GetAllNodes(std::string &list)
{
	printf("Getting all nodes in Topology: %u\n",grafo.nodeCount());
	list="";	

	for (TNodeList::const_iterator it=grafo.begin();it!=grafo.end();++it)
	{
		std::string label=it->second->m_label;
		printf("Node name is: %s\n", label.c_str() );		
		double x=0.0;
		double y=0.0;
		GetNodeLocation(label,x,y);
		list.append(format("%s %f %f#",label.c_str(),x,y));
	}
	//printf("All Nodes:[%s]\n",list.c_str());
}

/** Get a list of all arcs in the Topology.
  * List is made of "nodeorigin1 nodedestination1 arctype1#nodeorigin2 nodedestination2 arctype2#...
  */
void CAHGraph::GetAllArcs(std::string &list)
{

	list="";
	for (TNodeList::const_iterator it=grafo.begin();it!=grafo.end();++it)
	{
		std::string nodeoriginlabel=it->second->m_label;

		mrpt::hmtslam::TArcList arcs;
		it->second->getArcs(arcs);

		TArcList::iterator itarcs;

		for (itarcs=arcs.begin();itarcs!=arcs.end();++itarcs)
		{
			if (((*itarcs)->getNodeFrom()==it->second->getID()))
			{

				size_t nodeid=(*itarcs)->getNodeTo();
				std::string nodedestinationlabel=grafo.getNodeByID(nodeid)->m_label;
				list.append(format("%s %s %s#",nodeoriginlabel.c_str(),nodedestinationlabel.c_str(),(*itarcs)->m_arcType.getType().c_str()));


			}
		}
	}
	//printf("All Arcs:[%s]\n",list.c_str());
}

/** Delete all nodes of type arc_type outcomming from given node */
void CAHGraph::DeleteOutcommingArcs(const uint64_t idnode,const std::string arc_type)
{
	CHMHMapNodePtr node;
	node=grafo.getNodeByID(idnode);
	if (node)
	{
		mrpt::hmtslam::TArcList arcs;
		node->getArcs(arcs);

		TArcList::iterator it;

		for (it=arcs.begin();it!=arcs.end();++it)
		{
			if (((*it)->getNodeFrom()==idnode) && (*it)->m_arcType.getType()==arc_type)
			{
				it->clear();
			}
		}
		printf("Sucessfully deleted Outcomming arcs from node: %s\n",node->m_label.c_str());
	}
	else
		printf("Error deleting OutcommingArcs from node_ID: %u. Node NOT found.\n",idnode);
}

/** Delete all nodes of type arc_type incomming from given node */
void CAHGraph::DeleteIncommingArcs(const uint64_t idnode,const std::string arc_type)
{
	CHMHMapNodePtr node;
	node=grafo.getNodeByID(idnode);
	if (node)
	{
		mrpt::hmtslam::TArcList arcs;
		node->getArcs(arcs);

		TArcList::iterator it;

		for (it=arcs.begin();it!=arcs.end();++it)
		{
			if (((*it)->getNodeTo()==idnode) && (*it)->m_arcType.getType()==arc_type)
			{
				it->clear();
			}
		}
		printf("Sucessfully deleted Incomming arcs from node: %s\n",node->m_label.c_str());
	}
	else
		printf("Error deleting IncommingArcs from node_ID: %u. Node NOT found.\n",idnode);
}

/** Delete all nodes of type arc_type outcomming or incomming from given node */
void CAHGraph::DeleteAllArcs(const uint64_t idnode,const std::string arc_type)
{
	CHMHMapNodePtr node;
	node=grafo.getNodeByID(idnode);
	if (node)
	{
		mrpt::hmtslam::TArcList arcs;
		node->getArcs(arcs);

		TArcList::iterator it;

		for (it=arcs.begin();it!=arcs.end();++it)
		{
			if( (*it)->m_arcType.getType()==arc_type )
			{
				it->clear();
			}
		}
		printf("Sucessfully deleted All arcs from node: %s\n",node->m_label.c_str());
	}
	else
		printf("Error deleting AllArcs from node_ID: %u. Node NOT found.\n",idnode);
}

/** Delete all nodes outcomming or incomming from given node */
void CAHGraph::DeleteAllArcs(const uint64_t idnode)
{
	CHMHMapNodePtr node;
	node=grafo.getNodeByID(idnode);
	if (node)
	{
		mrpt::hmtslam::TArcList arcs;
		node->getArcs(arcs);

		TArcList::iterator it;

		for (it=arcs.begin();it!=arcs.end();++it)
			it->clear();		
		
		printf("Sucessfully deleted All arcs from node: %s\n",node->m_label.c_str());
	}
	else
		printf("Error deleting AllArcs from node_ID: %u. Node NOT found.\n",idnode);
}

/** Delete all arcs of type arc_type between two given nodes */
bool CAHGraph::DeleteArcsBetweenNodes(size_t idfrom, size_t idto, const std::string type)
{	
	// Check that both nodes exists
	CHMHMapNodePtr node1, node2;
	node1 = grafo.getNodeByID(idfrom);
	node2 = grafo.getNodeByID(idto);
	if( node1.null() || node2.null() )
	{
		printf("Error deleting Arcs between 2 nodes. Nodes NOT found.\n");
		return false;
	}

	// Get all arcs between given nodes
	mrpt::hmtslam::TArcList arcs;
	grafo.findArcsOfTypeBetweenNodes(idfrom,idto,0,type,arcs);

	//Remove them
	TArcList::iterator it;
	for (it=arcs.begin();it!=arcs.end();++it)
		it->clear();

	printf("Sucessfully deleted AllArcs of Type: %s Between nodes_id: %u - %u.\n",type.c_str(),idfrom,idto);
	return true;
}

/** Add new arc to Topology given the label of the nodes to be connected. */
bool CAHGraph::AddArcbyLabel(std::string idfrom,std::string idto,const std::string label,const std::string type,bool bidi,size_t &id)
{
	CHMHMapNodePtr nodefrom,nodeto;
	CHMHMapArcPtr arc;

	nodefrom=grafo.getNodeByLabel(idfrom,COMMON_TOPOLOG_HYP);
	nodeto=grafo.getNodeByLabel(idto,COMMON_TOPOLOG_HYP);

	if (nodefrom && nodeto)
	{
		arc = CHMHMapArc::Create(nodefrom,nodeto,0,&grafo);
		arc->m_arcType.setType(type);
		if (bidi)
		{
			CHMHMapArcPtr arcbidi;
			arcbidi = CHMHMapArc::Create(nodeto,nodefrom,0,&grafo);
			arcbidi->m_arcType.setType(type);	
		}
		id=0;
		printf("Added Arc between nodes: %s - %s.\n",nodefrom->m_label.c_str(),nodeto->m_label.c_str());
		return true;
	}
	else
	{
		printf("Error Adding Arcs between 2 nodes. Nodes NOT found.\n");
		return false;
	}
}

/** Add new arc to Topology given the IDs of the nodes to connect */
bool CAHGraph::AddArc(size_t idfrom,size_t idto,const std::string label,const std::string type,size_t &id)
{
	CHMHMapNodePtr nodefrom,nodeto;
	CHMHMapArcPtr arc;

	nodefrom=grafo.getNodeByID(idfrom);
	nodeto=grafo.getNodeByID(idto);
	if (nodefrom && nodeto)
	{
		arc = CHMHMapArc::Create(nodefrom,nodeto,0,&grafo);
		arc->m_arcType.setType(type);
		id=0;
		printf("Sucessfully Added Arc between nodes: %s - %s.\n",nodefrom->m_label.c_str(),nodeto->m_label.c_str());
		return true;
	}
	else
	{
		printf("Error Adding Arc between 2 nodes. Nodes NOT found.\n");
		return false;
	}
}

/** Find a path between Node idstart and node idend given their respective IDs */
bool CAHGraph::FindPath(size_t idstart,size_t idend,std::vector<size_t> &nodes_id)
{

	mrpt::hmtslam::TArcList path;	
	//cheack that nodes are not isolated
	std::vector<size_t> neighborsStart, neighborsEnd;
	GetNodeNeighbors(idstart,"Navegability",neighborsStart);
	GetNodeNeighbors(idend,"Navegability",neighborsEnd);
	if( neighborsStart.size()>=1 && neighborsEnd.size()>=1 )
	{
		cout << "Generating path between nodes " << idstart << " and " << idend << endl;
		grafo.findPathBetweenNodes(idstart,idend,COMMON_TOPOLOG_HYP,path,true);		
		printf("Path length %u\n",(unsigned int)path.size());
		TArcList::const_iterator it;
		for (it=path.begin();it!=path.end();++it)
		{
				nodes_id.push_back((size_t)(*it)->getNodeFrom());
				printf("[from %u,to %u]\n",(unsigned int)(*it)->getNodeFrom(),(unsigned int)(*it)->getNodeTo());
		}
		//printf("The last one [%d]\n",idend);
		nodes_id.push_back( idend);
		return (nodes_id.size()>=1);
	}
	else
	{
		cout << "[CAHGraph::FindPath] Node origin or destiny Isolated. Path not found" << endl;
		return false;
	}

}

/** Find a path between Node Robot (the current robot position in the graph) and node idend
  * if found, PATH = "(id1 label1 x1 y1) (id2 label2 x2 y2)..."
  */
bool CAHGraph::FindPath(size_t idstart, size_t idend, std::string &path)
{
	std::vector<size_t> nodes_id;
	std::string label;
	double x=0,y=0;
	path = "";
	
	// Trivial path. PATH = "(id_end label_node_end x_end y_end)
	if (idstart==idend) 
	{
		printf("trivial path\n");
		GetNodeLabel(idend,label);
		GetNodeLocation(label,x,y);
		path.append(format("%u %s %f %f ",(unsigned int)idend,label.c_str(),x,y));
		return true;
	}

	if (!FindPath(idstart,idend,nodes_id)) 
		return false;

	for (size_t i=0;i<nodes_id.size();i++)
	{
		GetNodeLabel(nodes_id[i],label);
		GetNodeLocation(label,x,y);
		path.append(format("%u %s %f %f ",(unsigned int)nodes_id[i],label.c_str(),x,y));
		//printf("[%s]\n",format("%d %s %f %f ",nodes_id[i],label.c_str(),x,y).c_str());
	}
	//printf("[%s]\n",path.c_str());
	return true;
}

/** change the location (x,y) of an existing Node */
bool CAHGraph::SetNodeLocation(string node_label,double x,double y)
{
	CHMHMapNodePtr node;
	node=grafo.getNodeByLabel(node_label,COMMON_TOPOLOG_HYP);
	if (node)
	{
		for (CMHPropertiesValuesList::iterator ann = node->m_annotations.begin(); ann != node->m_annotations.end(); ++ann)
		{
			ASSERT_(ann->value.present())

			if (IS_CLASS(ann->value,CPoint2D))
			{
				CPoint2DPtr(ann->value)->fromString(format("[%.03f %.03f]",x,y));

				printf("Set Node Location %s to %f,%f\n",node_label.c_str(),x,y);
			}
		}
		return true;
	}
	else return false;
}

/** Get location (x,y) of exisitng node */
bool CAHGraph::GetNodeLocation(string node_label,double &x,double &y)
{	
	CHMHMapNodePtr node;
	node=grafo.getNodeByLabel(node_label,COMMON_TOPOLOG_HYP);
	if (node)
	{
		printf("[WolrdModel] Requested location of node: %s. Searching annotations...\n",node->m_label.c_str());
		for (CMHPropertiesValuesList::const_iterator ann = node->m_annotations.begin(); ann != node->m_annotations.end(); ++ann)
		{
			printf("[WolrdModel] Annotation value is present = %s\n",ann->value.present() ? "true" : "false");			
			ASSERT_(ann->value.present())

			string  str;		

			if (IS_CLASS(ann->value,CPoint2D))
			{
				printf("[WolrdModel] IS CLASS\n");
				CPoint2DPtr o = CPoint2DPtr(ann->value);
				//o->asString(str);
				x = o->x();
				y = o->y();
				printf("[WolrdModel] Requested location of node: %s = [%.3f %.3f]\n",node_label.c_str(),x,y);
			}			
			else			
			{
				printf("[WolrdModel] Error - Incorrect ClassType.\n");
				//cout << "Class type is: ";
				//printf("%s\n",ann->value->GetRuntimeClass()->className);
			}
		}
		return true;
	}
	else
	{
		printf("[WolrdModel] Error - Node not found or Node location not available.\n");
		return false;
	}
}


/** Check if node with given label already exists in topology */
bool CAHGraph::ExistsNodeLabel (const std::string label)
{
	CHMHMapNodePtr node;
	node = grafo.getNodeByLabel(label,COMMON_TOPOLOG_HYP);
	if (!node.null()) 
		return true;
	else 
		return false;
}


/**  Delete a node from Topology */
bool CAHGraph::DeleteNode(const std::string label)
{
	if( !ExistsNodeLabel(label) )
	{
		printf("Error Deleting Node. Node NOT found.\n");
		return false;
	}

	//Ensure at least one remaining node (for Robot)
	if( grafo.nodeCount()<3 )
	{
		printf("Error Deleting Node. There must exists at least one node in the Topology.\n");
		return false;
	}

	// Check if node to delete is the current Robot node
	const uint64_t idnode = GetNodeId(label);
	size_t idrobot = GetNodeId("Robot");	
	std::vector<size_t> neighbors;
	GetNodeNeighbors(idrobot,"Location",neighbors);	
	if (neighbors[0] == idnode )
	{
		printf("Alert! Deleting Node where Robot is located. Robot will be relocalized.\n");
		DeleteOutcommingArcs(idrobot,"Location");
		for (TNodeList::const_iterator it=grafo.begin();it!=grafo.end();++it)
		{
			if( label.compare(it->second->m_label)!= 0)
			{
				size_t id;
				AddArc(idrobot,GetNodeId(it->second->m_label),"robotpose","Location",id);
				break;
			}
		}		
	}
	//First, delete all arcs Outcomming or incomming from it!	
	DeleteAllArcs(idnode,"Navegability");

	// Then delete the node itself.
	CHMHMapNodePtr node;
	node = grafo.getNodeByLabel(label,COMMON_TOPOLOG_HYP);
	node.clear();
	printf("Sucessfully Deleted node: %s\n",label.c_str());
	return true;
}