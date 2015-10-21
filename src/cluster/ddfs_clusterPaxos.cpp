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
 
ddfsClusterPaxos::ddfsClusterPaxos(string localHostName) {
	clusterID = s_clusterIDInvalid;
	clusterMemberCount = 1; /* 1 for the local Node */
	paxosProposalNumber = 0;

	localClusterMember = new ddfsClusterMemberPaxos();
    /* Initialize the local node */
    localClusterMember->init(localHostName, NULL);
	return;
}

ddfsClusterPaxos::~ddfsClusterPaxos() {


}

uint64_t ddfsClusterPaxos::getProposalNumber() {
    paxosProposalNumber++;
    if(paxosProposalNumber == std::numeric_limits<uint32_t>::max()) {
        paxosProposalNumber = 1;
    }
	
	return ((paxosProposalNumber << 32) | getLocalNode()->getUniqueIdentification());
    //return ((paxosProposalNumber << 32) | clusterMemberID);
}

ddfsStatus ddfsClusterPaxos::leaderElection() {
	vector<ddfsClusterMemberPaxos *>::iterator clusterMemberIter;
	vector<ddfsClusterMemberPaxos *> members;
    uint8_t retryCount = s_retryCountLE;
    bool leaderElectionCompleted = false;
	ddfsStatus status(DDFS_FAILURE);
	
    while(leaderElectionCompleted == false) {

        /* Remove all the elements from the list */
        members.clear();
        /*  If there is no other node in the cluster, no need to execute leader
        *  election paxos instance, make the local node leader.
        */
        if(clusterMemberCount == 1) {
            global_logger_cp << ddfsLogger::LOG_INFO
                << "Only one node in the cluster. Making localnode leader.";
			// Make local node as leader.
			setLeader(getLocalNode(), 1);
			
            return (ddfsStatus(DDFS_CLUSTER_INSUFFICIENT_NODES));
        }

        /*  Create a ddfsClusterPaxosInstance instance and let it rip
         *  
         *  NOTE : As this is leader election paxos instance, there is a
         *         sync. completion.
         */
        /* Execute the Paxos Instance */
		int pr = getProposalNumber();
		int newLeader = -1;
        status = paxosInstance.execute(pr, localClusterMember->getMemberID(), clusterMembers, &newLeader);
        if(status.compareStatus(ddfsStatus(DDFS_OK)) == false) {
			int timeout = 0;

            retryCount--;
            global_logger_cp << ddfsLogger::LOG_WARNING << "********* LE : PAXOS INSTANCE FAILED : RETRYING : "
                   << retryCount << " *********\n";
			srand(time(NULL));
			while(timeout == 0)
				timeout = rand()%400;
			timeout = timeout + 100;

			usleep(timeout);

            if(retryCount == 0) {
                global_logger_cp << ddfsLogger::LOG_WARNING << "********* LE : FAILED : "
                   << retryCount << " *********\n";
				break;
            }
                
        } else {
			/* TODO: Set node with uid of newLeader as the leader */
			//setLeader(getLocalNode(), pr);
			global_logger_cp << ddfsLogger::LOG_WARNING << "********* LE : SELECTED : "
                   << newLeader << " *********\n";
            leaderElectionCompleted = true;
        }
    }

	if(leaderElectionCompleted == true)
		global_logger_cp << ddfsLogger::LOG_WARNING << "LEADER ELECTION SUCCESSFULL.\n";
	else
		global_logger_cp << ddfsLogger::LOG_WARNING << "FAILED LEADER ELECTION.\n";

	return status; 
}

void ddfsClusterPaxos::asyncEventHandling(void *buffer, int bufferCount) {


}

ddfsStatus ddfsClusterPaxos::processMessage (ddfsClusterMemberPaxos *member, ddfsClusterMessage *message) {

    if((paxosInstance.getState() == s_paxosState_NONE) || (paxosInstance.getState() == s_paxosState_COMPLETED)) {
		global_logger_cp << ddfsLogger::LOG_INFO
					<< "ddfsClusterMemberPaxos:: Dropping the message.";
		return (ddfsStatus(DDFS_OK));
	}

    
    global_logger_cp << ddfsLogger::LOG_INFO
                    << "ddfsClusterMemberPaxos:: recieved message type :" << message->messageType << "\n";
	switch (message->messageType) {
		case CLUSTER_MESSAGE_LE_TYPE_PREPARE:
		{
            if(paxosInstance.getLastPromised() < message->proposalNumber) {
                /* Accept the proposal value */
                paxosInstance.setLastPromised(message->proposalNumber);
			}
            /* Send the Promise message across */
            ddfsClusterMessagePaxos reply = ddfsClusterMessagePaxos();

			//setCurrentState(s_clusterMemberPaxos_LE_PROMISED);
			reply.addMessage(CLUSTER_MESSAGE_LE_TYPE_PROMISE, paxosInstance.getLastPromised(),
                    paxosInstance.getLastAcceptedProposalNumber(), paxosInstance.getLastAcceptedValue());
			member->sendClusterMetaData(&reply);
			break;
		}
		case CLUSTER_MESSAGE_LE_TYPE_PROMISE:
		{
			if((paxosInstance.getState() == s_paxosState_PREPARE) && (paxosInstance.getLastPromised() == message->proposalNumber))
                    paxosInstance.incrementResponses();
            /*  This is considered vote <lastAcceptedProposalNumber, lastAcceptedProposalNumber> */
            if((message->lastAcceptedProposalNumber != 0) && (message->lastAcceptedProposalNumber > paxosInstance.getLastAcceptedProposalNumber())) {
                paxosInstance.setLastAcceptedProposalNumber(message->lastAcceptedProposalNumber);
                paxosInstance.setLastAcceptedValue(message->lastAcceptedValue);
            }
			break;
		}
		case CLUSTER_MESSAGE_LE_ACCEPT_REQUESTED:
		{	
			//if((paxosInstance.getCurrentState() == s_clusterMemberPaxos_LE_PROMISED) && (lastProposal == message->proposalNumber)) {
			if(paxosInstance.getLastPromised() == message->proposalNumber) {
				/* Send the Accepted message across */
				ddfsClusterMessagePaxos reply = ddfsClusterMessagePaxos();
				reply.addMessage(CLUSTER_MESSAGE_LE_ACCEPTED, paxosInstance.getLastPromised(),
                        paxosInstance.getLastAcceptedProposalNumber(), paxosInstance.getLastAcceptedValue());
				//setCurrentState(s_clusterMemberPaxos_LE_ACCEPTED);
				member->sendClusterMetaData(&reply);
			}
			break;
		}
		case CLUSTER_MESSAGE_LE_ACCEPTED:
		{	
			if(paxosInstance.getLastAcceptedProposalNumber() == message->lastAcceptedProposalNumber)
                    paxosInstance.incrementResponses();
				//setCurrentState(s_clusterMemberPaxos_LE_COMPLETE);
			break;
		}
		case CLUSTER_MESSAGE_LE_LEADER_ELECTED:
		{	
			//setCurrentState(s_clusterMemberPaxos_LEADER);
			// TODO: Get the unique id of the newly elected leader from the message.
			// 		 and then set it as the leader.
			//setLeader(this, message->proposalNumber);
			break;
		}
		default:
		{
			/* Case : Default */
			global_logger_cp << ddfsLogger::LOG_ERROR
				<< "ddfsClusterMemberPaxos :: Message type is incorrect.\n";
			
			break;
		}
	}

#if 0
	if(message->messageType == CLUSTER_MESSAGE_LE_TYPE_PREPARE) {
		}
	} else if(message->messageType == CLUSTER_MESSAGE_LE_TYPE_PROMISE) {

	} else if(message->messageType == CLUSTER_MESSAGE_LE_ACCEPT_REQUESTED) {

	} else if(message->messageType == CLUSTER_MESSAGE_LE_ACCEPTED) {

	}
#endif
	return (ddfsStatus(DDFS_OK));

}

ddfsStatus ddfsClusterPaxos::addMember(string newHostName) {
    
    vector<ddfsClusterMemberPaxos *>::iterator clusterMemberIter;
    
    global_logger_cp << ddfsLogger::LOG_WARNING << "Add Member Node.\n";

    for(clusterMemberIter = clusterMembers.begin(); clusterMemberIter != clusterMembers.end(); clusterMemberIter++) {
            global_logger_cp << ddfsLogger::LOG_INFO << "host Name : " << (*clusterMemberIter)->getHostName();
            continue;
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
	
	global_logger_cp << ddfsLogger::LOG_INFO << "CLUSTER :: Adding host, "
					<< newHostName << ", to the cluster\n";

    ddfsClusterMemberPaxos *newMember = new ddfsClusterMemberPaxos(this);

    newMember->init(newHostName, getLocalNode());

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

	if(exists == true) {
    	delete(deletedMember);
	    clusterMemberCount--;
	}
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
	message.addMessage(CLUSTER_MESSAGE_LE_LEADER_ELECTED, pr, 0, 0);

	for(iter = clusterMembers.begin(); iter != clusterMembers.end(); iter++) {
		(*iter)->sendClusterMetaData(&message);
		
		if((*iter) == leaderClusterMember)
			(*iter)->setCurrentState(s_clusterMemberPaxos_LEADER);
		else
			(*iter)->setCurrentState(s_clusterMemberPaxos_SLAVE);
	}

}

