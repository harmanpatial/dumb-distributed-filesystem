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
#include "../logger/ddfs_fileLogger.hpp"
#include "../global/ddfs_status.hpp"

#include <sys/types.h>
#include <ifaddrs.h>
#include <unistd.h>

ddfsLogger &global_logger_cp = ddfsLogger::getInstance();
 
ddfsClusterPaxos::ddfsClusterPaxos(string localHostName) {
	clusterID = s_clusterIDInvalid;
	paxosProposalNumber = 88;

	localClusterMember = new ddfsClusterMemberPaxos();
    clusterMembers.push_back(localClusterMember);
    clusterMemberCount++;

    leaderPaxosInstance = new ddfsClusterPaxosInstance();
    /* Initialize the local node */
    localClusterMember->init(localHostName, NULL);
	return;
}

ddfsClusterPaxos::~ddfsClusterPaxos() {

    delete(localClusterMember);
    delete(leaderClusterMember);

    delete(leaderPaxosInstance);
}

uint64_t ddfsClusterPaxos::getProposalNumber() {
    paxosProposalNumber++;

    /* This is the right way to get the proposal number. */
    /*  ProposalNumber  */
    uint64_t realNumber = (uint64_t)(paxosProposalNumber << 16);
    global_logger_cp << ddfsLogger::LOG_INFO
                << "First 48 bits of Proposal Number : " << realNumber << "\n"; 
    uint16_t uniq = (uint16_t) getLocalNode()->getUniqueIdentification();
    realNumber |= uniq;
    global_logger_cp << ddfsLogger::LOG_INFO
                << "Current Proposal Number : " << realNumber << "\n"; 
	return realNumber;
}

ddfsStatus ddfsClusterPaxos::leaderElection() {
	vector<ddfsClusterMemberPaxos *>::iterator clusterMemberIter;
	vector<ddfsClusterMemberPaxos *> members;
    uint8_t retryCount = s_retryCountLE;
    bool leaderElectionCompleted = false;
	ddfsStatus status(DDFS_FAILURE);
    int newLeader = -1;
	
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
			setLeader(getLocalNode()->getMemberID());
			
            return (ddfsStatus(DDFS_CLUSTER_INSUFFICIENT_NODES));
        }

        /*  Create a ddfsClusterPaxosInstance instance and let it rip
         *  
         *  NOTE : As this is leader election paxos instance, there is a
         *         sync. completion.
         */
        /* Execute the Paxos Instance */
		int pr = getProposalNumber();
        int roundNumber = 0;

        status = leaderPaxosInstance->execute(roundNumber, pr, localClusterMember->getMemberID(), clusterMembers, &newLeader);
        if(status.compareStatus(ddfsStatus(DDFS_OK)) == false) {
			int timeout = 0;

            if(getLeader() != NULL) {
                global_logger_cp << ddfsLogger::LOG_WARNING << "********* LE : LEADER ELECTION SUCCESSFULL. New Leader : "
                            << leaderClusterMember->getHostName() << "\n";
                return (ddfsStatus(DDFS_OK));
            }

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

	if(leaderElectionCompleted == true) {
        setLeader(newLeader);
		global_logger_cp << ddfsLogger::LOG_WARNING << "LEADER ELECTION SUCCESSFULL. New Leader : " << leaderClusterMember->getHostName() << ".\n";
}
	else
		global_logger_cp << ddfsLogger::LOG_WARNING << "FAILED LEADER ELECTION.\n";

	return status; 
}

void ddfsClusterPaxos::asyncEventHandling(void *buffer, int bufferCount) {


}

ddfsStatus ddfsClusterPaxos::processMessage (ddfsClusterMemberPaxos *member, ddfsClusterMessage *message) {

    global_logger_cp << ddfsLogger::LOG_INFO
                    << "ddfsClusterPaxos::processMessage recieved message type :" << message->messageType << "\n";
    //global_logger_cp << ddfsLogger::LOG_INFO
    //                << "ddfsClusterPaxos::processMessage current state :" << leaderPaxosInstance->getStateString() << "\n";
#if 0
    if((leaderPaxosInstance->getState() == s_paxosState_COMPLETED) && (getLeader() != NULL) && (getLeader()->isOnline() == true) {
		global_logger_cp << ddfsLogger::LOG_INFO
					<< "ddfsClusterPaxos::processMessage Leader is already elected : Drop this packet.";
		return (ddfsStatus(DDFS_OK));
	}
#endif
    
    global_logger_cp << ddfsLogger::LOG_INFO
                    << "ddfsClusterMemberPaxos:: recieved message type :" << message->messageType << "\n";
    global_logger_cp << ddfsLogger::LOG_INFO
                    << "ddfsClusterMemberPaxos:: Message has Number :" << message->proposalNumber << "\n";
	switch (message->messageType) {
		case CLUSTER_MESSAGE_LE_TYPE_PREPARE:
		{
            if(getRoundNumber() > message->roundNumber) {
                global_logger_cp << ddfsLogger::LOG_INFO
                    << "My Current round number is greater than the message's round Number.\n";
                // TODO : Send my accepted value(vote) in round -- message->roundNumber
                global_logger_cp << ddfsLogger::LOG_INFO
                    << "Discarding the Message. Ideally should be sending my vote in round -- message->roundNumber\n";
                break;
            }
            if(leaderPaxosInstance->getLastPromised() < message->proposalNumber) {
                /* Accept the proposal value */
                global_logger_cp << ddfsLogger::LOG_INFO << "Accepting the propose request : " << message->proposalNumber << ".\n";
                leaderPaxosInstance->setLastPromised(message->proposalNumber);
			}
            /* Send the Promise message across */
            ddfsClusterMessagePaxos reply = ddfsClusterMessagePaxos();

			//setCurrentState(s_clusterMemberPaxos_LE_PROMISED);
			reply.addMessage(message->roundNumber, CLUSTER_MESSAGE_LE_TYPE_PROMISE, leaderPaxosInstance->getLastPromised(),
                    leaderPaxosInstance->getLastAcceptedProposalNumber(), leaderPaxosInstance->getLastAcceptedValue());
			member->sendClusterMetaData(&reply);
			break;
		}
		case CLUSTER_MESSAGE_LE_TYPE_PROMISE:
		{
			if((leaderPaxosInstance->getState() == s_paxosState_PREPARE) && (leaderPaxosInstance->getLastPromised() == message->proposalNumber)) {
                global_logger_cp << ddfsLogger::LOG_INFO << "Got one Promise.\n";
                leaderPaxosInstance->incrementPromiseCount();
                global_logger_cp << ddfsLogger::LOG_INFO << "Total Promises so far : " << leaderPaxosInstance->getPromiseCount() << ".\n";
            }
            /*  This is considered vote <lastAcceptedProposalNumber, lastAcceptedProposalValue> */
            if((message->lastAcceptedProposalNumber != 0) && (message->lastAcceptedValue != leaderPaxosInstance->getLastAcceptedValue())) {
                leaderPaxosInstance->setLastAcceptedProposalNumber(message->lastAcceptedProposalNumber);
                leaderPaxosInstance->setLastAcceptedValue(message->lastAcceptedValue);
            }
			break;
		}
		case CLUSTER_MESSAGE_LE_ACCEPT_REQUESTED:
		{	
			if(leaderPaxosInstance->getLastPromised() == message->proposalNumber) {
                leaderPaxosInstance->setLastAcceptedProposalNumber(message->proposalNumber);
                leaderPaxosInstance->setLastAcceptedValue(message->lastAcceptedValue);

				/* Send the Accepted message across */
                global_logger_cp << ddfsLogger::LOG_INFO << "Accept Request for : " << message->proposalNumber << ".\n";
				ddfsClusterMessagePaxos reply = ddfsClusterMessagePaxos();
				reply.addMessage(message->roundNumber, CLUSTER_MESSAGE_LE_ACCEPTED, leaderPaxosInstance->getLastAcceptedProposalNumber(),
                        leaderPaxosInstance->getLastAcceptedProposalNumber(), leaderPaxosInstance->getLastAcceptedValue());
				member->sendClusterMetaData(&reply);

				//setCurrentState(s_clusterMemberPaxos_LE_ACCEPTED);
			}
			break;
		}
		case CLUSTER_MESSAGE_LE_ACCEPTED:
		{
            global_logger_cp << ddfsLogger::LOG_INFO << "last accepted proposal number : "
                            << leaderPaxosInstance->getLastAcceptedProposalNumber() << "\n";
			if(leaderPaxosInstance->getLastAcceptedProposalNumber() == message->lastAcceptedProposalNumber) {
                    global_logger_cp << ddfsLogger::LOG_INFO << "Got one Accepted.\n";
                    leaderPaxosInstance->incrementAcceptedCount();
                    global_logger_cp << ddfsLogger::LOG_INFO << "Total Accepted so far : " << leaderPaxosInstance->getAcceptedCount() << ".\n";
            }
				//setCurrentState(s_clusterMemberPaxos_LE_COMPLETE);
			break;
		}
		case CLUSTER_MESSAGE_LE_LEADER_ELECTED:
		{	
			//setCurrentState(s_clusterMemberPaxos_LEADER);
			// TODO: Get the unique id of the newly elected leader from the message.
			// 		 and then set it as the leader.
			global_logger_cp << ddfsLogger::LOG_WARNING << "Leader is Member ID : " << message->lastAcceptedValue << "\n";
			setLeader(message->lastAcceptedValue);
			global_logger_cp << ddfsLogger::LOG_WARNING << "New Leader is : " << getLeader()->getHostName() << "\n";
			leaderPaxosInstance->setState(s_paxosState_COMPLETED);
			break;
		}
		default:
		{
			/* Case : Default */
			global_logger_cp << ddfsLogger::LOG_ERROR
				<< "ddfsClusterPaxos :: Message type is incorrect." << message->messageType << "\n";
			
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
            global_logger_cp << ddfsLogger::LOG_INFO << "host Name : " << (*clusterMemberIter)->getHostName() << "\n";
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
	
	global_logger_cp << ddfsLogger::LOG_INFO << "CLUSTER :: Adding node "
					<< newHostName << " to the cluster\n";

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

void ddfsClusterPaxos::setLeader(int leaderMemberID) {
	vector<ddfsClusterMemberPaxos *>::iterator iter;
	ddfsClusterMessagePaxos message = ddfsClusterMessagePaxos();

#if 0
	message.addMessage(CLUSTER_MESSAGE_LE_LEADER_ELECTED, pr, 0, leaderMemberID);
#endif

    global_logger_cp << ddfsLogger::LOG_INFO << "CLUSTER :: setLeader : "
                << leaderMemberID << "\n";
	for(iter = clusterMembers.begin(); iter != clusterMembers.end(); iter++) {
        if((*iter)->getMemberID() == leaderMemberID) {
			global_logger_cp << ddfsLogger::LOG_WARNING << "Leader is : " << (*iter)->getMemberID() << "\n";
	        leaderClusterMember = (*iter);
			(*iter)->setCurrentState(s_clusterMemberPaxos_LEADER);
        } else {
			(*iter)->setCurrentState(s_clusterMemberPaxos_SLAVE);
        }

#if 0
		(*iter)->sendClusterMetaData(&message);
#endif
    }

}
