/*
 * @file ddfs_udpConnection.cpp
 *
 * @brief Module for managing UDP network communication.
 *
 * This is the module that would be responsible for UDP network
 * data management.
 *
 * One udpConnection instance is created per remote node.
 * Connection is between localNode <----> remoteNode.
 *
 * No udpConnection instance for localNode.
 *
 * There is one to one relation between udpConnection and MemberPaxos.
 * For the local MemberPaxos there is no udpConnection.
 *
 * TODO:
 * 1. Look into zero copy implementation.
 *
 * Author Harman Patial <harman.patial@gmail.com>
 */

#include "ddfs_udpConnection.h"

#define DDFS_SERVER_PORT	5000

ddfsLogger &global_logger = ddfsLogger::getInstance();

ddfsUdpConnection::ddfsUdpConnection() {
	network_type = DDFS_NETWORK_UDP;
    serverSocketFD = -1;
    clientSocketFD = -1;

    /* Pre allocating memory for storing the header */
    bzero(tempBuffer, 4096);
}

/* openConnection				*/
/**
 * @brief - Open a UDP connection.
 *
 * Open a UDP connection that would be used to
 * communicate with other nodes in the DFS.
 *
 * We would start a server port as well as a client port.
 *
 * @param   nodeUniqueID[in]	Node Host address.
 * @parm    serverSocket[in]    Server socketfd for the local server port.
 *
 * @return DDFS_OK	Success
 * @return DDFS_FAILURE	Failure
 */
ddfsStatus ddfsUdpConnection::openConnection(string nodeUniqueID, int serverSocket) {
	struct sockaddr_in serverAddr;
    int lengthServerAddr;
	struct sockaddr_in clientAddr;
    int lengthClientAddr;

    /* Open the socket for localNode client */
    if ((clientSocketFD = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1) {
        global_logger << ddfsLogger::LOG_WARNING
                    << "networkCommunication :: Unable to open client socket."
                    << strerror(errno) << "\n";
        return (ddfsStatus(DDFS_FAILURE));
    }

    clientAddr.sin_family = AF_INET;
    clientAddr.sin_port = 0; // Randomly select port
    clientAddr.sin_addr.s_addr = inet_addr("localhost");
    bzero(&(clientAddr.sin_zero),8);

    lengthClientAddr = sizeof(clientAddr);
    /* Bind socket with the client */
    if (::bind(clientSocketFD,(struct sockaddr *)&clientAddr,
        sizeof(struct sockaddr)) == -1)
    {
        global_logger << ddfsLogger::LOG_WARNING << "UDP::Client :: Unable to bind socket. "
                        << strerror(errno) <<"\n";
        return (ddfsStatus(DDFS_FAILURE));
    }

    /* Setting the destination socket addr */
    destinationAddr.sin_family = AF_INET;
    destinationAddr.sin_addr.s_addr = inet_addr(remoteNodeHostName.c_str());
    destinationAddr.sin_port = htons(DDFS_SERVER_PORT);
    destinationAddrSize = sizeof(destinationAddr);
    
    if ( serverSocket != -1 ) {
        serverSocketFD = serverSocket;
        global_logger << ddfsLogger::LOG_WARNING << "Server :: Server socket is already open.\n";
        return (ddfsStatus(DDFS_OK));
    }

	/* Open the server socket and wait for the client connections.
     * Server connection at each node opens at well defined port DDFS_SERVER_PORT.
     */
    if ((serverSocketFD = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1) {
        global_logger << ddfsLogger::LOG_WARNING << "Server :: Unable to open socket.\n";
        close(clientSocketFD);
        return (ddfsStatus(DDFS_FAILURE));
    }

    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(DDFS_SERVER_PORT);
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    bzero(&(serverAddr.sin_zero),8);

    lengthServerAddr = sizeof(serverAddr);

    /* Bind socket with the server */
    if (::bind(serverSocketFD,(struct sockaddr *)&serverAddr, sizeof(struct sockaddr)) == -1)
    {
        global_logger << ddfsLogger::LOG_WARNING << "UDP::Server :: Unable to bind socket. "
                        << strerror(errno) <<"\n";
        close(clientSocketFD);
        close(serverSocketFD);
        return (ddfsStatus(DDFS_FAILURE));
    }

    global_logger << ddfsLogger::LOG_INFO << "Successfully created the server socket " <<
                    "to handle incoming messages";

    /* Copy the node IP to this instance */
    remoteNodeHostName = nodeUniqueID;

    /* Start the thread to handle the incoming traffic from
     * the remote node(remoteNodeHostName) in the cluster.
     */
    bkThreads.push_back(std::thread(&ddfsUdpConnection::bk_routine, (void *) this)); 

    global_logger << ddfsLogger::LOG_INFO << "UDP::Both client and server connections opened."
                << strerror(errno) <<"\n";
    
	return (ddfsStatus(DDFS_OK));
}

/*	sendData			*/
/**
 * @brief   Send data across.
 *
 * Send data from the connection that has been
 * previously established.
 * @param   data		Pointer to the data that needs to be send
 * @param   size		Size of the data to be send
 * @param   fn			The callback function. If this is NULL, this is
 * 				synchronous call.
 * 				If this fn is not NULL, this is asynchronous calls.
 *
 * @return  DDFS_OK		Success
 * @return  DDFS_NETWORK_RETRY	Retry after some time
 * @return  DDFS_HOST_DOWN	Host is down
 * @return  DDFS_FAILURE	Failure
 */
ddfsStatus ddfsUdpConnection::sendData(void *data, int size, void *privatePtr) {
    int returnValue = 0;
    requestQEntry *entry = NULL;
    requestQueue *rQueueInstance = (requestQueue *) privatePtr;


    while(rQueueInstance->dataBuffer->empty() == false) {
        rQueueInstance->rLock.lock();
        entry = rQueueInstance->dataBuffer->front();
        rQueueInstance->dataBuffer->pop();
        rQueueInstance->rLock.unlock();
            
        /* Send the data to the other node */
        returnValue = sendto(clientSocketFD, entry->data,
                            entry->totalLength, 0,
                            (struct sockaddr *) &destinationAddr, destinationAddrSize);
        if(returnValue == -1) {
            global_logger << ddfsLogger::LOG_INFO << "UDP::Send : Unable to send data."
                <<  entry->uniqueID
                << strerror(errno) << "\n";
            /* TODO : Push a failure entry in the response queue with this uniqueID */
            return (ddfsStatus(DDFS_FAILURE));
        }
    }

    free(entry);
    
	return (ddfsStatus(DDFS_OK));
}

/*	receiveData			*/
/**
 * @brief   Receive data from the connection.
 *
 * Receive data from the connection that has been
 * previously established.
 *
 * @return  DDFS_OK		Success
 * @return  DDFS_HOST_DOWN	Host is down
 * @return  DDFS_FAILURE		Failure
 */
ddfsStatus ddfsUdpConnection::receiveData(void *des, int requestedSize, int *actualSize) {
    /* No need to implement this as data tranfer is done through
     * request and response queues. */
	return (ddfsStatus(DDFS_FAILURE));
}
/*	checkConnection			*/
/**
 * @brief   Check the connection.
 *
 * Check the connection that has been previously established.
 *
 * @return   DDFS_OK		Success
 * @return   DDFS_HOST_DOWN	Host is down
 * @return   DDFS_FAILURE	Failure
 */
ddfsStatus ddfsUdpConnection::checkConnection() {
    /* Should create a test queue and send a special DDFS packet
     * to the server.
     *
     * Alternative to this is the heartbeat or maybe it could be
     * complementary to it.
     */
	return (ddfsStatus(DDFS_FAILURE));
}
/*	subscribe			*/
/**
 * @brief   Subscribe the connection.
 *
 * Subscribe to the connection.
 * If data is received from this connection, the
 * registered callback function would be invoked.
 *
 * @param fn The callback function to be registered.
 *
 * @see copyData().
 *
 * @return   DDFS_OK		Success
 * @return   DDFS_FAILURE	Failure
 */
ddfsStatus ddfsUdpConnection::subscribe(void (*subscribeFn)(int), void *privatePtr) {
    requestQEntry *entry = NULL;
    responseQueue *rspQInstance = NULL;
    requestQueue *reqQInstance = (requestQueue *) privatePtr;

    if (privatePtr == NULL) {
            global_logger << ddfsLogger::LOG_INFO << "UDP::Send: Null privatePtr passed."
                <<  entry->uniqueID
                << strerror(errno) << "\n";
            return (ddfsStatus(DDFS_FAILURE));
    }

    rspQInstance = reqQInstance->corrResponseQueue;

    rspQInstance->subscriptions.addSubscription(subscribeFn);

	return (ddfsStatus(DDFS_OK));
}

/*	closeConnection			*/
/**
 * @brief   Close the connection.
 *
 * Close the connection.
 * Free all the resources utilized for this connection.
 *
 * @note Any outstanding data would be thrown away.
 *
 * @return   DDFS_OK		Success
 * @return   DDFS_FAILURE	Failure
 */
ddfsStatus ddfsUdpConnection::closeConnection() {

	/* Terminate the background threads. */
    int size, i = 0;

    size = bkThreads.size();

    while(i < size) {
            terminateThreads[i++] = true;
    }

    /* Wait for the threads to terminate */
    while(i < size) {
            bkThreads[i].join();
    }

    /* Remove all the threads from the queue */
    while ( bkThreads.empty() == false )
            bkThreads.pop_back();

    /* Notify all the components that have subscription */
    i=0;
    while((i<128)) {
        if(responseQueues[i] == NULL)
            continue;

        /* Call all the subscription fn for this resposen queue. */
        responseQueues[i]->subscriptions.callSubscription(-1);
    }

    /* Wait for 10 seconds for the subscribed components to
     * perform internal cleanup.
     */
    sleep(10);

    /* Clean all the memory allocated for the req/rsp queues.
     * Don't free the dataBuffer as that is allocated by the
     * upper componenets.
     */
    i =0;
    while((i<128)) {
        if(responseQueues[i] == NULL)
            continue;

        /* Remove all the subscription fn for this resposen queue. */
        responseQueues[i]->subscriptions.removeAllSubscription();
    }

    /* Close the connections */
    close(serverSocketFD);
    serverSocketFD = -1;

    close(clientSocketFD);
    clientSocketFD = -1;

    remoteNodeHostName.replace(remoteNodeHostName.begin(),
                            remoteNodeHostName.end(), 1, '\0');

    destinationAddrSize = -1;

    bzero(tempBuffer, 4096);

	return (ddfsStatus(DDFS_OK));
}

/*	copyData			*/
/**
 * @brief   Copy inbound data in the connection.
 *
 * Copy the data being received by the connection.
 *
 * @note If you in the callback function passed during the
 *       subscription, then use this function to get the data.
 *
 * @note For performance sake, we should share memory and
 * 	 should be doing zero copy. But for now, just copy
 * 	 data over.
 *
 * @param[in]  des		Destination pointer
 * @param[in]  requestedSize	The requested size of data
 * @param[in]  actualSize	Actual size which we were able to copy
 *
 * @return   DDFS_OK			Success
 * @return   DDFS_NETWORK_UNDERRUN	Data from connection is less than asked for
 * @return   DDFS_FAILURE		Failure
 */
ddfsStatus ddfsUdpConnection::copyData(void *des, int requestedSize, int *actualSize) {
	return (ddfsStatus(DDFS_FAILURE));
}

ddfsStatus ddfsUdpConnection::getServerSocket(int *retServerSocket) {
    *retServerSocket = serverSocketFD;
	return (ddfsStatus(DDFS_OK));
}


/*	bk_routine			*/
/**
 * @brief   This is background thread routine.
 *
 * This is the background thread routine responsible for handling the traffic
 * .
 *
 * @note Background routine for processing network events.
 *
 * @param[in]  None.
 *
 * @return   None.
 */
void* ddfsUdpConnection::bk_routine(void *arg) {
    ddfsUdpConnection *udpInstance = (ddfsUdpConnection *) arg;
	struct sockaddr_in serverAddr;
    int serverAddrLen = sizeof(struct sockaddr_in);
    struct hostent *hp;
    int returnLength = 0;
	int ret = 0;

	global_logger << ddfsLogger::LOG_WARNING << "UDP:: Started the background thread. "
						<< strerror(errno) <<"\n";

    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(DDFS_SERVER_PORT);
    bzero(&(serverAddr.sin_zero),8);

    hp = gethostbyname(udpInstance->remoteNodeHostName.c_str());
    if(!hp) {
	    global_logger << ddfsLogger::LOG_WARNING << "UDP:: BT : Fail to get IP address from Host Name. "
						<< strerror(errno) <<"\n";
        return 0;
    }

    memcpy((void *)&serverAddr.sin_addr, hp->h_addr_list[0], hp->h_length);

    /* Keep on listening to the port inorder to recieve data.
     */
	while(1) {
        /* Server port is already opened.
         * Start listening to the port.
         */
        returnLength = 0;
        ret = recvfrom(udpInstance->serverSocketFD, (void *) udpInstance->tempBuffer,
                        sizeof(ddfsClusterHeader), 0,
                        (struct sockaddr *) &serverAddr, (socklen_t *)&serverAddrLen);

        if(ret == -1) {
	        global_logger << ddfsLogger::LOG_WARNING << "UDP:: BT : Fail to get IP address from Host Name. "
						<< strerror(errno) <<"\n";
            continue;
        }
 
        returnLength = ret;

        if(ret < sizeof(ddfsClusterHeader)) {
	        global_logger << ddfsLogger::LOG_WARNING << "UDP:: BT : Data recieved is less that clusterheader from Host Name. "
						<< strerror(errno) <<"\n";
            continue;
        }

        /* Periodically keep on trying to connect to the set of nodes
         * that has been configured.
         */
        {
            global_logger << ddfsLogger::LOG_WARNING
                << "UDP::Client :: Unable to connect socket. "
                << strerror(errno) <<"\n";
                continue;
        }
	} /* while loop end */

	pthread_exit(NULL);

}

ddfsStatus ddfsUdpConnection::setupQueues(std::queue <requestQEntry *> *reqQueueBuffer,
                                        std::queue <responseQEntry *> *rspQueueBuffer, void *privatePtr) {
    int i=0;
    requestQueue *reqQueue = NULL;
    responseQueue *rspQueue = NULL;

    reqQueue = (requestQueue *) malloc(sizeof(requestQueue));
    rspQueue = (responseQueue *) malloc(sizeof(responseQueue));

    reqQueue->dataBuffer = reqQueueBuffer;
    rspQueue->dataBuffer = rspQueueBuffer;

    reqQueue->corrResponseQueue = rspQueue;
    rspQueue->corrRequestQueue = reqQueue;

    queuesLock.lock();
    
    while(i<128) {
        if(requestQueues[i] == NULL)
            break;
        i++;
    }

    if(i == 128) {
        global_logger << ddfsLogger::LOG_WARNING << "UDP:: BT : Max. number of req/rsp queues reached : 128."
                        << "\n";
        free(reqQueue);
        free(rspQueue);
        return ddfsStatus(DDFS_OK);
    }

    requestQueues[i] = reqQueue;
    responseQueues[i] = rspQueue;

    /* This pointer would be passed to us in the sendData */
    privatePtr = reqQueue;

    queuesLock.unlock();

    return ddfsStatus(DDFS_OK);
    
}

