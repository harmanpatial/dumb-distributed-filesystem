/*
 * @file ddfs_clusterPaxos.cpp
 *
 * @brief Module containing the cluster class.
 *
 * This is the module that contains cluster class.
 *
 * @author Harman Patial <harman.patial@gmail.com>
 *
 */

#include <fstream>
#include <string>

#include "ddfs_clusterPaxos.h"
#include "ddfs_clusterMessagesPaxos.h"
#include "../logger/ddfs_fileLogger.h"
#include "../global/ddfs_status.h"

#include <sys/types.h>
#include <ifaddrs.h>

using namespace std;

ddfsLogger &global_logger = ddfsLogger::getInstance();
 
ddfsClusterPaxos::ddfsClusterPaxos() {
	clusterID = s_clusterIDInvalid;
	localClusterMember = new ddfsClusterMemberPaxos;
	clusterMemberCount = 1; /* 1 for the local Node */

	return;
}

ddfsStatus ddfsClusterPaxos::leaderElection() {
	int localuniqueID;
	list<ddfsClusterMemberPaxos>::iterator clusterMemberIter;
	ddfsClusterMessagePaxos message = ddfsClusterMessagePaxos();
	
	if(clusterMemberCount <= 1) {
		global_logger << ddfsLogger::LOG_INFO
			<< "Number of members in the cluster : "<< clusterMemberCount;
		global_logger << ddfsLogger::LOG_INFO
			<< "Cannot perform leader election, Add more nodes in the cluster";
		return (ddfsStatus(DDFS_CLUSTER_INSUFFICIENT_NODES));
	}
	
	/* Start the leader Election */
	/* Alogirithm is straight formward.
	 * 
	 * Phase 1a: Prepare
	 * Phase 1b: Promise
	 * Phase 2a: Accept Request
	 * Phase 2b: Accepted
	 */
	/* Getting unique identifier to be used leader election */
	localuniqueID = getLocalNode()->getUniqueIdentification();
	
	/*  Send Prepare cluster message to all the nodes in the cluster. Promise cluster message should arrive */
	for(clusterMemberIter = clusterMembers.begin(); clusterMemberIter != clusterMembers.end(); clusterMemberIter++) {
		if( clusterMemberIter->isOnline().compareStatus(ddfsStatus(DDFS_OK)) == 0) {
			global_logger << ddfsLogger::LOG_WARNING << "Node " << clusterMemberIter->getUniqueIdentification() << " is offline";
		}
		/* Create packet for the Prepare request and send it to the choosen node */
		message.addMessage(CLUSTER_MESSAGE_TYPE_PREPARE, localuniqueID);
		clusterMemberIter->sendClusterMetaData(&message);
		clusterMemberIter->setCurrentState(s_clusterMemberPaxos_LE_PREPARE);
	}

	/*  Wait for 10 seconds for the Promise response from Quorum */
	

	return (ddfsStatus(DDFS_FAILURE)); 
}

void ddfsClusterPaxos::asyncEventHandling() {
	
}

ddfsStatus ddfsClusterPaxos::addMember(ddfsClusterMemberPaxos) {
	return (ddfsStatus(DDFS_FAILURE));
}

ddfsStatus ddfsClusterPaxos::addMembers() {
	return (ddfsStatus(DDFS_FAILURE));
}

ddfsStatus ddfsClusterPaxos::deleteMember() {
	return (ddfsStatus(DDFS_FAILURE));
}

ddfsStatus ddfsClusterPaxos::deleteMembers() {
	return (ddfsStatus(DDFS_FAILURE));
}

ddfsClusterMemberPaxos* ddfsClusterPaxos::getLocalNode() {
	return localClusterMember;
}
