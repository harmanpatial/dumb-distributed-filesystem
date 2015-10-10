/*!
 *    \file  ddfs_clusterPaxosInstance.h
 *   \brief  Class that describes one instance of Paxos algorithm.
 *  
 *  This class describes one instance of Paxos algorithm.
 *  This is for Paxos instance that local node is "Leader" of.
 *  Any paxos instance in which local node participate, but is not a 
 *  "Leader", will not be managed by local node.
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

#include <vector>

//#include "ddfs_clusterPaxos.hpp"
#include "ddfs_clusterMemberPaxos.hpp"
#include "ddfs_clusterMessagesPaxos.hpp"

#include "../global/ddfs_status.hpp"
#include "../logger/ddfs_fileLogger.hpp"

using namespace std;

enum paxosState {
	/*  Paxos Instance state Leader Election.
	 */
    s_paxosState_NONE = 0,
	s_paxosState_PREPARE,
	s_paxosState_PROMISE_RECV, /* Remote Node promised local node */
	s_paxosState_PROMISED, /* Local Node promised remote node */
	s_paxosState_ACCEPT_REQUESTED,
	s_paxosState_REQUEST_ACCEPTED,	/* Remote Node accepted request from Local Node */
    s_paxosState_COMPLETED,
};

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
		ddfsStatus execute(uint64_t proposalNumber, uint64_t value, vector <ddfsClusterMemberPaxos *>& allMembers, int *consesusValue);
		//ddfsStatus executeAsync(uint64_t proposalNumber, vector <ddfsClusterMemberPaxos *>& participatingMembers, ddfsClusterPaxos& cluster);
		/* ====================  MUTATORS      ======================================= */
		void abandon();
		/* ====================  OPERATORS     ======================================= */

		ddfsClusterPaxosInstance& operator = ( const ddfsClusterPaxosInstance &other ); /* assignment operator */

		paxosState getState() {
			return state;
		}

		int getLastPromised() { return lastPromised; }
		void setLastPromised(int newV) { lastPromised = newV; }
		int getLastAcceptedProposalNumber() { return lastAcceptedProposalNumber; }
		void setLastAcceptedProposalNumber(int newV) { lastAcceptedProposalNumber = newV; }
		int getLastAcceptedValue() { return lastAcceptedValue; }
		void setLastAcceptedValue(int newV) { lastAcceptedValue = newV; }
		void incrementResponses() { responses++; }

	protected:
		/* ====================  METHODS       ======================================= */

		/* ====================  DATA MEMBERS  ======================================= */

	private:
		/* ====================  METHODS       ======================================= */
		ddfsClusterPaxosInstance (const ddfsClusterPaxosInstance &other);   /* copy constructor */
        static const int s_timeout = 2;		// In seconds.
		static const int s_paxosInstanceInvalid = -1;
		static const unsigned int s_quorum = 2; // This is a factor value. 2 means totalParticipatingMembers/2. So, half of the participating members.

		/* ====================  DATA MEMBERS  ======================================= */
		int internalProposalNumber;
        paxosState state;
        int quorum;
        int responses;
//		list <ddfsClusterMemberPaxos>& participatingMembers;
		int lastPromised;
		int lastAcceptedProposalNumber;
		int lastAcceptedValue;
		int currentVersionNumber;

}; /* -----  end of class ddfsClusterPaxosInstance  ----- */

#endif /*  Ending DDFS_CLUSTER_PAXOS_INSTANCE_H */
