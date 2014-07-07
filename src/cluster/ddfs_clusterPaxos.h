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
 * addMember*, deleteMember* interface is exposed to the Admin can add or delete members for the cluster.
 *
 */
 
class ddfsClusterPaxos:protected ddfsCluster<ddfsClusterMemberPaxos> {
private:
	static const int s_maxClusterMembers = 4;
	ddfsClusterMemberPaxos* localClusterMember;
	int clusterMemberCount;
protected:
	ddfsStatus init();
	list<ddfsClusterMemberPaxos *> clusterMembers;
	/*
	 * Function that would contain the logic to perform leader election.
	 */
	ddfsStatus leaderElection();
	uint64_t getProposalNumber();
	void asyncEventHandling();
	ddfsStatus addMember(ddfsClusterMemberPaxos);
	ddfsStatus addMembers();
	ddfsStatus deleteMember();
	ddfsStatus deleteMembers();
	
	/* Methods specific to Paxos algorithm */
	ddfsClusterMemberPaxos* getLocalNode();
public:
	ddfsClusterPaxos();
	~ddfsClusterPaxos();
	static const int s_clusterIDInvalid = -1;
}; // class end

#endif /* Ending DDFS_CLUSTER_PAXOS_H */
