/*!
 *    \file  ddfs_clusterPaxosInstance.cpp
 *   \brief  One instance of the Paxos algorithm.
 *  
 *  
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

#include "ddfs_clusterPaxosInstance.h"
#include <unistd.h>

ddfsLogger &global_logger_cpi = ddfsLogger::getInstance();

ddfsClusterPaxosInstance::ddfsClusterPaxosInstance () {
	uniqueID = -1;
}

ddfsClusterPaxosInstance::~ddfsClusterPaxosInstance () {}

ddfsStatus ddfsClusterPaxosInstance::execute (uint64_t uniqueID, list <ddfsClusterMemberPaxos *>& participatingMembers)
{
	list<ddfsClusterMemberPaxos *>::iterator clusterMemberIter;
	ddfsClusterMessagePaxos message = ddfsClusterMessagePaxos();
	int clusterQuorum = (participatingMembers.size()+1)/2;
	int currentCount = 0;
	
	/* Start the leader Election */
	/* Algorithm is straight formward.
	 * 
	 * Phase 1a: Prepare
	 * Phase 1b: Promise
	 * Phase 2a: Accept Request
	 * Phase 2b: Accepted
	 */
	while (1) {
		/*  Send a Prepare cluster message to be send to all the nodes in the cluster.
         *  Promise cluster message should arrive for all the cluster members or atleast majority of cluster members.
         */
		for(clusterMemberIter = participatingMembers.begin(); clusterMemberIter != participatingMembers.end(); clusterMemberIter++) {
			if( (*clusterMemberIter)->isOnline().compareStatus(ddfsStatus(DDFS_OK)) == 0) {
				global_logger_cpi << ddfsLogger::LOG_WARNING << "Node " << (*clusterMemberIter)->getUniqueIdentification() << " is offline";
			}
			/* Create packet for the Prepare request and send it to the choosen node */
			message.addMessage(CLUSTER_MESSAGE_LE_TYPE_PREPARE, uniqueID);
			(*clusterMemberIter)->sendClusterMetaData(&message);
			(*clusterMemberIter)->setCurrentState(s_clusterMemberPaxos_LE_PREPARE);
		}

		/*  Wait for the Promise response from Quorum */
		sleep(s_timeout);
	
		/*  Check the status of this instance of paxos algorithm */
		for(clusterMemberIter = participatingMembers.begin(); clusterMemberIter != participatingMembers.end(); clusterMemberIter++) {
			if((*clusterMemberIter)->getCurrentState() == s_clusterMemberPaxos_LE_PROMISE)
				currentCount++;
		}

		if(currentCount < clusterQuorum)
			return (ddfsStatus(DDFS_FAILURE));

		/* Send the accept request to the nodes */
		for(clusterMemberIter = participatingMembers.begin(); clusterMemberIter != participatingMembers.end(); clusterMemberIter++) {
			if((*clusterMemberIter)->getCurrentState() == s_clusterMemberPaxos_LE_PROMISE) {
				/* Create packet for the "Accept Request" request and send it to the choosen node */
				message.addMessage(CLUSTER_MESSAGE_LE_ACCEPT_REQUESTED, uniqueID);
				(*clusterMemberIter)->sendClusterMetaData(&message);
				(*clusterMemberIter)->setCurrentState(s_clusterMemberPaxos_LE_ACCEPT_REQUEST);
			}
		}
		
		/*  Wait for the Accept response from Quorum */
		sleep(s_timeout);

		currentCount = 0;

		/*  Check the status of this instance of paxos algorithm */
		for(clusterMemberIter = participatingMembers.begin(); clusterMemberIter != participatingMembers.end(); clusterMemberIter++) {
			if((*clusterMemberIter)->getCurrentState() == s_clusterMemberPaxos_LE_ACCEPTED)
				currentCount++;
		}

		if(currentCount < clusterQuorum)
			return (ddfsStatus(DDFS_FAILURE));

		/* This instance has successfully completed. */
		return (ddfsStatus(DDFS_OK));

	} /* End of while */

	return (ddfsStatus(DDFS_FAILURE));
}		/* -----  end of method ddfsClusterPaxosInstance::execute  ----- */

ddfsStatus executeAsync(uint64_t uniqueID, list <ddfsClusterMemberPaxos *>& participatingMembers, ddfsClusterPaxos& cluster) {
	return (ddfsStatus(DDFS_FAILURE));
}		/* -----  end of method ddfsClusterPaxosInstance::executeAsync  ----- */

void ddfsClusterPaxosInstance::abandon()
{
	return;
}		/* -----  end of method ddfsClusterPaxosInstance::abandon  ----- */

