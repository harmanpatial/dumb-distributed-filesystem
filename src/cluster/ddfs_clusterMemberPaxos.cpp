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

ddfsLogger &global_logger_cmp = ddfsLogger::getInstance();

ddfsClusterMemberPaxos::ddfsClusterMemberPaxos() {
	clusterID = -1;
	memberID = -1;
	uniqueIdentification = -1;
	memberState = s_clusterMemberUnknown;
    networkPrivatePtr = NULL;

    network = new ddfsUdpConnection<ddfsClusterMemberPaxos>();
}

ddfsClusterMemberPaxos::~ddfsClusterMemberPaxos() {
    delete(network);
}

ddfsStatus ddfsClusterMemberPaxos::init(string hostn) {
    ddfsStatus status(DDFS_FAILURE);

    hostName = hostn;

    /* Initialize the underline network class */
    status = network->openConnection(hostName);

    if(status.compareStatus(ddfsStatus(DDFS_OK)) == false) {
        global_logger_cmp << ddfsLogger::LOG_WARNING
                    << "clusterMemberPaxos :: Unable to Open connection.\n";
        return status;
    }

    /* Allocate request and response queues */
    status = network->setupQueues(&reqQueue, &rspQueue, networkPrivatePtr); 

    if(status.compareStatus(ddfsStatus(DDFS_OK)) == false) {
        global_logger_cmp << ddfsLogger::LOG_WARNING
                    << "clusterMemberPaxos :: Unable to setup Queues.\n";
        return status;
    }

    /* Start the response queue processing thread */
    workingThreadQ.push_back(std::thread(&ddfsClusterMemberPaxos::processingResponses, this));

    /* Subscribe to the response queue */
    status = network->subscribe(this, networkPrivatePtr);

    if(status.compareStatus(ddfsStatus(DDFS_OK)) == false) {
        global_logger_cmp << ddfsLogger::LOG_WARNING
                    << "clusterMemberPaxos :: Unable to subscribe to Queues.\n";
        return status;
    }

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

int ddfsClusterMemberPaxos::getClusterID() {
    return clusterID;
}

ddfsStatus ddfsClusterMemberPaxos::setClusterID(int newClusterID) {
    clusterID = newClusterID;
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
	uniqueIdentification = newIdentifier;
	clusterMemberLock.unlock();
}

int ddfsClusterMemberPaxos::getUniqueIdentification() {
    if(isLocalNode() == false)
        return 0;
	return uniqueIdentification;
}

#if 0
int ddfsClusterMemberPaxos::getLocalSocket() {
    int serverSocket = -1;
    ddfsStatus status(DDFS_FAILURE);

    status = network->getServerSocket(&serverSocket);
    if(status.compareStatus(ddfsStatus(DDFS_OK)) == false) {
        global_logger_cmp << ddfsLogger::LOG_WARNING
                    << "clusterMember:: Unable to get the server socket FD.\n";
    }

    return serverSocket;
}
#endif

void ddfsClusterMemberPaxos::callback(int numberOfEntries) {
    /* This is a special case for when the connection is being about to
     * close.
     */
    if(numberOfEntries == -1) {

    }

    if(numberOfEntries != 0) {
        std::unique_lock<std::mutex> lock(responseQLock);
        needToProcess.notify_all();
    }/* responseQLock automatically unlocked */
}

ddfsStatus ddfsClusterMemberPaxos::processMessage(ddfsClusterMessage *) {

	return (ddfsStatus(DDFS_OK));
}


void ddfsClusterMemberPaxos::processingResponses() {
    //ddfsClusterMemberPaxos *member = (ddfsClusterMemberPaxos *) thisInstance;
    responseQEntry *entry = NULL;
    uint8_t numberOfDdfsMessages;
    //uint16_t typeOfService, totalLength;
    uint8_t *data;
    int i=0;
    ddfsStatus status(DDFS_FAILURE);

    /* Go through the response queue */
    while(1) {
        std::unique_lock<std::mutex> lk(responseQLock);
        needToProcess.wait(lk,
                        [] { return true; });

        while ( rspQueue.empty() == false ) {
            entry = rspQueue.front();
            rspQueue.pop();
            data = (uint8_t *) entry->data;

            /* If this is a packet for the paxos iteration, network class should
             * handle this internally.
             */
            if(entry->typeOfService == CLUSTER_MESSAGE_TOF_CLUSTER_MGMT) {
                numberOfDdfsMessages = ((entry->totalLength)-sizeof(ddfsClusterHeader))/sizeof(ddfsClusterMessage);

                for ( i = 0; i < numberOfDdfsMessages; i++ ) {
                    status = processMessage((ddfsClusterMessage *)(data + sizeof(ddfsClusterHeader) + (i*sizeof(ddfsClusterMessage))));
                    if(status.compareStatus(ddfsStatus(DDFS_OK)) == false) {
                        global_logger_cmp << ddfsLogger::LOG_WARNING
                                    << "networkCommunication :: Discarding a packet.\n";
                        continue;
                    }
                }
            }
        }
    }
}

ddfsStatus ddfsClusterMemberPaxos::sendClusterMetaData(ddfsClusterMessagePaxos *message) {
    /* Need to have a reference of network packet and send the buffer to it */
    requestQEntry *request = NULL;
    ddfsClusterHeader *packetHeader = NULL;

    if(message == NULL) {
        global_logger_cmp << ddfsLogger::LOG_WARNING
                    << "clusterMember :: Unable to open client socket."
                    << strerror(errno) << "\n";
        return (ddfsStatus(DDFS_FAILURE));
    }

    request = (requestQEntry *) malloc(sizeof(requestQEntry));

    packetHeader = (ddfsClusterHeader *) message;

    /* Create the Request queue entry for this packet */
    request->typeOfService = packetHeader->typeOfService;
    request->totalLength = packetHeader->totalLength;
    request->data = message;
    request->uniqueID = packetHeader->uniqueID;
    request->privateData = NULL;
    
    reqQueue.push(request);
    /* Push the data to the request queue */
    network->sendData(message->returnBuffer(), message->returnBufferSize(),
                    networkPrivatePtr);
	return (ddfsStatus(DDFS_OK));
}


string ddfsClusterMemberPaxos::getHostName() {
    return hostName;
}
