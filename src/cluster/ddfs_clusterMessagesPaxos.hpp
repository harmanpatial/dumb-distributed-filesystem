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

#include "../global/ddfs_status.hpp"

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
	CLUSTER_MESSAGE_TOF_CLUSTER_MGMT = 76,
	CLUSTER_MESSAGE_TOF_CLUSTER_DATA,
	CLUSTER_MESSAGE_TOF_CLUSTER_UNKNOWN
};

enum clusterMessageType {
	CLUSTER_MESSAGE_LE_TYPE_PREPARE = 7,
	CLUSTER_MESSAGE_LE_TYPE_PROMISE,
	CLUSTER_MESSAGE_LE_ACCEPT_REQUESTED,
	CLUSTER_MESSAGE_LE_ACCEPTED,
	CLUSTER_MESSAGE_LE_LEADER_ELECTED,
    /*  Addition - Removal of cluster members */
    CLUSTER_MESSAGE_ADDING_MEMBER,
    CLUSTER_MESSAGE_REMOVING_MEMBER,
    /*  Replica Responsibility */
    CLUSTER_MESSAGE_REPLICA_RESPONSIBILTY_REQUEST,
    CLUSTER_MESSAGE_REPLICA_RESPONSIBILTY_REPLY,
    /*  File INFO */
    CLUSTER_MESSAGE_FILE_INFORMATION_REQUEST,
    CLUSTER_MESSAGE_FILE_INFORMATION_REPLY,
    /*  Write-ahead log info */
    CLUSTER_MESSAGE_LOGFILE_INFORMATION_REQUEST,
    CLUSTER_MESSAGE_LOGFILE_INFORMATION_REPLY,
    /*  Backup related messsages */
    CLUSTER_MESSAGE_CREATE_BOOKMARK_REQUEST,
    CLUSTER_MESSAGE_CREATE_BOOKMARK_REPLY,
};

/******************************************************************

			DDFS Header

0                   1                   2                   3
0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|    Version	|Type of Service|  	Total Length  	        |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|  			    Unique ID
|
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
	uint64_t version;        /* 8 bytes */
	uint64_t typeOfService;  /* 8 bytes -- enum clusterMessageTypeOfService */
	uint64_t totalLength;   /* 8 bytes */
    uint64_t uniqueID;      /* 8 bytes */
	uint64_t internalIndex;	/* 8 bytes -- This is used for Request-Response Queue maintenance */
    uint64_t Reserved1;     /* 8 bytes */
    uint64_t Reserved2;     /* 8 bytes */
} __attribute__((packed)) ddfsClusterHeader;    /* Total 32 bytes */

/*  Packet cluster message */
typedef struct {
    uint16_t messageType;	            /* 2 bytes -- enum clusterMessageType */
    uint16_t Reserved1;                 /* 2 bytes */
    int64_t proposalNumber;             /* 8 bytes */
    int64_t lastAcceptedProposalNumber; /* 8 bytes */
	int64_t lastAcceptedValue;			/* 8 bytes This is value that needs to be send in Promise and Accept Command. */
} __attribute__((packed)) ddfsClusterMessage;    /* Total 28 bytes */

/*  Packet data message */
/*  \note This should only be used by DDFS clients.
 *        Data is send directly from the DDFS client
 *        to individual nodes of the cluster.
 */
#if 0
typedef struct {
    uint16_t dataOffset;
    uint16_t Reserved1;
    uint64_t uniqueID;
} __attribute__((packed)) ddfsClusterData;
#endif

#define MAX_REQUEST_SIZE	256

/* TODO: Not see any usage of this */
typedef struct {
    uint8_t typeOfService;
    uint16_t totalLength;
    uint8_t data[MAX_REQUEST_SIZE];
    /* Following entries in used only for internal data manipulation */
    uint64_t uniqueID;
    void *privateData;
} requestQEntry;

typedef struct {
    uint8_t typeOfService;
    uint16_t totalLength;
    void *data;
} responseQEntry; 

/*!
 *  \class  ddfsClusterMessagePaxos
 *  \brief  This is the class for creating cluster message.
 *  
 *   These messages are essential for correct working of
 *   the cluster.
 */
class ddfsClusterMessagePaxos {
private:
    ddfsClusterHeader ddfsHeader;
    ddfsClusterMessage ddfsMessage;
#if 0
    ddfsClusterData ddfsData;
#endif
    void* message;
public:
	ddfsClusterMessagePaxos();
	~ddfsClusterMessagePaxos();
	virtual ddfsStatus addMessage(uint16_t messageType, uint64_t proposalNumber, uint64_t lastAcceptedProposalNumber, uint64_t lastAcceptedValue);
	virtual void returnBuffer(void *);
	uint64_t returnBufferSize();
};

#endif	/* Ending DDFS_CLUSTER_MESSAGES_PAXOS_H */
