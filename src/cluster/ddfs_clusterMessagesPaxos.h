/*
 * @file ddfs_clusterMessagePaxos.h 
 *
 * @breif Module containing the cluster messages.
 *
 * This is the module that contains cluster messages.
 *
 * @author Harman Patial <harman.patial@gmail.com>
 *
 */
#ifndef DDFS_CLUSTER_MESSAGES_PAXOS_H
#define DDFS_CLUSTER_MESSAGES_PAXOS_H

#include <fstream>
#include <string>
#include <list>

#include "../global/ddfs_status.h"

using namespace std;

/**
 * @class ddfsClusterMessagesPaxos
 *
 * @brief Cluster messages class for DDFS.
 *
 * A cluster messages implementation for the DDFS.
 *
 * @note This would implement all the cluster messages related functions.
 *
 */

enum clusterMessageTypeOfService {
	CLUSTER_MESSAGE_TOF_CLUSTER_MGMT = 0,
	CLUSTER_MESSAGE_TOF_CLUSTER_DATA,
	CLUSTER_MESSAGE_TOF_CLUSTER_UNKNOWN
};

enum clusterMessageType {
	CLUSTER_MESSAGE_TYPE_PREPARE = 0,
	CLUSTER_MESSAGE_TYPE_PROMISE,
	CLUSTER_MESSAGE_ACCEPT_REQUESTED,
	CLUSTER_MESSAGE_ACCEPTED
};

/******************************************************************

			DDFS Header

0                   1                   2                   3
0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|    Version	|Type of Service|  	Total Length  	        |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|			Reserved1				|
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|			Reserved2				|
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+


If type of service is "Cluster Management", bunch of messages can
be attached in this packet. "Total Length".

0                   1                   2                   3
0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|  		Type		|         Unique ID		|
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+

If type of service is "Cluster Management", bunch of messages can
be attached in this packet. "Total Length".

0                   1                   2                   3
0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|  				Data         			|
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+


******************************************************************/

class ddfsClusterMessagePaxos {
private:
	/*  Packet Header */
	uint8_t version;
	uint8_t typeOfService;
	uint16_t totalLength;
	char* message;
public:
	ddfsClusterMessagePaxos();
	~ddfsClusterMessagePaxos();
	virtual ddfsStatus addMessage(uint32_t type, uint32_t uuid);
	virtual void * returnBuffer();
};

#endif	/* Ending DDFS_CLUSTER_MESSAGES_PAXOS_H */
