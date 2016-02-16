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

#include <unistd.h>

#include "ddfs_clusterPaxosInstance.hpp"
#include "ddfs_clusterMemberPaxos.hpp"
#include "../logger/ddfs_fileLogger.hpp"

using namespace std;

ddfsLogger &global_logger_cpi = ddfsLogger::getInstance();

ddfsClusterPaxosInstance::ddfsClusterPaxosInstance () {
    global_logger_cpi << ddfsLogger::LOG_WARNING << "ddfsClusterPaxosInstance: Constructor Enter.\n";
	internalProposalNumber = -1;
    state = s_paxosState_NONE;
    quorum = 0;

    lastPromised = 0;
    lastAcceptedProposalNumber = 0;
    lastAcceptedValue = 0;
    currentVersionNumber = 0;

    promisesRecieved = 0;
    acceptedRecieved = 0;
    global_logger_cpi << ddfsLogger::LOG_WARNING << "ddfsClusterPaxosInstance: Constructor Return.\n";
}

ddfsClusterPaxosInstance::~ddfsClusterPaxosInstance () {}

ddfsStatus ddfsClusterPaxosInstance::execute (uint64_t roundNumber, uint64_t proposalNumber,
                    uint64_t value, vector<ddfsClusterMemberPaxos *>& allMembers, int *consensusValue)
{
    vector<ddfsClusterMemberPaxos *> participatingMembers;
	vector<ddfsClusterMemberPaxos *>::iterator clusterMemberIter;
	ddfsClusterMessagePaxos message = ddfsClusterMessagePaxos();
	int clusterQuorum = 0;

    if(lastPromised > proposalNumber) {
        global_logger_cpi << ddfsLogger::LOG_WARNING << "Last Promised(" << lastPromised << ") is greater than current proposal number("
                        << proposalNumber << ".\n";
        return (ddfsStatus(DDFS_FAILURE));
    }

    if(lastAcceptedProposalNumber) {
        global_logger_cpi << ddfsLogger::LOG_WARNING << "Last Accepted Proposal Number is " << lastAcceptedProposalNumber << ".\n";
        return (ddfsStatus(DDFS_FAILURE));
    }

	internalProposalNumber = proposalNumber;	

    /* This is not neaded as a node can start a leader election even though
     * another node has already started the election.
    */
#if 0
	if(state != s_paxosState_COMPLETED && state != s_paxosState_NONE)
        return (ddfsStatus(DDFS_FAILURE));
#endif

    unsigned int quorum = (allMembers.size()/2) + 1;
    int count = 0;
    srand(time(NULL));

    /* Push only the quorum members of the cluster to a local list */
    for(clusterMemberIter = allMembers.begin(); clusterMemberIter != allMembers.end(); clusterMemberIter++) {
        global_logger_cpi << ddfsLogger::LOG_WARNING << "Member online state is : " << (*clusterMemberIter)->isOnline() << "\n";
        if((*clusterMemberIter)->isLocalNode() == true) {
            participatingMembers.push_back(*clusterMemberIter);
            count++;
            continue;
        }

        if((*clusterMemberIter)->isOnline() == false) {
            global_logger_cpi << ddfsLogger::LOG_WARNING << "Node " << (*clusterMemberIter)->getHostName() << " is offline\n";
            count++;
            continue;
        }

        if(participatingMembers.size() == quorum)
            break;

        /* Choosing members randomly, until we cannot
         *
         * TODO : This could be done based on proximity or some other
         *        variable rather than random.
          */
        if((allMembers.size() - count) == (quorum -  participatingMembers.size())) {
            participatingMembers.push_back(*clusterMemberIter);
        } else if (rand()%2) {
            participatingMembers.push_back(*clusterMemberIter);
        }

        count++;
    }

    global_logger_cpi << ddfsLogger::LOG_WARNING << "Quorum : " << quorum << "participating size : " << participatingMembers.size() << "\n";
    global_logger_cpi << ddfsLogger::LOG_WARNING << "Participating Members : " << "\n";
    for(clusterMemberIter = participatingMembers.begin(); clusterMemberIter != participatingMembers.end(); clusterMemberIter++) {
        global_logger_cpi << ddfsLogger::LOG_WARNING << "Node " << (*clusterMemberIter)->getHostName() << "\n";
    }

	clusterQuorum = participatingMembers.size();

    if(participatingMembers.size() < quorum)
        return (ddfsStatus(DDFS_CLUSTER_INSUFFICIENT_NODES));


	/* Start the leader Election */
	/* Algorithm is straight formward.
	 * 
	 * Phase 1a: Prepare
	 * Phase 1b: Promise
	 * Phase 2a: Accept Request
	 * Phase 2b: Accepted
	 *
	 * Phase 3: Commit - Commit the consensus that has been just reached.
	 */
	while (1) {

        if(getState() == s_paxosState_COMPLETED) {
            global_logger_cpi << ddfsLogger::LOG_INFO
                << "Paxos :: Leader is already elected.\n";
			return (ddfsStatus(DDFS_FAILURE));
        }

		/*  Send a Prepare cluster message to be send to all the nodes in the cluster.
		 *  Promise cluster message should arrive for all the cluster members or atleast majority of cluster members.
		 */
        global_logger_cpi << ddfsLogger::LOG_INFO
            << "Paxos :: Prepare :: " << internalProposalNumber << "\n";

        message.clearBuffer();
        message.addMessage(roundNumber, CLUSTER_MESSAGE_LE_TYPE_PREPARE, internalProposalNumber, 0, 0);
   
		for(clusterMemberIter = participatingMembers.begin(); clusterMemberIter != participatingMembers.end(); clusterMemberIter++) {
            if((*clusterMemberIter)->isLocalNode() == false) {
			    if( (*clusterMemberIter)->isOnline() == false) {
				    global_logger_cpi << ddfsLogger::LOG_WARNING << "Node " << (*clusterMemberIter)->getUniqueIdentification() << " is offline" << "\n";
			        continue;
                }

                global_logger_cpi << ddfsLogger::LOG_INFO << "ddfsClusterPaxosInstance :: Sending Prepare message to " << (*clusterMemberIter)->getHostName() << ".\n";
			    /* Create packet for the PAXOS Prepare request and send it to the choosen node */
			    (*clusterMemberIter)->sendClusterMetaData(&message);
			}
		}

        /* Local Node is accepting this Paxos Proposal */
        incrementPromiseCount();
        setLastPromised(internalProposalNumber);

        state = s_paxosState_PREPARE;

		/*  Wait for the Promise response from Quorum */
		sleep(s_timeout);
	
		/*  Check the status of this instance of paxos algorithm.
		 *  TODO: Promise message would also contain the vote information.
		 *  	  A vote is tuple of (proposal Number and agreedUponValue).
		 *  	  AgreedUponValue in this case is the id of the elected leader.
		 *
		 *  	  If any promise replied with the AgreedUponValue of anything except -1, then
		 *  	  that AgreedUponValue should be used in the ACCEPT Request and not the local
		 *  	  node's id.
		 */

		if(getPromiseCount() < clusterQuorum) {
        	global_logger_cpi << ddfsLogger::LOG_INFO
            	<< "Paxos :: Exit after Prepare :: " << getPromiseCount() << "\n";
            resetPromiseCount();
			return (ddfsStatus(DDFS_FAILURE));
		}

        global_logger_cpi << ddfsLogger::LOG_INFO
            << "Paxos :: Accept :: " << internalProposalNumber << "\n";

        resetPromiseCount();

        //setLastAcceptedProposalNumber(internalProposalNumber);

        state = s_paxosState_PROMISE_RECV;

        resetAcceptedCount();

        message.clearBuffer();
        message.addMessage(roundNumber, CLUSTER_MESSAGE_LE_ACCEPT_REQUESTED, internalProposalNumber, getLastAcceptedProposalNumber(), getLastAcceptedValue());

		/* Send the accept request to the nodes */
		/* TODO: Should only send the accept request to the set of nodes that responded 
		 * positively to the prepare request.
		 * This is what protocol dictates. SHOULD STRICTLY FOLLOW THE PROTOCOL.
		 */
		for(clusterMemberIter = participatingMembers.begin(); clusterMemberIter != participatingMembers.end(); clusterMemberIter++) {
            /* Create packet for the "Accept Request" request and send it to the choosen node */

            if((*clusterMemberIter)->isLocalNode() == false) {
                global_logger_cpi << ddfsLogger::LOG_INFO << "ddfsClusterPaxosInstance :: Sending Accept Request message to " << (*clusterMemberIter)->getHostName() << ".\n";
			    (*clusterMemberIter)->sendClusterMetaData(&message);

			    //(*clusterMemberIter)->setCurrentState(s_clusterMemberPaxos_LE_ACCEPT_REQUESTED);
		    }
        }
		
        state = s_paxosState_ACCEPT_REQUESTED;

        incrementAcceptedCount();

        if(getLastAcceptedProposalNumber() < internalProposalNumber) {
            global_logger_cpi << ddfsLogger::LOG_INFO << "ddfsClusterPaxosInstance :: Setting the last accepted proposal number to "
                                << internalProposalNumber << "\n";
            setLastAcceptedProposalNumber(internalProposalNumber);
        }

        if(getLastAcceptedValue() == 0)
            setLastAcceptedValue(value);

		/*  Wait for the Accept response from Quorum */
		sleep(s_timeout);

		if(getAcceptedCount() < clusterQuorum) {
        	global_logger_cpi << ddfsLogger::LOG_INFO
            	<< "Paxos :: Exit after Promise :: " << getAcceptedCount() << "\n";
            resetAcceptedCount();
			return (ddfsStatus(DDFS_FAILURE));
		}

        resetAcceptedCount();

        global_logger_cpi << ddfsLogger::LOG_INFO
            << "Paxos :: Commit :: " << internalProposalNumber << "\n";

        message.clearBuffer();
        message.addMessage(roundNumber, CLUSTER_MESSAGE_LE_LEADER_ELECTED, internalProposalNumber, getLastAcceptedProposalNumber() , getLastAcceptedValue());

		state = s_paxosState_COMPLETED;

        /* Commit */
		for(clusterMemberIter = allMembers.begin(); clusterMemberIter != allMembers.end(); clusterMemberIter++) {
            if((*clusterMemberIter)->isLocalNode() == false) {
                global_logger_cpi << ddfsLogger::LOG_INFO << "ddfsClusterPaxosInstance :: Sending Commit message to " << (*clusterMemberIter)->getHostName() << ".\n";
                (*clusterMemberIter)->sendClusterMetaData(&message);
            }
        }

        *consensusValue = getLastAcceptedValue();

		/* This instance has successfully completed. */
		return (ddfsStatus(DDFS_OK));

	} /* End of while */

	return (ddfsStatus(DDFS_FAILURE));
}		/* -----  end of method ddfsClusterPaxosInstance::execute  ----- */

#if 0
ddfsStatus executeAsync(uint64_t proposalNumber, list <ddfsClusterMemberPaxos *>& participatingMembers, ddfsClusterPaxos& cluster) {
	return (ddfsStatus(DDFS_FAILURE));
}		/* -----  end of method ddfsClusterPaxosInstance::executeAsync  ----- */
#endif

void ddfsClusterPaxosInstance::abandon()
{
	return;
}		/* -----  end of method ddfsClusterPaxosInstance::abandon  ----- */

