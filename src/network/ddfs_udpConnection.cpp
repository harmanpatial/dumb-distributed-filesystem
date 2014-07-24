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
#include "../logger/ddfs_fileLogger.h"
#include "../global/ddfs_status.h"

#define DDFS_SERVER_PORT	5000

ddfsLogger &global_logger = ddfsLogger::getInstance();

ddfsUdpConnection::ddfsUdpConnection() {
    int i = 0;

	network_type = DDFS_NETWORK_UDP;
    serverSocketFD = -1;
    clientSocketFD = -1;
    bzero(tempBuffer, 512);

    /* Pre allocating memory for storing the header */
    for ( i =0; i < 512; i++ ) {
        tempBuffer[i] = malloc(DDFS_HEADE_SIZE + sizeof(void *));
    }
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
 *
 * @return DDFS_OK	Success
 * @return DDFS_FAILURE	Failure
 */
ddfsStatus ddfsUdpConnection::openConnection(string nodeUniqueID) {
	struct sockaddr_in serverAddr;

	/* Open the server socket and wait for the client connections.
     * Server connection at each node opens at well defined port DDFS_SERVER_PORT.
     */
    /* Open the server port to except incoming connections */
    if ((serverSocketFD = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1) {
        global_logger << ddfsLogger::LOG_WARNING << "Server :: Unable to open socket.\n";
        return (ddfsStatus(DDFS_FAILURE));
    }

    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(DDFS_SERVER_PORT);
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    bzero(&(serverAddr.sin_zero),8);

    /* Bind socket with the server */
    if (bind(serverSocketFD,(struct sockaddr *)&serverAddr,
        sizeof(serverAddr)) == -1)
    {
        global_logger << ddfsLogger::LOG_WARNING << "UDP::Server :: Unable to bind socket. "
                        << strerror(errno) <<"\n";
        return (ddfsStatus(DDFS_FAILURE));
    }

    /* Copy the node IP to this instance */
    remoteNodeHostName = nodeUniqueID;

    /* Start the thread to handle the incoming traffic from remote node
     * in the cluster.
     */
    if(!pthread_create(&bk_thread, NULL, &ddfsUdpConnection::bk_routine, (void *)this)) {
        global_logger << ddfsLogger::LOG_WARNING
            << "UDP ::Successfully created the background thread."
            << strerror(errno) <<"\n";
        return (ddfsStatus(DDFS_FAILURE));
    }

    /* Open the socket for localNode client */


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
ddfsStatus ddfsUdpConnection::sendData(void *data, int size, void (*fn)(int)) {
	return (ddfsStatus(DDFS_FAILURE));
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
	return (ddfsStatus(DDFS_FAILURE));
}
/*	checkConnection			*/
/**
 * @brief   Check the connection.
 *
 * Check the connection that has been
 * previously established.
 *
 * @return   DDFS_OK		Success
 * @return   DDFS_HOST_DOWN	Host is down
 * @return   DDFS_FAILURE	Failure
 */
ddfsStatus ddfsUdpConnection::checkConnection() {
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
ddfsStatus ddfsUdpConnection::subscribe(void (*)(int)) {
	return (ddfsStatus(DDFS_FAILURE));
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

	/* 1. Kill the background process.
	 * 2. Disconnect from all the servers.
	 */
    int return_value;
    void *value_ptr = &return_value;
    pthread_join(bk_thread, &value_ptr);
    return (ddfsStatus(DDFS_FAILURE));
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

	while(1) {
        /* Server port is already opened.
         * Start listening to the port.
         */
        ret = recvfrom(udpInstance->serverSocketFD, (void *) udpInstance->tempBuffer,
                        DDFS_HEADE_SIZE, 0,
                        (struct sockaddr *) &serverAddr, (socklen_t *)&serverAddrLen);

        if(ret == -1) {
	        global_logger << ddfsLogger::LOG_WARNING << "UDP:: BT : Fail to get IP address from Host Name. "
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

