/*
 * @file ddfs_clusterPaxos.h
 *
 * @brief 
 *
 * This is the module that contains cluster class implementing Paxos consensus algorithm.
 * Paxos.
 *
 * @author Harman Patial <harman.patial@gmail.com>
 *
 */
#ifndef DDFS_CLUSTER_PAXOS_H
#define DDFS_CLUSTER_PAXOS_H

#include <fstream>
#include <string>
#include <vector>

#include "ddfs_cluster.hpp"
#include "ddfs_clusterMessagesPaxos.hpp"
// Harman #include "ddfs_clusterMemberPaxos.hpp"
#include "ddfs_clusterPaxosInstance.hpp"
#include "../global/ddfs_status.hpp"

using namespace std;

/**
 * @class ddfsClusterPaxos
 *
 * @brief Cluster class for DDFS.
 *
 * A cluster implementation for the DDFS.
 *
 * @note This would implement all the cluster related functions.
 * 	 A singleton class.
 *
 * @note This is primarily exposed to Administrator.
 * addMember*, removeMember* interface is exposed to the Admin can add or remove members for the cluster.
 *
 */

class ddfsClusterMemberPaxos; 
class ddfsClusterPaxosInstance;

class ddfsClusterPaxos:public ddfsCluster<ddfsClusterMemberPaxos *, string> {
private:
    /* Maximum Cluster Members */
	static const unsigned int s_maxClusterMembers = 5;

    /* Maximum Retries for a Leader Election */
    static const unsigned int s_retryCountLE = 5;
	ddfsClusterMemberPaxos* localClusterMember;
	ddfsClusterMemberPaxos* leaderClusterMember;
    /* Current cluster Member Count */
	int clusterMemberCount;
	uint32_t clusterMemberID;

    ddfsClusterPaxosInstance *leaderPaxosInstance;
    int64_t internalRoundNumber;

public:
	ddfsStatus init();
    /* All the cluster Members including the local Node */
	vector<ddfsClusterMemberPaxos *> clusterMembers;
	/*
	 * Function that would contain the logic to perform leader election.
	 *
	 * TODO : If this function fails, a seperate thread should be created
	 *        that would keep on trying this leader election algorithm.
	 *        That thread would only stop when local node becomes part of
	 *        a cluster(either member or a master node).
	 */
	ddfsStatus leaderElection();
	uint64_t getProposalNumber();

    void incrementRoundNumber() {
        internalRoundNumber++;
    }

    int64_t getRoundNumber() {
       return internalRoundNumber;
    }

	void asyncEventHandling(void *buffer, int bufferCount);
	ddfsStatus processMessage (ddfsClusterMemberPaxos *member, ddfsClusterMessage *message);
	ddfsStatus addMember(string addHostName);
	ddfsStatus addMembers();    /* Does nothing at this point */
	ddfsStatus removeMember(string removeHostName);
	ddfsStatus removeMembers(); /* Does nothing at this point */
	
	/* Methods specific to Paxos algorithm */
	ddfsClusterMemberPaxos* getLocalNode();
	ddfsClusterMemberPaxos* getLeader();
	ddfsClusterMemberPaxos* getMemberByID();

public:
	ddfsClusterPaxos(string localHostName);
	~ddfsClusterPaxos();
	static const int s_clusterIDInvalid = -1;
	void setLeader(int leaderMemberID);
}; // class end

#endif /* Ending DDFS_CLUSTER_PAXOS_H */
