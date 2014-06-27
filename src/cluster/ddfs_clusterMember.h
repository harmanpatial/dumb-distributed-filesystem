/*
 * @file ddfs_clusterMember.h 
 *
 * @breif Module containing the cluster member class.
 *
 * This is the module that contains cluster member class.
 *
 * @author Harman Patial <harman.patial@gmail.com>
 *
 */
#ifndef DDFS_CLUSTER_MEMBER_H
#define DDFS_CLUSTER_MEMBER_H

#include <fstream>
#include <string>

#include "../global/ddfs_status.h"

using namespace std;

/**
 * @class ddfsClusterMember
 *
 * @brief Cluster Member class for DDFS.
 *
 * A cluster member implementation for the DDFS.
 *
 * @note This would implement all the cluster member related functions.
 * 	 A singleton class.
 *
 * @note 
 *	 
 */

template <typename T_clusterMemberState, typename T_clusterID, typename T_ClusterMessage, typename T_memberID, typename T_uniqueID>
class ddfsClusterMember {
public:
	virtual ddfsStatus isOnline();
	virtual ddfsStatus isDead();
	
	virtual T_clusterMemberState getCurrentState();
	virtual ddfsStatus setCurrentState(T_clusterMemberState);
	
	virtual T_clusterID getClusterID();
	virtual ddfsStatus setClusterID(T_clusterID);
	
	virtual void setMemberID(T_memberID);
	virtual T_memberID getMemberID();
	
	virtual void setUniqueIdentification(T_uniqueID);
	virtual int getUniqueIdentification();
	virtual ddfsStatus sendClusterMetaData(T_ClusterMessage *);
public:
	static const int s_clusterMemberIdInvalid = -1;
	ddfsClusterMember();
private:
	T_clusterID clusterID;
	T_memberID memberID;
	int uniqueIdentification;
	T_clusterMemberState currentState;
}; // class end

#endif /* Ending DDFS_CLUSTER_MEMBER_H */
