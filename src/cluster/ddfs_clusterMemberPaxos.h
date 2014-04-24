/*
 * @file ddfs_clusterMemberPaxos.h 
 *
 * @breif Module containing the cluster class.
 *
 * This is the module that contains cluster class.
 *
 * @author Harman Patial <harman.patial@gmail.com>
 *
 */
#ifndef DDFS_CLUSTER_MEMBER_PAXOS_H
#define DDFS_CLUSTER_MEMBER_PAXOS_H

#include <fstream>
#include <string>

#include "ddfs_clusterMember.h"
#include "../global/ddfs_status.h"

using namespace std;

enum clusterMemberState {
	s_clusterMemberOnline	= 0,
	s_clusterMemberOffline,
	s_clusterMemberDead	,
	s_clusterMemberRecovering,
	s_clusterMemberUnknown
};

/*
 * T_clusterMemberState = clusterMemberState
 * T_clusterID = int
 * T_memberID = int
 * T_uniqueID = int
 */
class ddfsClusterMemberPaxos : public ddfsClusterMember<clusterMemberState, int, int, int> {
protected:
	ddfsStatus init();
	ddfsStatus isOnline();
	ddfsStatus isDead();
	clusterMemberState getCurrentState();
	ddfsStatus setCurrentState(clusterMemberState);
	void setMemberID(int);
	int getMemberID();
	void setUniqueIdentification(int);
	int getUniqueIdentification();
public:
private:
	int clusterID;
	int memberID;
	int uniqueIdentification;
	int currentState;
}; // class end

#endif /* Ending DDFS_CLUSTER_MEMBER_PAXOS_H */
