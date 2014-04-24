/*
 * @file ddfs_clusterPaxos.h 
 *
 * @breif Module containing the cluster class.
 *
 * This is the module that contains cluster class.
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
 * @note 
 *	 
 */
 
class ddfsClusterPaxos:protected ddfsCluster<ddfsClusterMemberPaxos> {
private:
	int clusterID;
	static const int s_maxClusterMembers = 4;
protected:
	ddfsStatus init();
	ddfsStatus leaderElection();
	void asyncEventHandling();
	ddfsClusterMemberPaxos clusterMembers[s_maxClusterMembers];
	ddfsStatus addMember(ddfsClusterMemberPaxos);
	ddfsStatus addMembers();
	ddfsStatus deleteMember();
	ddfsStatus deleteMembers();
public:
	ddfsClusterPaxos();
	static const int s_clusterIDInvalid = -1;
}; // class end

#endif /* Ending DDFS_CLUSTER_PAXOS_H */
