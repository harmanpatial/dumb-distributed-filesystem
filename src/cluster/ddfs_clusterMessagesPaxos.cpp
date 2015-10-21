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

using namespace std;

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

ddfsClusterMessagePaxos::ddfsClusterMessagePaxos() {
	ddfsHeader.version = 0xFF;
	ddfsHeader.typeOfService = CLUSTER_MESSAGE_TOF_CLUSTER_UNKNOWN;
	ddfsHeader.totalLength = 0;
	memset(&ddfsMessage, 0, sizeof(ddfsClusterMessage));
	message = (char *)malloc(SIZE_OF_HEADER + MAX_SIZE_OF_MESSAGES);
}

ddfsClusterMessagePaxos::~ddfsClusterMessagePaxos() {
    free(message);
}

ddfsStatus ddfsClusterMessagePaxos::addMessage(uint16_t messageType, uint64_t proposalNumber, uint64_t lastAcceptedProposalNumber, uint64_t lastAcceptedValue) {
	ddfsMessage.messageType = htonl(messageType);
    ddfsMessage.Reserved1 = htonl(0);
	/* In case of cluster meta data this is the proposal number */
	ddfsMessage.proposalNumber = htonl(proposalNumber);
	ddfsMessage.lastAcceptedProposalNumber = htonl(lastAcceptedProposalNumber);
	ddfsMessage.lastAcceptedValue = htonl(lastAcceptedValue);

	ddfsHeader.typeOfService = CLUSTER_MESSAGE_TOF_CLUSTER_MGMT;

	/* Maximum four messages at a time are supported */
	if(ddfsHeader.totalLength == MAX_SIZE_OF_MESSAGES)
		return (ddfsStatus(DDFS_FAILURE));

	memcpy((uint8_t *)message + SIZE_OF_HEADER + ddfsHeader.totalLength,
            &ddfsMessage, sizeof(ddfsMessage));

	ddfsHeader.totalLength += SIZE_OF_MESSAGE;

	return (ddfsStatus(DDFS_OK));
}

void ddfsClusterMessagePaxos::returnBuffer(void *outputBuffer) {
	memcpy(message, &ddfsHeader.version, sizeof(ddfsHeader.version));
	memcpy((uint8_t *)message + sizeof(ddfsHeader.version),
		&ddfsHeader.typeOfService, sizeof(ddfsHeader.typeOfService));
	
	memcpy((uint8_t *)message + sizeof(ddfsHeader.version) + sizeof(ddfsHeader.typeOfService),
		&ddfsHeader.totalLength, sizeof(ddfsHeader.totalLength));

    memcpy(outputBuffer, (void *) message, ddfsHeader.totalLength);
}

uint64_t ddfsClusterMessagePaxos::returnBufferSize() {
    return ddfsHeader.totalLength;
}

