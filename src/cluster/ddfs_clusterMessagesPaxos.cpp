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

#include <string>
#include <arpa/inet.h>

#include "ddfs_clusterMessagesPaxos.h"
#include "../global/ddfs_status.h"

using namespace std;

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

#define SIZE_OF_HEADER		12
#define SIZE_OF_MESSAGE		8
#define MAX_NUM_OF_MESSAGES	4
#define MAX_SIZE_OF_MESSAGES	(SIZE_OF_MESSAGE * MAX_NUM_OF_MESSAGES) 

ddfsClusterMessagePaxos::ddfsClusterMessagePaxos() {
	header.version = 0;
	header.typeOfService = CLUSTER_MESSAGE_TOF_CLUSTER_UNKNOWN;
	header.totalLength = 0;
	message = (char *)malloc(SIZE_OF_HEADER + MAX_SIZE_OF_MESSAGES);
}

ddfsStatus ddfsClusterMessagePaxos::addMessage(uint32_t type, uint32_t uuid) {
	uint32_t wtype = htonl(type);
	uint64_t wuuid = htonl(uuid);

	if(header.typeOfService == CLUSTER_MESSAGE_TOF_CLUSTER_UNKNOWN)
		return (ddfsStatus(DDFS_FAILURE));

	header.typeOfService = CLUSTER_MESSAGE_TOF_CLUSTER_MGMT;

	/* Maximum four messages at a time are supported */
	if(header.totalLength == MAX_SIZE_OF_MESSAGES)
		return (ddfsStatus(DDFS_FAILURE));


	memcpy((uint8_t *)message + SIZE_OF_HEADER + header.totalLength, &wtype, sizeof(wtype));
	memcpy((uint8_t *)message + SIZE_OF_HEADER + header.totalLength + sizeof(wtype), &wuuid, sizeof(wuuid));

	header.totalLength += SIZE_OF_MESSAGE;

	return (ddfsStatus(DDFS_OK));
}

void * ddfsClusterMessagePaxos::returnBuffer() {
	memcpy(message, &header.version, sizeof(header.version));
	memcpy((uint8_t *)message + sizeof(header.version),
		&header.typeOfService, sizeof(header.typeOfService));
	
	memcpy((uint8_t *)message + sizeof(header.version) + sizeof(header.typeOfService),
		&header.totalLength, sizeof(header.totalLength));

	return (void *)message;
}
