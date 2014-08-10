/*
 * @file ddfs_clusterMemberPaxos.cpp
 *
 * @brief Module containing the cluster member class.
 *
 * This is the module that contains cluster member class.
 *
 * @author Harman Patial <harman.patial@gmail.com>
 *
 */

#include <fstream>
#include <string>

#include "ddfs_clusterMemberPaxos.h"

ddfsLogger &global_logger = ddfsLogger::getInstance();

ddfsClusterMemberPaxos::ddfsClusterMemberPaxos() {
	clusterID = -1;
	memberID = -1;
	uniqueIdentification = -1;
	memberState = s_clusterMemberUnknown;
    networkPrivatePtr = NULL;

	return;
}

ddfsStatus ddfsClusterMemberPaxos::init(bool isLocalNode, int serverSocketFD) {
    if (isLocalNode == false) {
        /* Get the server socket for the local Cluster Member Node */
    }

    /* Initialize the underline network class */
    network.openConnection(hostName, -1);
    /* Allocate request and response queues */
    network.setupQueues(&requestQueue, &responseQueue, networkPrivatePtr); 

    /* Incase this instance is of localNode, 
     * now it's ready to exchange DDFS packages
     */
    return (ddfsStatus(DDFS_OK));
}

ddfsStatus ddfsClusterMemberPaxos::isOnline() {
    bool stateOnline = false;

	clusterMemberLock.lock();
    if(memberState == s_clusterMemberOnline)
        stateOnline = true;

	clusterMemberLock.unlock();

    if(stateOnline == true)
            return (ddfsStatus(DDFS_OK));

	return (ddfsStatus(DDFS_FAILURE));
}

ddfsStatus ddfsClusterMemberPaxos::isDead() {
    bool stateOnline = false;

	clusterMemberLock.lock();
    if(memberState == s_clusterMemberDead)
        stateOnline = true;

	clusterMemberLock.unlock();

    if(stateOnline == true)
            return (ddfsStatus(DDFS_OK));

	return (ddfsStatus(DDFS_FAILURE));
}

clusterMemberState ddfsClusterMemberPaxos::getCurrentState() {
	return memberState;	
}

ddfsStatus ddfsClusterMemberPaxos::setCurrentState(clusterMemberState newState) {
	clusterMemberLock.lock();
	memberState = newState;
	clusterMemberLock.unlock();
	return (ddfsStatus(DDFS_OK));
}

void ddfsClusterMemberPaxos::setMemberID(int newMemberID) {
	clusterMemberLock.lock();
	memberID = newMemberID;
	clusterMemberLock.unlock();
}

int ddfsClusterMemberPaxos::getMemberID() {
	return memberID;
}

void ddfsClusterMemberPaxos::setUniqueIdentification(int newIdentifier) {
	clusterMemberLock.lock();
	uniqueIdentification =  newIdentifier;
	clusterMemberLock.unlock();
}

int ddfsClusterMemberPaxos::getUniqueIdentification() {
	return uniqueIdentification;
}

void ddfsClusterMemberPaxos::registerHandler(int value) {

    if(value == -1) {
        /* Network class is closing the connection.
         *  */
    }


}

int ddfsClusterMemberPaxos::getLocalSocket() {
    int serverSocket;
    network.getServerSocket(&serverSocket);
    return serverSocket;
}

ddfsStatus ddfsClusterMemberPaxos::sendClusterMetaData(ddfsClusterMessagePaxos *message) {
    /* Need to have a reference of network packet and send the buffer to it */
    requestQEntry *request = NULL;
    ddfsClusterHeader *packetHeader = NULL;

    if(message == NULL) {
        global_logger << ddfsLogger::LOG_WARNING
                    << "clusterMember :: Unable to open client socket."
                    << strerror(errno) << "\n";
        return (ddfsStatus(DDFS_FAILURE));
    }

    request = (requestQEntry *) malloc(sizeof(requestQEntry));

    packetHeader = (ddfsClusterHeader *) message;
    /*************START OF DAY *************************/
    request->typeOfService = packetHeader->typeOfService;
    request->totalLength = packetHeader->totalLength;
    request->data = message;
    request->uniqueID = packetHeader->uniqueID;
    request->privateData = NULL;
    
    requestQueue.push(request);
    /* TODO: Register a callback function */
    /* Push the data to the request queue */
    network.sendData(message->returnBuffer(), message->returnBufferSize(),
                    networkPrivatePtr);
	return (ddfsStatus(DDFS_OK));
}

