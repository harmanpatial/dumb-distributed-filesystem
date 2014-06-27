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
#define SIZE_OF_MESSAGE		4
#define MAX_NUM_OF_MESSAGES	4
#define MAX_SIZE_OF_MESSAGES	(SIZE_OF_MESSAGE * MAX_NUM_OF_MESSAGES) 

ddfsClusterMessagePaxos::ddfsClusterMessagePaxos() {
	version = 0;
	typeOfService = CLUSTER_MESSAGE_TOF_CLUSTER_UNKNOWN;
	totalLength = 0;
	message = (char *)malloc(SIZE_OF_HEADER + MAX_SIZE_OF_MESSAGES);
}

ddfsStatus ddfsClusterMessagePaxos::addMessage(uint32_t type, uint32_t uuid) {
	uint32_t wtype = htonl(type);
	uint32_t wuuid = htonl(uuid);

	if(typeOfService == CLUSTER_MESSAGE_TOF_CLUSTER_UNKNOWN)
		return (ddfsStatus(DDFS_FAILURE));

	typeOfService = CLUSTER_MESSAGE_TOF_CLUSTER_MGMT;

	/* Maximum four messages at a time are supported */
	if(totalLength == MAX_SIZE_OF_MESSAGES)
		return (ddfsStatus(DDFS_FAILURE));


	memcpy(message + SIZE_OF_HEADER + totalLength, &wtype, sizeof(wtype));
	memcpy(message + SIZE_OF_HEADER + totalLength + 2, &wuuid, sizeof(wuuid));

	totalLength += 4;

	return (ddfsStatus(DDFS_OK));
}


void * ddfsClusterMessagePaxos::returnBuffer() {
	memcpy(message, &version, sizeof(version));
	memcpy(message + sizeof(version),
		&typeOfService, sizeof(typeOfService));
	
	memcpy(message + sizeof(version) + sizeof(typeOfService),
		&totalLength, sizeof(totalLength));

	return (void *)message;
}
