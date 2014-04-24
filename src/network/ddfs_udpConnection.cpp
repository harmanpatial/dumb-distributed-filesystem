/*
 * @file ddfs_udpConnection.cpp
 *
 * Module for managing network communication.
 *
 * This is the module that would be responsible for network
 * data management.
 *
 * TODO:
 * 1. Look into zero copy implementation.
 *
 * Author Harman Patial <harman.patial@gmail.com>
 */

#include <iostream>
#include <strings.h>

#include "ddfs_udpConnection.h"
#include "../logger/ddfs_fileLogger.h"
#include "../global/ddfs_status.h"

#define DDFS_SERVER_PORT	5000

ddfsLogger &global_logger = ddfsLogger::getInstance();

UdpConnection::UdpConnection() {
	network_type = DDFS_NETWORK_UDP;
}

/* openConnection				*/
/**
 * @brief - Open a UDP connection.
 *
 * Open a UDP connection that would be used to
 * communicate with other nodes in the DFS.
 *
 * As this is a cluster environment, need to start a server
 * port as well as a client port.
 *
 * @param   isClient[in]	Is "true" if this is a client connection.
 * @param   serverIp[in]	If "isClient" is true, this would be the
 * 				server IP to connect to.
 *
 * @return DDFS_OK	Success
 * @return DDFS_FAILURE	Failure
 */
ddfsStatus UdpConnection::openConnection(bool isClient, string serverIp) {
	struct sockaddr_in server_addr;
	int server_sockfd = 0;

	/* Open the server socket and wait for the client connections */
	if(isClient == false) {
		/* Open the server port to except incoming connections */
		if ((server_sockfd = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
			global_logger << ddfsLogger::LOG_WARNING << "Server :: Unable to open socket.\n";
			return (ddfsStatus(DDFS_FAILURE));
		}

		server_addr.sin_family = AF_INET;
		server_addr.sin_port = htons(DDFS_SERVER_PORT);
		server_addr.sin_addr.s_addr = INADDR_ANY;
		bzero(&(server_addr.sin_zero),8);

		/* Bind socket with the server */
		if (bind(server_sockfd,(struct sockaddr *)&server_addr,
			sizeof(server_addr)) == -1)
		{
			global_logger << ddfsLogger::LOG_WARNING << "UDP::Server :: Unable to bind socket. "
								<< strerror(errno) <<"\n";
			return (ddfsStatus(DDFS_FAILURE));
		}
		server_sockfd_k = server_sockfd;
		return (ddfsStatus(DDFS_FAILURE));
	}

	if(isClient == true && !serverIp.empty()) {
		/* Start a thread that would create udp connections with other
		 * nodes in the cluster.
		 */
		if(!pthread_create(&bk_thread, NULL, &UdpConnection::bk_routine, NULL)) {
			global_logger << ddfsLogger::LOG_WARNING
				<< "UDP ::Successfully created the background thread."
				<< strerror(errno) <<"\n";
			return (ddfsStatus(DDFS_FAILURE));
		}
		return (ddfsStatus(DDFS_OK));
	}

	return (ddfsStatus(DDFS_OK));
	/* */
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
ddfsStatus UdpConnection::sendData(void *data, int size, void (*fn)(int)) {
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
ddfsStatus UdpConnection::receiveData(void *des, int requestedSize, int *actualSize) {
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
ddfsStatus UdpConnection::checkConnection() {
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
ddfsStatus UdpConnection::subscribe(void (*)(int)) {
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
ddfsStatus UdpConnection::closeConnection() {

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
ddfsStatus UdpConnection::copyData(void *des, int requestedSize, int *actualSize) {
	return (ddfsStatus(DDFS_FAILURE));
}

/*	bk_routine			*/
/**
 * @brief   This is background thread routine.
 *
 * This is the background thread routine used to handle
 * network events including node unreachable/node temrerory unavailable.
 *
 * @note Background routine for processing network events.
 *
 * @param[in]  None.
 *
 * @return   None.
 */
void* UdpConnection::bk_routine(void *arg) {
#if 0
	struct sockaddr_in server_addr;
	client_sockfd = 0;
#endif
	int i = 0;

	global_logger << ddfsLogger::LOG_WARNING << "UDP:: Started the background thread. "
						<< strerror(errno) <<"\n";

	while(1) {
		for(i=0; i<MAX_UDP_CONNECTIONS; i++) {
#if 0
			if(client_list[i].empty())
				continue;

			/* Open the client port and try to connect to other nodes
			 * in the cluster.
			 */
			if ((client_sockfd = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
				global_logger << ddfsLogger::LOG_WARNING << "Client :: Unable to open socket.\n";
				continue;
			}

			server_addr.sin_family = AF_INET;
			server_addr.sin_port = htons(DDFS_SERVER_PORT);

			inet_pton(AF_INET, client_list[i].c_str(), &server_addr.sin_addr);
			memset(&(server_addr.sin_zero), 0, 8);

			/* Periodically keep on trying to connect to the set of nodes
			 * that has been configured.
			 */
			if (connect(client_sockfd,(struct sockaddr *)&server_addr,
			    sizeof(struct sockaddr)) == -1)
			{
				global_logger << ddfsLogger::LOG_WARNING
						<< "UDP::Client :: Unable to connect socket. "
						<< strerror(errno) <<"\n";
				continue;
			}
#endif
			/* Able to make a udp connection successful.
			 * Should wait for the remote node to do a successful login
			 * to local node.
			 */

		} /* for loop end */
		/* Sleep for 10 minutes */
		sleep(10);
	} /* while loop end */

	pthread_exit(NULL);

}
