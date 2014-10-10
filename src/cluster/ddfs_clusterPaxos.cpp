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
#include "ddfs_clusterMemberPaxos.h"
#include "ddfs_clusterMessagesPaxos.h"
#include "ddfs_clusterPaxosInstance.h"
#include "../logger/ddfs_fileLogger.h"
#include "../global/ddfs_status.h"

#include <sys/types.h>
#include <ifaddrs.h>
#include <unistd.h>

ddfsLogger &global_logger_cp = ddfsLogger::getInstance();
 
ddfsClusterPaxos::ddfsClusterPaxos() {
	clusterID = s_clusterIDInvalid;
	clusterMemberCount = 1; /* 1 for the local Node */
	paxosProposalNumber = 
		getLocalNode()->getUniqueIdentification();

	localClusterMember = new ddfsClusterMemberPaxos();
    /* Initialize the local node */
    localClusterMember->init("localhost");
	return;
}

ddfsClusterPaxos::~ddfsClusterPaxos() {


}

uint64_t ddfsClusterPaxos::getProposalNumber() {
    paxosProposalNumber++;
    if(paxosProposalNumber == std::numeric_limits<uint64_t>::max()) {
        paxosProposalNumber = 1;
    }
    return paxosProposalNumber;
}

ddfsStatus ddfsClusterPaxos::leaderElection() {
	list<ddfsClusterMemberPaxos *>::iterator clusterMemberIter;
	list<ddfsClusterMemberPaxos *> members;
    uint8_t retryCount = s_retryCountLE;
    bool leaderElectioncomp = false;
	ddfsStatus status(DDFS_FAILURE);
	
    while(leaderElectioncomp == false) {

        /* Remove all the elements from the list */
        members.clear();
        /*  If there is no other node in the cluster, no need to execute leader
        *  election paxos instance.
        */
        if(clusterMemberCount <= 1) {
            global_logger_cp << ddfsLogger::LOG_INFO
                << "Cannot perform leader election. Add more nodes in the cluster";
            global_logger_cp << ddfsLogger::LOG_INFO
                << "Number of members in the cluster : "<< clusterMemberCount;
            return (ddfsStatus(DDFS_CLUSTER_INSUFFICIENT_NODES));
        }

        /* Push all the online members of the cluster to a local list */
        for(clusterMemberIter = clusterMembers.begin(); clusterMemberIter != clusterMembers.end(); clusterMemberIter++) {
            if((*clusterMemberIter)->isOnline().compareStatus(ddfsStatus(DDFS_OK)) == false) {
                global_logger_cp << ddfsLogger::LOG_WARNING << "Node " << (*clusterMemberIter)->getUniqueIdentification() << " is offline";
            }
            /* Push this member to the local members list. */
            members.push_back(*clusterMemberIter);
        }

        /*  Create a ddfsClusterPaxosInstance instance and let it rip
         *  
         *  NOTE : As this is leader election paxos instance, there is no need
         *  	   for async completion of the paxos instance.
         */
        ddfsClusterPaxosInstance paxosInstance; 

        /* Execute the Paxos Instance */
        status = paxosInstance.execute(getProposalNumber(), members);
        if(status.compareStatus(ddfsStatus(DDFS_OK)) == false) {
            retryCount--;
            global_logger_cp << ddfsLogger::LOG_WARNING << "********* LE : PAXOS INSTANCE FAILED : RETRY COUNT : "
                   << retryCount << " *********";
            if(retryCount == 0) {
                global_logger_cp << ddfsLogger::LOG_WARNING << "********* LE : FAILED : "
                   << retryCount << " *********";
            }
                
        } else {
            leaderElectioncomp = true;
        }
    }

    /*  TODO :: Start a thread for trying leader election again */
    if ( leaderElectioncomp == false ) {
        
    }

    global_logger_cp << ddfsLogger::LOG_WARNING << "LEADER ELECTION SUCCESSFULL.";
	return status; 
}

void ddfsClusterPaxos::asyncEventHandling(void *buffer, int bufferCount) {


}

//ddfsStatus ddfsClusterPaxos::addMember(ddfsClusterMemberPaxos *newMember) {
ddfsStatus ddfsClusterPaxos::addMember(string newHostName) {
    
    list<ddfsClusterMemberPaxos *>::iterator clusterMemberIter;
    
    for(clusterMemberIter = clusterMembers.begin(); clusterMemberIter != clusterMembers.end(); clusterMemberIter++) {
            if((*clusterMemberIter)->getHostName().compare(newHostName) == 0) {
                global_logger_cp << ddfsLogger::LOG_WARNING << "Node "
                                << (*clusterMemberIter)->getUniqueIdentification()
                                << " is already configured to be part of cluster";
                return (ddfsStatus(DDFS_CLUSTER_ALREADY_MEMBER));
            }
    }

    /* NOTE : This would always be a remote node.
     *        Local node is added during ClusterPaxos class
     *        initialization.
     */
    ddfsClusterMemberPaxos *newMember = new ddfsClusterMemberPaxos();

    newMember->init(newHostName);
    clusterMembers.push_back(newMember);
    clusterMemberCount++;
	return (ddfsStatus(DDFS_OK));
}

ddfsStatus ddfsClusterPaxos::addMembers() {
	return (ddfsStatus(DDFS_FAILURE));
}

ddfsStatus ddfsClusterPaxos::deleteMember(string removeHostName) {
    list<ddfsClusterMemberPaxos *>::iterator clusterMemberIter;
    ddfsClusterMemberPaxos *deletedMember = NULL;
    bool exists = false;
    
    for(clusterMemberIter = clusterMembers.begin(); clusterMemberIter != clusterMembers.end(); clusterMemberIter++) {
            if((*clusterMemberIter)->getHostName().compare(removeHostName) == 0) {
                exists = true;
                deletedMember = *clusterMemberIter;
            }
    }
    if(exists == false) {
        global_logger_cp << ddfsLogger::LOG_WARNING << "Node "
                    << (*clusterMemberIter)->getUniqueIdentification()
                    << " is not configured to be part of cluster.";
        return (ddfsStatus(DDFS_GENERAL_PARAM_INVALID));
    }

    clusterMembers.remove(deletedMember);

    delete(deletedMember);
    clusterMemberCount--;

	return (ddfsStatus(DDFS_OK));
}

ddfsStatus ddfsClusterPaxos::deleteMembers() {
	return (ddfsStatus(DDFS_FAILURE));
}

ddfsClusterMemberPaxos* ddfsClusterPaxos::getLocalNode() {
	return localClusterMember;
}

