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

#include "ddfs_clusterMessagesPaxos.hpp"
#include "../global/ddfs_status.hpp"
#include "../logger/ddfs_fileLogger.hpp"

using namespace std;

class ddfsClusterMemberPaxos;

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
		ddfsStatus execute(uint64_t roundNumber, uint64_t proposalNumber, uint64_t value, vector <ddfsClusterMemberPaxos *>& allMembers, int *consesusValue);
		//ddfsStatus executeAsync(uint64_t proposalNumber, vector <ddfsClusterMemberPaxos *>& participatingMembers, ddfsClusterPaxos& cluster);
		/* ====================  MUTATORS      ======================================= */
		void abandon();
		/* ====================  OPERATORS     ======================================= */

		ddfsClusterPaxosInstance& operator = ( const ddfsClusterPaxosInstance &other ); /* assignment operator */

		paxosState getState() {
			return state;
		}

		void setState(paxosState newState) {
            state = newState;
		}

        string getStateString() {
            switch (state) {
                case s_paxosState_NONE:             return "None";
                case s_paxosState_PREPARE:          return "PREPARE";
                case s_paxosState_PROMISE_RECV:     return "PROMISE RECV";
                case s_paxosState_PROMISED:         return "PROMISED";
                case s_paxosState_ACCEPT_REQUESTED: return "ACCEPT REQUESTED";
                case s_paxosState_REQUEST_ACCEPTED: return "REQUESTED ACCEPTED";
                case s_paxosState_COMPLETED:        return "COMPLETED";
                default:                            return "INVALID";
            }
        }

		int getLastPromised() { return lastPromised; }
		void setLastPromised(int newV) { lastPromised = newV; }

		int getLastAcceptedProposalNumber() { return lastAcceptedProposalNumber; }
		void setLastAcceptedProposalNumber(int newV) { lastAcceptedProposalNumber = newV; }

		int getLastAcceptedValue() { return lastAcceptedValue; }
		void setLastAcceptedValue(int newV) { lastAcceptedValue = newV; }

		void incrementPromiseCount() { promisesRecieved++; }
		int getPromiseCount() { return promisesRecieved; }
        void resetPromiseCount() { promisesRecieved = 0; }

		void incrementAcceptedCount() { acceptedRecieved++; }
		int getAcceptedCount() { return acceptedRecieved; }
        void resetAcceptedCount() { acceptedRecieved = 0; }


	protected:
		/* ====================  METHODS       ======================================= */

		/* ====================  DATA MEMBERS  ======================================= */

	private:
		/* ====================  METHODS       ======================================= */
		ddfsClusterPaxosInstance (const ddfsClusterPaxosInstance &other);   /* copy constructor */
        static const int s_timeout = 2;		// In seconds.
		static const int s_paxosInstanceInvalid = -1;
		static const unsigned int s_quorum = 2; // This is a factor value. 2 means totalParticipatingMembers/2. So, half of the all members.

		/* ====================  DATA MEMBERS  ======================================= */
		int internalProposalNumber;
        paxosState state;
        int quorum;
        int promisesRecieved;
        int acceptedRecieved;
//		list <ddfsClusterMemberPaxos>& participatingMembers;
		uint64_t lastPromised;
		uint64_t lastAcceptedProposalNumber;
		uint64_t lastAcceptedValue;
		uint64_t currentVersionNumber;

}; /* -----  end of class ddfsClusterPaxosInstance  ----- */

#endif /*  Ending DDFS_CLUSTER_PAXOS_INSTANCE_H */
