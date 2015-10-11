/*
 * /file ddfs_clusterMemberPaxos.h 
 *
 * @brief Module containing the cluster member class.
 *
 * This is the module that contains cluster member class.
 *
 * @author Harman Patial <harman.patial@gmail.com>
 *
 */
#ifndef DDFS_CLUSTER_MEMBER_PAXOS_H
#define DDFS_CLUSTER_MEMBER_PAXOS_H

#include <fstream>
#include <string>
#include <mutex>
#include <condition_variable>
#include <thread>
#include <atomic>

#include "ddfs_clusterMember.hpp"
//#include "ddfs_clusterPaxos.hpp"
#include "ddfs_clusterMessagesPaxos.hpp"
#include "../network/ddfs_tcpConnection.hpp"
#include "../global/ddfs_status.hpp"
#include "../logger/ddfs_fileLogger.hpp"

class ddfsClusterPaxos;

enum clusterMemberState {
	s_clusterMemberOnline	= 0, /* cluster member is online. */
	s_clusterMemberOffline,
	s_clusterMemberDead,
	s_clusterMemberRecovering,
	s_clusterMemberUnknown,
	/*  Member state associated with Leader Election.
	 */
	s_clusterMemberPaxos_LE_PREPARE,
	s_clusterMemberPaxos_LE_PROMISE_RECV, /* Remote Node promised local node */
	s_clusterMemberPaxos_LE_PROMISED, /* Local Node promised remote node */
	s_clusterMemberPaxos_LE_ACCEPT_REQUESTED,
	s_clusterMemberPaxos_LE_REQUEST_ACCEPTED,	/* Remote Node accepted request from Local Node */
	s_clusterMemberPaxos_LE_ACCEPTED,	/* Local Node accepted request from Remote Node */
	s_clusterMemberPaxos_LE_COMPLETE,
	s_clusterMemberPaxos_LEADER,	/* Member is the Cluster Leader */
	s_clusterMemberPaxos_SLAVE	/* Member is slave */
};

/*
 * T_clusterMemberState = clusterMemberState
 * T_clusterID = int
 * T_memberID = int
 * T_uniqueID = string
 */
class ddfsClusterMemberPaxos : public ddfsClusterMember<clusterMemberState, int, ddfsClusterMessagePaxos, int, int, ddfsTcpConnection<ddfsClusterMemberPaxos> > {
public:
	ddfsClusterMemberPaxos();

	ddfsClusterMemberPaxos(ddfsClusterPaxos *cp) {
		clusterPaxos = cp;
	}

	~ddfsClusterMemberPaxos();
	ddfsStatus init(string hostn, ddfsClusterMemberPaxos* localNode);
	bool isOnline();
	bool isDead();
	clusterMemberState getCurrentState();
	ddfsStatus setCurrentState(clusterMemberState);

	int getClusterID();
	ddfsStatus setClusterID(int);

	void setMemberID(int);
	int getMemberID();

	void setUniqueIdentification(int );
	int getUniqueIdentification();

	string getHostName();
#if 0
    int getLocalSocket();
#endif
	ddfsStatus sendClusterMetaData(ddfsClusterMessagePaxos *);
    void processingResponses();
    void callback(void *data, int size);
private:
	/* Identifies the cluster, of which this member is part of */
	int clusterID;
	static const int s_invalid_clusterID = -1;
	/* memberID : This is unique ID of a cluster member */
	int memberID;
	static const int s_invalid_memberID = -1;
	/* uniqueIdentifier for using it as Number in Paxos algorithm */
	/* This is mac address shifted left by 12 bits */
	uint32_t uniqueIdentification;

	/* TODO: Should make it  */
	std::atomic<clusterMemberState> memberState;
	/* The network class */
	ddfsTcpConnection <ddfsClusterMemberPaxos> *network;
	/* Mutex lock for this object */
	std::mutex clusterMemberLock;

	/* The ddfs cluster Paxos thing */
	ddfsClusterPaxos* clusterPaxos;

    /* Request and Response queues shared with network layer */
    //std::queue <requestQEntry *> reqQueue;
    //std::queue <responseQEntry *> rspQueue;
    void *networkPrivatePtr;

	bool local_node;

    std::vector<std::thread> workingThreadQ;
    //std::thread workingThread(ddfsClusterMemberPaxos::processingResponses, this);

    ddfsStatus processMessage(ddfsClusterMessage *);

    std::condition_variable needToProcess;
    std::mutex responseQLock;

    /* HostName of this cluster member */
    string hostName;

    bool isLocalNode() {
        if(hostName.compare("localhost") == 0)
            return true;

        return false;
    }

	ddfsClusterMemberPaxos* localNode;
	ddfsClusterMemberPaxos *getLocalNode() {
		return localNode;
	}

	ddfsClusterMemberPaxos(const ddfsClusterMemberPaxos &other);  /* copy constructor */
}; // class end

#endif /* Ending DDFS_CLUSTER_MEMBER_PAXOS_H */
