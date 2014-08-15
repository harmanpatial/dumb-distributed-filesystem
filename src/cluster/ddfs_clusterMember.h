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

template <typename T_clusterMemberState, typename T_clusterID, typename T_ClusterMessage, typename T_memberID, typename T_uniqueID, typename T_network>
class ddfsClusterMember {
public:
	virtual ddfsStatus isOnline() = 0;
	virtual ddfsStatus isDead() = 0;
	
	virtual T_clusterMemberState getCurrentState() = 0;
	virtual ddfsStatus setCurrentState(T_clusterMemberState) = 0;
	
	virtual T_clusterID getClusterID() = 0;
	virtual ddfsStatus setClusterID(T_clusterID) = 0;
	
	virtual void setMemberID(T_memberID) = 0;
	virtual T_memberID getMemberID() = 0;
	
	virtual void setUniqueIdentification(T_uniqueID) = 0;
	virtual int getUniqueIdentification() = 0;
	virtual ddfsStatus sendClusterMetaData(T_ClusterMessage *) = 0;
public:
	static const int s_clusterMemberIdInvalid = -1;
	ddfsClusterMember() {}
    virtual ~ddfsClusterMember() {}
private:
	T_clusterID clusterID;
	T_memberID memberID;
	int uniqueIdentification;
	T_clusterMemberState currentState;
    T_network ddfsNetwork;
}; // class end

#endif /* Ending DDFS_CLUSTER_MEMBER_H */
