/*!
 * \file ddfs_cluster.h 
 *
 * \brief Module containing the cluster class.
 *
 * This is the module that contains cluster class.
 *
 * \author Harman Patial <harman.patial@gmail.com>
 *
 */
#ifndef DDFS_CLUSTER_H
#define DDFS_CLUSTER_H

#include <fstream>
#include <string>
#include <list>

#include "ddfs_clusterMember.h"
#include "../global/ddfs_status.h"

using namespace std;

/**
 * \class ddfsCluster
 *
 * \brief Cluster class for DDFS.
 *
 * A cluster implementation for the DDFS.
 *
 * \note This would implement all the cluster related functions.
 * 	 A singleton class.
 *
 * \note 
 *	 
 */
template <typename T_ddfsClusterMember>
class ddfsCluster {
protected:
	~ddfsCluster();
	list<T_ddfsClusterMember> clusterMembers;
	virtual ddfsStatus leaderElection();
	virtual void asyncEventHandling();
	virtual ddfsStatus addMember(T_ddfsClusterMember);
	virtual ddfsStatus addMembers();
	virtual ddfsStatus deleteMember();
	virtual ddfsStatus deleteMembers();
public:
	static const int s_clusterIDInvalid = -1;
	ddfsCluster();
private:
	int clusterID;
	ddfsCluster(ddfsCluster const&);     // Don't Implement
	void operator=(ddfsCluster const&); // Don't implement
}; // class end

#endif /* Ending DDFS_CLUSTER_H */
