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

#include "ddfs_clusterMemberPaxos.hpp"

ddfsLogger &global_logger_cmp = ddfsLogger::getInstance();

ddfsClusterMemberPaxos::ddfsClusterMemberPaxos() {
    clusterID = s_invalid_clusterID;
    memberID = s_invalid_memberID;
    uniqueIdentification = -1;
    memberState.store(s_clusterMemberUnknown);
    networkPrivatePtr = NULL;

    network = new ddfsTcpConnection<ddfsClusterMemberPaxos>();
    network->init();
}

ddfsClusterMemberPaxos::~ddfsClusterMemberPaxos() {
    delete(network);
}

ddfsStatus ddfsClusterMemberPaxos::init(string hostn, ddfsClusterMemberPaxos *localNode) {
    ddfsStatus status(DDFS_OK);

    hostName = hostn;

    if(network) {
        if(!localNode) {
            global_logger_cmp << ddfsLogger::LOG_INFO
                        << "clusterMemberPaxos :: Localhost Initialization started.\n";
        } else {
            global_logger_cmp << ddfsLogger::LOG_INFO
                        << "clusterMemberPaxos :: Initialization already done.\n";
            return (ddfsStatus(DDFS_OK));
        }
    } else {
        network = new ddfsTcpConnection<ddfsClusterMemberPaxos>();
    }

    network->init();

    global_logger_cmp << ddfsLogger::LOG_INFO
                        << "clusterMemberPaxos :: Opening the network connection.\n";

    /*   TODO : Only connect to the remote IP server port if localIP < remoteIP,
     *          else it is the responsibility of the remote IP to connect to my
     *          DDFS_SERVER_PORT.
     */

    bool doNotConnect = false;
    
    if(localNode != NULL) {
        string localHostName = localNode->getHostName();

        global_logger_cmp << ddfsLogger::LOG_INFO
                        << "localHost name : " << localHostName << ". remoteHostName : " << hostn << "\n";

        for(unsigned int i=0; i < localHostName.size() && i < hostn.size(); i++) {
            if(localHostName[i] > hostn[i]) {
                doNotConnect = true;
            }
        }
    }

    /*  Initialize the underline network class */
    if(doNotConnect == false) {
        if(!localNode)
            status = network->openConnection("localhost");
        else
            status = network->openConnection(hostName);
    }

    if(status.compareStatus(ddfsStatus(DDFS_OK)) == false) {
        global_logger_cmp << ddfsLogger::LOG_WARNING
                    << "clusterMemberPaxos :: Unable to Open connection.\n";
        return status;
    }

    /* Allocate request and response queues */
    //status = network->setupQueues(&reqQueue, &rspQueue, networkPrivatePtr); 
    status = network->setupPortal(&networkPrivatePtr); 

    if(status.compareStatus(ddfsStatus(DDFS_OK)) == false) {
        global_logger_cmp << ddfsLogger::LOG_WARNING
                    << "clusterMemberPaxos :: Unable to setup Queues.\n";
        return status;
    }

    /* Start the response queue processing thread */
    //workingThreadQ.push_back(std::thread(&ddfsClusterMemberPaxos::processingResponses, this));

    /* Subscribe to the response queue */
    status = network->subscribe(this, networkPrivatePtr);

    if(status.compareStatus(ddfsStatus(DDFS_OK)) == false) {
        global_logger_cmp << ddfsLogger::LOG_WARNING
                    << "clusterMemberPaxos :: Unable to subscribe to Queues.\n";
        return status;
    }

    if(getHostName().compare("localhost") && 0) {
        global_logger_cmp << ddfsLogger::LOG_INFO
                << "Sending a test packet to " << getHostName() << "\n";

        std::string s("DDFS Destination Node : ");
        s.append(getHostName());
        network->sendData((void *)s.c_str(), s.size(), networkPrivatePtr);
    }

    /* Incase this instance is of localNode, 
     * now it's ready to exchange DDFS packages
     */
    return (ddfsStatus(DDFS_OK));
}

bool ddfsClusterMemberPaxos::isOnline() {
    bool online = false;

	clusterMemberLock.lock();
    if(memberState.load() == s_clusterMemberOnline)
        online = true;

	clusterMemberLock.unlock();

	return online;
}

bool ddfsClusterMemberPaxos::isDead() {
	bool dead = false;

	clusterMemberLock.lock();
    if(memberState.load() == s_clusterMemberDead)
        dead = true;

	clusterMemberLock.unlock();

	return dead;
}

clusterMemberState ddfsClusterMemberPaxos::getCurrentState() {
	return memberState;	
}

ddfsStatus ddfsClusterMemberPaxos::setCurrentState(clusterMemberState newState) {
	memberState.store(newState);
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
                    << "ddfsClusterMemberPaxos:: Unable to get the server socket FD.\n";
    }

    return serverSocket;
}
#endif

ddfsStatus ddfsClusterMemberPaxos::processMessage(ddfsClusterMessage *message) {

#if 0
	if(isLocalNode() == false) {
		global_logger_cmp << ddfsLogger::LOG_ERROR
			<< "ddfsClusterMemberPaxos :: Not a local Node.\n";
		return (ddfsStatus(DDFS_FAILURE));
	}
#endif

#if 0
	if(message->messageType == CLUSTER_MESSAGE_ADDING_MEMBER) {
		if(isLocalNode()) {
			clusterPaxos->removeMember();
#endif

	//return clusterPaxos->processMessage(this, message);
	return (ddfsStatus(DDFS_FAILURE));

}

void ddfsClusterMemberPaxos::callback(void *data, int size) {
    //ddfsClusterMemberPaxos *member = (ddfsClusterMemberPaxos *) thisInstance;
    responseQEntry *entry = NULL;
    uint8_t numberOfDdfsMessages;
    //uint16_t typeOfService, totalLength;
    uint8_t *data_p = (uint8_t *) data;
    ddfsStatus status(DDFS_FAILURE);

    /* This is a special case for when the connection is being about to
     * close.
     */
    if((data == NULL) || (size <= 0)) {
        return;
    }

#if 0
    if(numberOfEntries != 0) {
        std::unique_lock<std::mutex> lock(responseQLock);
        needToProcess.notify_all();
    }/* responseQLock automatically unlocked */


void ddfsClusterMemberPaxos::processingResponses() {

    /* Go through the response queue */
    while(1) {
        std::unique_lock<std::mutex> lk(responseQLock);
        needToProcess.wait(lk,
                        [] { return true; });

        while (rspQueue.empty() == false ) {
#endif
            entry = (responseQEntry *) data;

            /* If this is a packet for the paxos iteration, network class should
             * handle this internally.
             */
            if(entry->typeOfService == CLUSTER_MESSAGE_TOF_CLUSTER_MGMT) {
                numberOfDdfsMessages = ((entry->totalLength)-sizeof(ddfsClusterHeader))/sizeof(ddfsClusterMessage);

                for (int i = 0; i < numberOfDdfsMessages; i++) {
                    status = processMessage((ddfsClusterMessage *)(data_p + sizeof(ddfsClusterHeader) + (i*sizeof(ddfsClusterMessage))));
                    if(status.compareStatus(ddfsStatus(DDFS_OK)) == false) {
                        global_logger_cmp << ddfsLogger::LOG_WARNING
                                    << "networkCommunication :: Discarding a packet.\n";
                        continue;
                    }
                }
            }
     //   }
//    }
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
	if(request->totalLength > MAX_REQUEST_SIZE) {
		free(request);
		global_logger_cmp << ddfsLogger::LOG_WARNING
				<< "Size of message is greater than MAX_REQUEST_SIZE : " << packetHeader->totalLength << "\n";
		return (ddfsStatus(DDFS_FAILURE));
	}
	memcpy(request->data, message->returnBuffer(), request->totalLength);
    request->uniqueID = packetHeader->uniqueID;
    request->privateData = NULL;
    
    //reqQueue.push(request);
    /* Push the data to the request queue */
    network->sendData(message->returnBuffer(), message->returnBufferSize(),
                    networkPrivatePtr);
	return (ddfsStatus(DDFS_OK));
}


string ddfsClusterMemberPaxos::getHostName() {
    return hostName;
}
