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

#include "ddfs_clusterPaxos.hpp"
#include "ddfs_clusterMemberPaxos.hpp"
#include "ddfs_clusterMessagesPaxos.hpp"
#include "ddfs_clusterPaxosInstance.hpp"
#include "../logger/ddfs_fileLogger.hpp"
#include "../global/ddfs_status.hpp"

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
	vector<ddfsClusterMemberPaxos *>::iterator clusterMemberIter;
	vector<ddfsClusterMemberPaxos *> members;
    uint8_t retryCount = s_retryCountLE;
    bool leaderElectioncomp = false;
	ddfsStatus status(DDFS_FAILURE);
	
    while(leaderElectioncomp == false) {

        /* Remove all the elements from the list */
        members.clear();
        /*  If there is no other node in the cluster, no need to execute leader
        *  election paxos instance, make the local node leader.
        */
        if(clusterMemberCount == 1) {
            global_logger_cp << ddfsLogger::LOG_INFO
                << "Only one node in the cluster. Making localnode leader.";
			// TODO : Make local node as leader.
			setLeader(getLocalNode(), 1);
			
            return (ddfsStatus(DDFS_CLUSTER_INSUFFICIENT_NODES));
        }

        /* Push all the online members of the cluster to a local list */
        for(clusterMemberIter = clusterMembers.begin(); clusterMemberIter != clusterMembers.end(); clusterMemberIter++) {
            if((*clusterMemberIter)->isOnline() == false) {
                global_logger_cp << ddfsLogger::LOG_WARNING << "Node " << (*clusterMemberIter)->getUniqueIdentification() << " is offline";
            }
            /* Push this member to the local members list. */
            members.push_back(*clusterMemberIter);
        }

        /*  Create a ddfsClusterPaxosInstance instance and let it rip
         *  
         *  NOTE : As this is leader election paxos instance, there is no need
         *  	   for async completion.
         */
        ddfsClusterPaxosInstance paxosInstance; 

        /* Execute the Paxos Instance */
		int pr = getProposalNumber();
        status = paxosInstance.execute(pr, members);
        if(status.compareStatus(ddfsStatus(DDFS_OK)) == false) {
			int timeout = 0;

            retryCount--;
            global_logger_cp << ddfsLogger::LOG_WARNING << "********* LE : PAXOS INSTANCE FAILED : RETRYING : "
                   << retryCount << " *********";
			srand(time(NULL));
			while(timeout == 0)
				timeout = rand()%400;
			timeout = timeout + 100;

			usleep(timeout);

            if(retryCount == 0) {
                global_logger_cp << ddfsLogger::LOG_WARNING << "********* LE : FAILED : "
                   << retryCount << " *********";
				break;
            }
                
        } else {
			setLeader(getLocalNode(), pr);
            leaderElectioncomp = true;
        }
    }

	if(leaderElectioncomp == true)
		global_logger_cp << ddfsLogger::LOG_WARNING << "LEADER ELECTION SUCCESSFULL.";
	else
		global_logger_cp << ddfsLogger::LOG_WARNING << "FAILED LEADER ELECTION.";

	return status; 
}

void ddfsClusterPaxos::asyncEventHandling(void *buffer, int bufferCount) {


}

//ddfsStatus ddfsClusterPaxos::addMember(ddfsClusterMemberPaxos *newMember) {
ddfsStatus ddfsClusterPaxos::addMember(string newHostName) {
    
    vector<ddfsClusterMemberPaxos *>::iterator clusterMemberIter;
    
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

ddfsStatus ddfsClusterPaxos::removeMember(string removeHostName) {
    vector<ddfsClusterMemberPaxos *>::iterator clusterMemberIter;
    ddfsClusterMemberPaxos *deletedMember = NULL;
    bool exists = false;
    
    for(clusterMemberIter = clusterMembers.begin(); clusterMemberIter != clusterMembers.end();) {
            if((*clusterMemberIter)->getHostName().compare(removeHostName) == 0) {
                exists = true;
				deletedMember = *clusterMemberIter;
                clusterMembers.erase(clusterMemberIter);
				break;
            }
			else
				clusterMemberIter++;
    }

    delete(deletedMember);
    clusterMemberCount--;

	return (ddfsStatus(DDFS_OK));
}

ddfsStatus ddfsClusterPaxos::removeMembers() {
	return (ddfsStatus(DDFS_FAILURE));
}

ddfsClusterMemberPaxos* ddfsClusterPaxos::getLocalNode() {
	return localClusterMember;
}

ddfsClusterMemberPaxos* ddfsClusterPaxos::getLeader() { return leaderClusterMember; }

void ddfsClusterPaxos::setLeader(ddfsClusterMemberPaxos* latest_leader, int pr) {
	vector<ddfsClusterMemberPaxos *>::iterator iter;
	ddfsClusterMessagePaxos message = ddfsClusterMessagePaxos();

	leaderClusterMember = latest_leader;
	message.addMessage(CLUSTER_MESSAGE_LE_LEADER_ELECTED, pr);

	for(iter = clusterMembers.begin(); iter != clusterMembers.end(); iter++) {
		(*iter)->sendClusterMetaData(&message);
		
		if((*iter) == leaderClusterMember)
			(*iter)->setCurrentState(s_clusterMemberPaxos_LEADER);
		else
			(*iter)->setCurrentState(s_clusterMemberPaxos_SLAVE);
	}

}

