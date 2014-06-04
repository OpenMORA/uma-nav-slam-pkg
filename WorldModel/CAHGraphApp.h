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

#ifndef CAHGraphApp_H
#define CAHGraphApp_H

#include <COpenMORAMOOSApp.h>

#include "ahgraph.h"
// Any other includes..

class CAHGraphApp : public COpenMORAApp
{
public:
    CAHGraphApp();
    virtual ~CAHGraphApp();

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

	void RefreshGraph();
	CAHGraph graph;
	std::string world_model_file;

	// DATA. Your local variables here...

};
#endif

/** @moos_TODO
  
*/

/** @moos_changelog
  *  12-11-2013 Updated method GET_PATH. Now it only takes one argument (the target node to travel to), and its implemented as NewMail
  *  12-11-2013 Added condition to OnNewMail ADD_NODE. Label of the new node should not already exists.
  *  13-11-2013 Moved functionallity from Iterate to OnNewMail, since they only should be called on request.
  *  13-11-2013 Addded Subscription to CHANGE_NODE_LABEL to change the label of a node in the topology.
  *  13.11.2013 Added Subscription to REMOVE_ARC to remove all arcs (for bidirectionality) between two nodes.
  *  13.11.2013 Added Subscription to REMOVE_NODE to remove node from Topology.
*/