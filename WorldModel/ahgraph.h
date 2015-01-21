#ifndef __CAHGRAPH_H
#define __CAHGRAPH_H

#include <mrpt/hmtslam/CHierarchicalMHMap.h>

class CAHGraph
{
public:
	CAHGraph();
	~CAHGraph();
	bool LoadGraph(const std::string file);
	bool SaveGraph(const std::string file);

	bool AddNode(const std::string label,const std::string type,size_t &id,double x=0,double y=0);
	size_t GetNodeId (const std::string label);
	void GetNodeLabel (const size_t id,std::string &label);
	bool SetNodeLabel (const size_t id,std::string label);
	void GetNodeNeighbors(const size_t idnode,const std::string arc_type,std::vector<size_t> &neighbors);

	//list is made of "nodelabel1 posx1 posy1#nodelabel2 posx2 posy2#...
	void GetAllNodes(std::string &list);

	//list is made of "nodeorigin1 nodedestination1 arctype1#nodeorigin2 nodedestination2 arctype2#...
	void GetAllArcs(std::string &list);

	/** Delete all Arcs of type arc_type outcomming from a given node */
	void DeleteOutcommingArcs(const uint64_t idnode,const std::string arc_type);
	/** Delete all Arcs of type arc_type incomming from a given node */
	void DeleteIncommingArcs(const uint64_t idnode,const std::string arc_type);
	/** Delete all Arcs of type arc_type outcomming or incomming from a given node */
	void DeleteAllArcs(const uint64_t idnode,const std::string arc_type);
	/** Delete all Arcs outcomming or incomming from a given node */
	void DeleteAllArcs(const uint64_t idnode);



	bool AddArc(size_t idfrom,size_t idto,const std::string label,const std::string type,size_t &id);
	bool AddArcbyLabel(std::string idfrom,std::string idto,const std::string label,const std::string type,bool bidi,size_t &id);

	//Return a path as a vector of node ids
	bool FindPath(size_t idstart,size_t idend,std::vector<size_t> &nodes_id);

	//Idem but with more information:
	//a path is a string: id_node id_label x_pos y_pos id_node id_label......
	bool FindPath(size_t idstart,size_t idend,std::string &path);

	bool GetNodeLocation(std::string node_label,double &x,double &y);
	bool SetNodeLocation(std::string node_label,double x,double y);

	// Check if node with given label already exists in topology
	bool ExistsNodeLabel (const std::string label);

	// Delete all arcs of type arc_type between two given nodes
	bool DeleteArcsBetweenNodes(size_t idfrom, size_t idto, const std::string type);

	// Delete a node from Topology
	bool DeleteNode(const std::string label);

private:

	mrpt::hmtslam::CHierarchicalMHMap grafo;


};
#endif

/** @moos_TODO
  
*/

/** @moos_changelog
  * 12-11-2013	Included comments on most functions and methods.
  * 12-11-2013  Added function to Rename (change label) an existing node: bool SetNodeLabel (const size_t id,std::string &label);
  * 12-11-2013  Added function to Check if node with given label already exists: bool ExistsNodeLabel (std::string label);
  * 13-11-2013  Added function to remove all existing arcs between two given nodes: DeleteArcsBetweenNodes(size_t idfrom, size_t idto, const std::string type)
  * 13-11-2013  Added function to remove all incomming arcs from a given node: CAHGraph::DeleteIncommingArcs(const uint64_t idnode,const std::string arc_type)
  * 13-11-2013  Added function to remove all existing arcs incomming or outcomming from a given node: CAHGraph::DeleteAllArcs(const uint64_t idnode,const std::string arc_type)
  * 13-11-2013  Added function to remove all existing arcs incomming or outcomming from a given node: CAHGraph::DeleteAllArcs(const uint64_t idnode)
  * 13-11-2013  Added function to remove an existing node: DeleteNode(const std::string label)
  * 13-11-2013  Fixed problems when nodes do not exists, avoiding RunTimeErrors.
*/
