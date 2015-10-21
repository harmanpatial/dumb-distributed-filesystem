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
#include "../logger/ddfs_fileLogger.hpp"

using namespace std;

ddfsLogger &global_logger_cpi = ddfsLogger::getInstance();

ddfsClusterPaxosInstance::ddfsClusterPaxosInstance () {
	internalProposalNumber = -1;
    state = s_paxosState_NONE;
    quorum = 0;
    responses = 0;

    lastPromised = -1;
    lastAcceptedProposalNumber = -1;
    lastAcceptedValue = -1;
    currentVersionNumber = -1;

}

ddfsClusterPaxosInstance::~ddfsClusterPaxosInstance () {}

ddfsStatus ddfsClusterPaxosInstance::execute (uint64_t proposalNumber, uint64_t value, vector<ddfsClusterMemberPaxos *>& allMembers, int *consensusValue)
{
    vector<ddfsClusterMemberPaxos *> participatingMembers;
	vector<ddfsClusterMemberPaxos *>::iterator clusterMemberIter;
	ddfsClusterMessagePaxos message = ddfsClusterMessagePaxos();
	int clusterQuorum = (participatingMembers.size()+1)/s_quorum;

	internalProposalNumber = proposalNumber;	

	if(state != s_paxosState_COMPLETED && state != s_paxosState_NONE)
        return (ddfsStatus(DDFS_FAILURE));

    unsigned int quorum = (allMembers.size()/2) + 1;
    int count = 0;
    srand(time(NULL));

    vector<bool> nodeToBeIncluded;

    for (unsigned int i = 0; i < allMembers.size(); i++) {
        nodeToBeIncluded.push_back(false);
    }

    /* Push only the quorum members of the cluster to a local list */
    for(clusterMemberIter = allMembers.begin(); clusterMemberIter != allMembers.end(); clusterMemberIter++) {
        global_logger_cpi << ddfsLogger::LOG_WARNING << "Member online state is : " << (*clusterMemberIter)->isOnline() << "\n";
        if((*clusterMemberIter)->isOnline() == false) {
            global_logger_cpi << ddfsLogger::LOG_WARNING << "Node " << (*clusterMemberIter)->getUniqueIdentification() << " is offline\n";
            count++;
            continue;
        }
        participatingMembers.push_back(*clusterMemberIter);

        if(participatingMembers.size() == quorum)
            continue;

        /* Choosing members randomly, until we cannot */
        if((rand()%2) || ((allMembers.size() - count) == (quorum -  participatingMembers.size()))) {
            global_logger_cpi << ddfsLogger::LOG_WARNING << "Adding host.";
            participatingMembers.push_back(*clusterMemberIter);
        }

        count++;
    }

    global_logger_cpi << ddfsLogger::LOG_WARNING << "Quorum : " << quorum << "participating size : " << participatingMembers.size() << "\n";

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
		/*  Send a Prepare cluster message to be send to all the nodes in the cluster.
		 *  Promise cluster message should arrive for all the cluster members or atleast majority of cluster members.
		 */
        global_logger_cpi << ddfsLogger::LOG_INFO
            << "Paxos :: Prepare :: " << internalProposalNumber << "\n";

		for(clusterMemberIter = participatingMembers.begin(); clusterMemberIter != participatingMembers.end(); clusterMemberIter++) {
			if( (*clusterMemberIter)->isOnline() == false) {
				global_logger_cpi << ddfsLogger::LOG_WARNING << "Node " << (*clusterMemberIter)->getUniqueIdentification() << " is offline" << "\n";
			}

            global_logger_cpi << ddfsLogger::LOG_INFO << "ddfsClusterPaxosInstance :: execute: here1 \n";
			/* Create packet for the PAXOS Prepare request and send it to the choosen node */
			message.addMessage(CLUSTER_MESSAGE_LE_TYPE_PREPARE, internalProposalNumber, 0, 0);

			(*clusterMemberIter)->sendClusterMetaData(&message);
			state = s_paxosState_PREPARE;

			//(*clusterMemberIter)->setCurrentState(s_clusterMemberPaxos_LE_PREPARE);
			//(*clusterMemberIter)->setLastProposal(internalProposalNumber);
		}

        lastPromised = internalProposalNumber;

		/*  Wait for the Promise response from Quorum */
		sleep(s_timeout);
	
		/*  Check the status of this instance of paxos algorithm.
		 *  TODO: Promise message would also contain the vote information.
		 *  	  A vote is tuple of (proposal Number and agreedUponValue).
		 *  	  AgreedUponValue is this case is the id of the elected leader.
		 *
		 *  	  If any promise replied with the AgreedUponValue of anything except -1, then
		 *  	  that AgreedUponValue should be used in the ACCEPT Request and not the local
		 *  	  node's id.
		 */

		if(responses < clusterQuorum) {
        	global_logger_cpi << ddfsLogger::LOG_INFO
            	<< "Paxos :: Exit after Prepare :: " << responses << "\n";
			return (ddfsStatus(DDFS_FAILURE));
		}

        global_logger_cpi << ddfsLogger::LOG_INFO
            << "Paxos :: Accept :: " << internalProposalNumber << "\n";

        lastAcceptedProposalNumber = internalProposalNumber;
		responses = 0;

        if(lastAcceptedValue == 0)
            lastAcceptedValue = value;

		/* Send the accept request to the nodes */
		/* TODO: Should only send the accept request to the set of nodes that responded 
		 * positively to the prepare request.
		 * This is what protocol dictates. SHOULD STRICTLY FOLLOW THE PROTOCOL.
		 */
		for(clusterMemberIter = participatingMembers.begin(); clusterMemberIter != participatingMembers.end(); clusterMemberIter++) {
            /* Create packet for the "Accept Request" request and send it to the choosen node */
			message.addMessage(CLUSTER_MESSAGE_LE_ACCEPT_REQUESTED, internalProposalNumber, lastAcceptedProposalNumber, lastAcceptedValue);

			(*clusterMemberIter)->sendClusterMetaData(&message);

			(*clusterMemberIter)->setCurrentState(s_clusterMemberPaxos_LE_ACCEPT_REQUESTED);
		}
		
		/*  Wait for the Accept response from Quorum */
		sleep(s_timeout);

		if(responses < clusterQuorum) {
        	global_logger_cpi << ddfsLogger::LOG_INFO
            	<< "Paxos :: Exit after Prepare :: " << responses << "\n";
			return (ddfsStatus(DDFS_FAILURE));
		}

        global_logger_cpi << ddfsLogger::LOG_INFO
            << "Paxos :: Commit :: " << internalProposalNumber << "\n";

        /* Commit */
		for(clusterMemberIter = allMembers.begin(); clusterMemberIter != participatingMembers.end(); clusterMemberIter++) {
            message.addMessage(CLUSTER_MESSAGE_LE_LEADER_ELECTED, internalProposalNumber, 0, lastAcceptedValue);
            (*clusterMemberIter)->sendClusterMetaData(&message);
        }

		state = s_paxosState_COMPLETED;

        *consensusValue = lastAcceptedValue;

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

