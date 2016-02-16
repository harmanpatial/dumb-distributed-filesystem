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
#include <iostream>

#include "ddfs_clusterMemberPaxos.hpp"
#include "ddfs_clusterPaxos.hpp"
#include "ddfs_clusterPaxosInstance.hpp"

using namespace std;

ddfsLogger &global_logger_cmp = ddfsLogger::getInstance();

ddfsClusterMemberPaxos::ddfsClusterMemberPaxos() {
    clusterID = s_invalid_clusterID;
    memberID = s_invalid_memberID;
    uniqueIdentification = -1;
    memberState.store(s_clusterMemberUnknown);
    networkPrivatePtr = NULL;
    _isLocalNode = false;

    network = new ddfsTcpConnection<ddfsClusterMemberPaxos>();
    network->init();
}

ddfsClusterMemberPaxos::~ddfsClusterMemberPaxos() {
    delete(network);
}

ddfsStatus ddfsClusterMemberPaxos::init(string hostn, ddfsClusterMemberPaxos *localNode) {
    ddfsStatus status(DDFS_OK);

    hostName = hostn;

    if(!localNode)
        _isLocalNode = true;

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

    /*  Only connect to the remote IP server port if following condition is met.
     *
     *  localIP < remoteIP
     *
     *  else it is the responsibility of the remote IP to connect to my
     *  DDFS_SERVER_PORT.
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
    if(!localNode)
        status = network->openConnection("localhost", doNotConnect);
    else
        status = network->openConnection(hostName, doNotConnect);

    if(status.compareStatus(ddfsStatus(DDFS_OK)) == false) {
        global_logger_cmp << ddfsLogger::LOG_WARNING
                    << "clusterMemberPaxos :: Unable to Open connection.\n";
        return status;
    }

    /* Allocate request and response queues */
    status = network->setupPortal(&networkPrivatePtr); 

    if(status.compareStatus(ddfsStatus(DDFS_OK)) == false) {
        global_logger_cmp << ddfsLogger::LOG_WARNING
                    << "clusterMemberPaxos :: Unable to setup Queues.\n";
        return status;
    }

    /* Start the response queue processing thread */
    //workingThreadQ.push_back(std::thread(&ddfsClusterMemberPaxos::processingResponses, this));

    /* Subscribe to the response queue */
    if(localNode)
        status = network->subscribe(this, networkPrivatePtr);

    if(status.compareStatus(ddfsStatus(DDFS_OK)) == false) {
        global_logger_cmp << ddfsLogger::LOG_WARNING
                    << "clusterMemberPaxos :: Unable to subscribe to Queues.\n";
        return status;
    }

#if 0

    if(localNode) {
        while(1) {
            sleep(2);
            status = network->checkConnection();

            if(status.compareStatus(ddfsStatus(DDFS_OK)) == false) {
                global_logger_cmp << ddfsLogger::LOG_INFO
                        << "Check Connection failed for host : " << hostn << "\n";
            } else {
                global_logger_cmp << ddfsLogger::LOG_INFO
                        << "Connection is now established with : " << hostn << "\n";

                /* TODO : Test code */
                if(getHostName().compare("localhost")) {
                    global_logger_cmp << ddfsLogger::LOG_INFO
                        << "Sending a test packet to " << getHostName() << "\n";

                    std::string s("DDFS Destination Node : ");
                    s.append(getHostName());
                    s.append(" Request ");
                    network->sendData((void *)s.c_str(), s.size(), networkPrivatePtr);
                }
                break;
            }
        }
    }
#endif

    struct in_addr addr;

    inet_aton(hostn.c_str(), &addr) ;
    addr.s_addr = addr.s_addr >> 16;

    /* Set if this is the localNode */
    setUniqueIdentification(addr.s_addr);
    setMemberID(addr.s_addr);

    return (ddfsStatus(DDFS_OK));
}

bool ddfsClusterMemberPaxos::isOnline() {
    bool online = false;
    ddfsStatus status(DDFS_FAILURE);

    status = network->checkConnection();

    if(status.compareStatus(ddfsStatus(DDFS_OK)) == true) {
        setCurrentState(s_clusterMemberOnline);
        return true;
    }

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
    global_logger_cmp << ddfsLogger::LOG_WARNING
                << "Setting Member ID : " << newMemberID << "\n";
	clusterMemberLock.lock();
	memberID = newMemberID;
	clusterMemberLock.unlock();
}

int ddfsClusterMemberPaxos::getMemberID() {
	return memberID;
}

void ddfsClusterMemberPaxos::setUniqueIdentification(int newUniqueID) {

	if(isLocalNode() == false)
        	return;
	clusterMemberLock.lock();
	uniqueIdentification = newUniqueID;
	clusterMemberLock.unlock();
    global_logger_cmp << ddfsLogger::LOG_WARNING
                    << "ddfsClusterMemberPaxos:: Unique id set to " << uniqueIdentification << "\n";
}

int ddfsClusterMemberPaxos::getUniqueIdentification() {
	if(isLocalNode() == false)
        	return 0;
    global_logger_cmp << ddfsLogger::LOG_WARNING
                    << "ddfsClusterMemberPaxos:: Returning Unique id " << uniqueIdentification << "\n";
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
    uint8_t numberOfDdfsMessages;
    ddfsClusterHeader *ddfsHeader;
    //uint16_t typeOfService, totalLength;
    ddfsStatus status(DDFS_FAILURE);

    global_logger_cmp << ddfsLogger::LOG_WARNING
            << "ddfsClusterMemberPaxos::callback: Enter.\n";

    /* This is a special case for when the connection is being about to
     * close.
     */
    cout << "Data is : " << data << " size is " << size << "\n";

    if((data == NULL) || (size <= 0)) {
        return;
    }

    ddfsHeader = (ddfsClusterHeader *) data;

    global_logger_cmp << ddfsLogger::LOG_INFO << "CMP: v: " << ddfsHeader->version << ". tOS: " << ddfsHeader->typeOfService << "\n";
    global_logger_cmp << ddfsLogger::LOG_INFO << "CMP: tl:" << ddfsHeader->totalLength << ".ID: " << ddfsHeader->uniqueID << "\n";
    global_logger_cmp << ddfsLogger::LOG_INFO << "CMP: sizeof(ddfsClusterHeader) : " << sizeof(ddfsClusterHeader) << "\n";
    global_logger_cmp << ddfsLogger::LOG_INFO << "CMP: sizeof(ddfsClusterMessage) : " << sizeof(ddfsClusterMessage) << "\n";

#if 0
    global_logger_cmp << ddfsLogger::LOG_INFO
                << "This is the message received from " << getHostName << " : "
                << entry->data[0] << entry->data[1] << entry->data[2] << entry->data[3]
                << entry->data[4] << entry->data[5] << entry->data[6] << entry->data[7]
                << entry->data[8] << entry->data[9] << entry->data[10] << entry->data[11]
                << entry->data[12] << entry->data[13] << entry->data[14] << entry->data[15] << "\n";
#endif
    if(ddfsHeader->typeOfService == CLUSTER_MESSAGE_TOF_CLUSTER_MGMT) {
                numberOfDdfsMessages = ((ddfsHeader->totalLength)-sizeof(ddfsClusterHeader))/sizeof(ddfsClusterMessage);

        global_logger_cmp << ddfsLogger::LOG_WARNING << "Total DDFS Messages in this packet is : " << numberOfDdfsMessages << "\n";
        for (int i = 0; i < numberOfDdfsMessages; i++) {
            ddfsClusterMessage *message = (ddfsClusterMessage *)((uint8_t *) data + sizeof(ddfsClusterHeader) + (i*sizeof(ddfsClusterMessage)));

            global_logger_cmp << ddfsLogger::LOG_INFO << "CMP: Message Type : " << message->messageType << "\n";
            if((message->messageType >= CLUSTER_MESSAGE_LE_TYPE_PREPARE) && (message->messageType <= CLUSTER_MESSAGE_LE_LEADER_ELECTED)) {
                    status = clusterPaxos->processMessage(this, message);
            } else { 
                status = processMessage(message);
            }

            if(status.compareStatus(ddfsStatus(DDFS_OK)) == false) {
                global_logger_cmp << ddfsLogger::LOG_WARNING
                            << "CMP:: Discarding a packet.\n";
                continue;
            }
        }
    }
}

ddfsStatus ddfsClusterMemberPaxos::sendClusterMetaData(ddfsClusterMessagePaxos *message) {
    /* Need to have a reference of network packet and send the buffer to it */
    //requestQEntry *request = NULL;
    ddfsClusterHeader *packetHeader = NULL;

    if(message == NULL) {
        global_logger_cmp << ddfsLogger::LOG_WARNING
                    << "CMP:: Unable to open client socket."
                    << strerror(errno) << "\n";
        return (ddfsStatus(DDFS_FAILURE));
    }

    global_logger_cmp << ddfsLogger::LOG_INFO
            << "CMP:: sendClusterMetaData: sending data.\n";
    
    //request = new requestQEntry;

    uint8_t packet[2048];
    message->returnBuffer(&packet);

    packetHeader = (ddfsClusterHeader *) packet;

    global_logger_cmp << ddfsLogger::LOG_INFO
            << "sendClusterMetaData :: packetDetail : " << packetHeader->version << " " << packetHeader->typeOfService << " " << packetHeader->totalLength << "\n";
    
#if 0
    /* Create the Request queue entry for this packet */
    request->typeOfService = packetHeader->typeOfService;
    request->totalLength = packetHeader->totalLength;

    global_logger_cmp << ddfsLogger::LOG_INFO
            << "ddfsClusterMemberPaxos :: sendClusterMetaData: size of " << request->totalLength << " bytes." << "\n";
    
	if(request->totalLength > MAX_REQUEST_SIZE) {
		free(request);
		global_logger_cmp << ddfsLogger::LOG_WARNING
				<< "Size of message is greater than MAX_REQUEST_SIZE : " << request->totalLength << "\n";
		return (ddfsStatus(DDFS_FAILURE));
	}
	memcpy(request->data, packet, request->totalLength);
    request->uniqueID = packetHeader->uniqueID;
    request->privateData = NULL;
    
    packetHeader = (ddfsClusterHeader *) request->data;
#endif

    global_logger_cmp << ddfsLogger::LOG_INFO
            << "sendClusterMetaData :: packetDetail for Request : " << packetHeader->version << " " << packetHeader->typeOfService << " " << packetHeader->totalLength << "\n";
    
    //reqQueue.push(request);
    /* Push the data to the request queue */
    network->sendData(packet, packetHeader->totalLength, networkPrivatePtr);
	return (ddfsStatus(DDFS_OK));
}


string ddfsClusterMemberPaxos::getHostName() {
    return hostName;
}
