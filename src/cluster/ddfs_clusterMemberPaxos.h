/*
 * @file ddfs_clusterMemberPaxos.h 
 *
 * @brief Module containing the cluster class.
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
#include <mutex>
#include <thread>

#include "ddfs_clusterMember.h"
#include "ddfs_clusterMessagesPaxos.h"
#include "../network/ddfs_udpConnection.h"
#include "../global/ddfs_status.h"
#include "../logger/ddfs_fileLogger.h"

enum clusterMemberState {
	s_clusterMemberOnline	= 0, /* cluster member is online. */
	s_clusterMemberOffline,
	s_clusterMemberDead,
	s_clusterMemberRecovering,
	s_clusterMemberUnknown,
	/*  Member state associated with Leader Election */
	s_clusterMemberPaxos_LE_PREPARE,
	s_clusterMemberPaxos_LE_PROMISE,
	s_clusterMemberPaxos_LE_ACCEPT_REQUEST,
	s_clusterMemberPaxos_LE_ACCEPTED,
	s_clusterMemberPaxos_LE_COMPLETE,
	s_clusterMemberPaxos_LEADER,	/* Member is the Cluster Leader */
	s_clusterMemberPaxos_SLAVE	/* Member is slave */
};

/*
 * T_clusterMemberState = clusterMemberState
 * T_clusterID = int
 * T_memberID = int
 * T_uniqueID = int
 */
class ddfsClusterMemberPaxos : public ddfsClusterMember<clusterMemberState, int, ddfsClusterMessagePaxos, int, int, ddfsUdpConnection> {
public:
	ddfsClusterMemberPaxos();
	~ddfsClusterMemberPaxos();
	ddfsStatus init(bool isLocalNode, int serverSocketFD);
	ddfsStatus isOnline();
	ddfsStatus isDead();
	clusterMemberState getCurrentState();
	ddfsStatus setCurrentState(clusterMemberState);
	void setMemberID(int);
	int getMemberID();
	void setUniqueIdentification(int);
	int getUniqueIdentification();
    int getLocalSocket();
	ddfsStatus sendClusterMetaData(ddfsClusterMessagePaxos *);
private:
	/* Identifies the cluster, of which this member is part of */
	int clusterID;
	/* memberID : This is unique ID of a cluster member */
	int memberID;
	/* uniqueIdentifier for using it as Number in Paxos algorithm */
	/* This is mac address shifted left by 12 bits */
	int uniqueIdentification;
	/* Current state of the cluster member */
	clusterMemberState memberState;
	/* Current state of the cluster member */
	ddfsUdpConnection network;
	/* Mutex lock for this object */
	std::mutex clusterMemberLock;

    /* Request and Response queues shared with network layer */
    std::queue <requestQEntry *> requestQueue;
    std::queue <responseQEntry *> responseQueue;
    void *networkPrivatePtr;
    void registerHandler(int);

    std::thread workingThread(int processingDdfsResponses);
    void processingDdfsResponses();

    /* HostName of this cluster member */
    string hostName;
	ddfsClusterMemberPaxos(const ddfsClusterMemberPaxos &other);  /* copy constructor */
}; // class end

#endif /* Ending DDFS_CLUSTER_MEMBER_PAXOS_H */
