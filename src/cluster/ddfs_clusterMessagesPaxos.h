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
	CLUSTER_MESSAGE_LE_TYPE_PREPARE = 0,
	CLUSTER_MESSAGE_LE_TYPE_PROMISE,
	CLUSTER_MESSAGE_LE_ACCEPT_REQUESTED,
	CLUSTER_MESSAGE_LE_ACCEPTED
};

/******************************************************************

			DDFS Header

0                   1                   2                   3
0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|    Version	|Type of Service|  	Total Length  	        |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|			Reserved1				
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|			Reserved2				
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+


If type of service is "Cluster Management", bunch of messages can
be attached in this packet. "Total Length".

0                   1                   2                   3
0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|  		Message Type			|         Reserved1
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|  			    Unique ID
|
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+

If type of service is "Cluster Management", bunch of messages can
be attached in this packet. "Total Length".

0                   1                   2                   3
0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|  			 Data Offset	    |       Reserved1
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|  			        Unique ID				
|
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|  				            DATA
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+


******************************************************************/

/*  Packet Header */
typedef struct {
	uint8_t version;
	uint8_t typeOfService;
	uint16_t totalLength;
    uint32_t Reserved1;
    uint32_t Reserved2;
} __attribute__((packed)) ddfsClusterHeader;

/*  Packet cluster message */
typedef struct {
    uint16_t messageType;
    uint16_t Reserved1;
    uint64_t uniqueID;
} __attribute__((packed)) ddfsClusterMessage;

/*  Packet data message */
/*  \note This should only be used by DDFS clients.
 *        Data is send directly from the DDFS client
 *        to individual nodes of the cluster.
 */
typedef struct {
    uint16_t dataOffset;
    uint16_t Reserved1;
    uint64_t uniqueID;
} __attribute__((packed)) ddfsClusterData;

/*!
 *  \class  ddfsClusterMessagePaxos
 *  \brief  This is the class for forming the cluster message.
 *  
 *   These messages are essential for correct working of
 *   the cluster.
 */
class ddfsClusterMessagePaxos {
private:
    ddfsClusterHeader ddfsHeader;
    ddfsClusterMessage ddfsMessage;
    ddfsClusterData ddfsData;
	void* message;
public:
	ddfsClusterMessagePaxos();
	~ddfsClusterMessagePaxos();
	virtual ddfsStatus addMessage(uint16_t type, uint64_t uuid);
	virtual void * returnBuffer();
	uint64_t returnBufferSize();
};

#endif	/* Ending DDFS_CLUSTER_MESSAGES_PAXOS_H */
