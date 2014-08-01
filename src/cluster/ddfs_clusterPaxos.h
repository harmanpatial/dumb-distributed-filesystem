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

#include "ddfs_cluster.h"
#include "ddfs_clusterMemberPaxos.h"
#include "../global/ddfs_status.h"

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
 * addMember*, deleteMember* interface is exposed to the Admin can add or delete members for the cluster.
 *
 */
 
class ddfsClusterPaxos:protected ddfsCluster<ddfsClusterMemberPaxos *> {
private:
	static const uint8_t s_maxClusterMembers = 4;
    static const uint8_t  s_retryCountLE = 5;
	ddfsClusterMemberPaxos* localClusterMember;
	int clusterMemberCount;
protected:
	ddfsStatus init();
	list<ddfsClusterMemberPaxos *> clusterMembers;
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
	void asyncEventHandling(void *buffer, int bufferCount);
	ddfsStatus addMember(ddfsClusterMemberPaxos *);
	ddfsStatus addMembers();    /* Does nothing at this point */
	ddfsStatus deleteMember(ddfsClusterMemberPaxos *);
	ddfsStatus deleteMembers(); /* Does nothing at this point */
	
	/* Methods specific to Paxos algorithm */
	ddfsClusterMemberPaxos* getLocalNode();
public:
	ddfsClusterPaxos();
	~ddfsClusterPaxos();
	static const int s_clusterIDInvalid = -1;
}; // class end

#endif /* Ending DDFS_CLUSTER_PAXOS_H */
