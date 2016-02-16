/*
 * @file ddfs_clusterMessagePaxos.cpp
 *
 * @breif Module containing the cluster messages.
 *
 * This is the module that contains cluster messages.
 *
 * @author Harman Patial <harman.patial@gmail.com>
 *
 */

#include <iostream>
#include <string>
#include <arpa/inet.h>
#include <cstring>
#include <stdlib.h>

#include "ddfs_clusterMessagesPaxos.hpp"
#include "../global/ddfs_status.hpp"

/**
 * @class ddfsClusterMessageMessages
 *
 * @brief Cluster messages class for DDFS.
 *
 * A cluster messages implementation for the DDFS.
 *
 * @note This would implement all the cluster messages related functions.
 *
 */

#define SIZE_OF_HEADER		sizeof(ddfsClusterHeader)
#define SIZE_OF_MESSAGE		sizeof(ddfsClusterMessage)
#define MAX_NUM_OF_MESSAGES	4
#define MAX_SIZE_OF_MESSAGES	(SIZE_OF_MESSAGE * MAX_NUM_OF_MESSAGES) 

void ddfsClusterMessagePaxos::init() {
	ddfsHeader.version = 111;
	ddfsHeader.typeOfService = CLUSTER_MESSAGE_TOF_CLUSTER_UNKNOWN;
	ddfsHeader.totalLength = SIZE_OF_HEADER;
	ddfsHeader.uniqueID = 10;
    ddfsHeader.Reserved1 = 250;
    ddfsHeader.Reserved2 = 189;
	memset(&ddfsMessage, 0, sizeof(ddfsClusterMessage));
}

ddfsClusterMessagePaxos::ddfsClusterMessagePaxos() {
    init();
	message = (void *)malloc(SIZE_OF_HEADER + MAX_SIZE_OF_MESSAGES);
}

ddfsClusterMessagePaxos::~ddfsClusterMessagePaxos() {
    free(message);
}

ddfsStatus ddfsClusterMessagePaxos::addMessage(uint64_t roundNumber, uint16_t messageType,
            uint64_t proposalNumber, uint64_t lastAcceptedProposalNumber, uint64_t lastAcceptedValue) {
	ddfsMessage.messageType = messageType;
    ddfsMessage.Reserved1 = 0;
	/* In case of cluster meta data this is the proposal number */
    ddfsMessage.roundNumber = roundNumber;
	ddfsMessage.proposalNumber = proposalNumber;
	ddfsMessage.lastAcceptedProposalNumber = lastAcceptedProposalNumber;
	ddfsMessage.lastAcceptedValue = lastAcceptedValue;

	ddfsHeader.typeOfService = CLUSTER_MESSAGE_TOF_CLUSTER_MGMT;

	/* Maximum four messages at a time are supported */
	if(ddfsHeader.totalLength == MAX_SIZE_OF_MESSAGES)
		return (ddfsStatus(DDFS_FAILURE));

	memcpy((uint8_t *)message + ddfsHeader.totalLength,
            &ddfsMessage, sizeof(ddfsMessage));

	ddfsHeader.totalLength += SIZE_OF_MESSAGE;

	return (ddfsStatus(DDFS_OK));
}

void ddfsClusterMessagePaxos::returnBuffer(void *outputBuffer) {

	memcpy(message, &ddfsHeader, sizeof(ddfsHeader));
#if 0
	memcpy(message, &ddfsHeader.version, sizeof(ddfsHeader.version));
	memcpy((uint8_t *)message + sizeof(ddfsHeader.version),
		&ddfsHeader.typeOfService, sizeof(ddfsHeader.typeOfService));
	
	memcpy((uint8_t *)message + sizeof(ddfsHeader.version) + sizeof(ddfsHeader.typeOfService),
		&ddfsHeader.totalLength, sizeof(ddfsHeader.totalLength));
#endif
    memcpy(outputBuffer, message, ddfsHeader.totalLength);
}

uint64_t ddfsClusterMessagePaxos::returnBufferSize() {
    return ddfsHeader.totalLength;
}


void ddfsClusterMessagePaxos::clearBuffer() {
    bzero(message, sizeof(SIZE_OF_HEADER + MAX_SIZE_OF_MESSAGES));
    init();
}
