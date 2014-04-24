/*
 * @file ddfs_cluster.h 
 *
 * @breif Module containing the cluster class.
 *
 * This is the module that contains cluster class.
 *
 * @author Harman Patial <harman.patial@gmail.com>
 *
 */
#ifndef DDFS_CLUSTER_H
#define DDFS_CLUSTER_H

#include <fstream>
#include <string>

#include "ddfs_clusterMember.h"
#include "../global/ddfs_status.h"

using namespace std;

/**
 * @class ddfsCluster
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

//#include "ddfs_clusterMember.h"
#include "../global/ddfs_status.h"

template <typename T_ddfsClusterMember>
class ddfsCluster {
protected:
	~ddfsCluster();
	virtual ddfsStatus init();
	virtual ddfsStatus leaderElection();
	virtual void asyncEventHandling();
//	T_ddfsClusterMember clusterMembers[ddfsCluster.s_max_cluster_members];
	T_ddfsClusterMember clusterMembers[4];
	virtual ddfsStatus addMember(T_ddfsClusterMember);
	virtual ddfsStatus addMembers();
	virtual ddfsStatus deleteMember();
	virtual ddfsStatus deleteMembers();
public:
	static const int s_clusterIDInvalid = -1;
private:
	int clusterID;
	ddfsCluster();
	ddfsCluster(ddfsCluster const&);     // Don't Implement
	void operator=(ddfsCluster const&); // Don't implement
}; // class end

#endif /* Ending DDFS_CLUSTER_H */
