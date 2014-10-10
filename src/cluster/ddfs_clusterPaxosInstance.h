/*!
 *    \file  ddfs_clusterPaxosInstance.h
 *   \brief  Class that describes one instance of Paxos algorithm.
 *  
 *  This class describes one instance of Paxos algorithm.
 *  This is for Paxos instance that this node is "Leader" of.
 *  Any paxos instance in which a local node participate, but is not a 
 *  "Leader", should/will not be managed by local node.
 *  
 *  \author  Harman Patial, harman.patial@gmail.com
 *  
 *  \internal
 *      Compiler:  g++
 *     Copyright:  
 *  
 *  This source code is released for free distribution under the terms of the
 *  GNU General Public License as published by the Free Software Foundation.
 */

#ifndef DDFS_CLUSTER_PAXOS_INSTANCE_H
#define DDFS_CLUSTER_PAXOS_INSTANCE_H


#include "ddfs_clusterPaxos.h"
#include "ddfs_clusterMemberPaxos.h"
#include "ddfs_clusterMessagesPaxos.h"

#include "../global/ddfs_status.h"
#include "../logger/ddfs_fileLogger.h"
#define	s_paxosInstanceInvalid	-1

/*!
 *  \class  ddfs_clusterPaxosInstance
 *  \brief  This class describes one instance of Paxos algorithm.
 *  
 *  This class describes one instance of Paxos algorithm of which
 *  local node is the leader.
 *  
 */
class ddfsClusterPaxosInstance
{
	public:
		// ====================  LIFECYCLE     ======================================= 
		ddfsClusterPaxosInstance ();                             /* constructor */
		ddfsClusterPaxosInstance (ddfsClusterMessagePaxos members); /* constructor */  

		~ddfsClusterPaxosInstance ();                            /* destructor */    

		/* ====================  ACCESSORS     ======================================= */
		ddfsStatus execute(uint64_t uniqueID, list <ddfsClusterMemberPaxos *>& participatingMembers);
		ddfsStatus executeAsync(uint64_t uniqueID, list <ddfsClusterMemberPaxos *>& participatingMembers, ddfsClusterPaxos& cluster);
		/* ====================  MUTATORS      ======================================= */
		void abandon();
		/* ====================  OPERATORS     ======================================= */

		ddfsClusterPaxosInstance& operator = ( const ddfsClusterPaxosInstance &other ); /* assignment operator */

	protected:
		/* ====================  METHODS       ======================================= */

		/* ====================  DATA MEMBERS  ======================================= */

	private:
		/* ====================  METHODS       ======================================= */
		ddfsClusterPaxosInstance (const ddfsClusterPaxosInstance &other);   /* copy constructor */
        const static int s_timeout = 2;

		/* ====================  DATA MEMBERS  ======================================= */
		int uniqueID;
//		list <ddfsClusterMemberPaxos>& participatingMembers;

}; /* -----  end of class ddfsClusterPaxosInstance  ----- */

#endif /*  Ending DDFS_CLUSTER_PAXOS_INSTANCE_H */
