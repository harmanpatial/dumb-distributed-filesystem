/*
 * ddfs_udpConnection.cpp
 *
 * Module for managing network communication.
 *
 * This is the module that would be responsible for network
 * data management.
 * Primary responsibility of this module :
 *
 * TODO:
 * 1. Return type should be a of class type.
 * 2. Look into zero copy implementation.
 *
 * Author Harman Patial <harman.patial@gmail.com>
 */

#include <iostream>
#include <strings.h>
#include <netinet/in.h>

#include "ddfs_network.h"
#include "../logger/ddfs_fileLogger.h"
#include "../global/ddfs_status.h"

#define DDFS_SERVER_PORT	5000

ddfsLogger &global_logger = ddfsLogger::getInstance();

class UdpConnection : public Network {
protected:
	/* openConnection				*/
	/**
	 * @brief - Open a UDP connection.
	 *
	 * Open a UDP connection that would be used to
	 * communicate with other nodes in the DFS.
	 *
	 * @return DDFS_OK	Success
	 * @return DDFS_FAILURE	Failure
	 */
	ddfsStatus openConnection() {
		struct sockaddr_in server_addr, client_addr;
		int sockfd;

		if (socket(AF_INET, SOCK_DGRAM, 0) == -1) {
			global_logger << ddfsLogger::LOG_WARNING << "Unable to open socket";
			return (ddfsStatus(DDFS_FAILURE));
		}

		server_addr.sin_family = AF_INET;
		server_addr.sin_port = htons(DDFS_SERVER_PORT);
		server_addr.sin_addr.s_addr = INADDR_ANY;
		bzero(&(server_addr.sin_zero),8);

		/* Bind socket with the server */
		if (bind(sockfd,(struct sockaddr *)&server_addr,
		    sizeof(struct sockaddr)) == -1)
		{
		    perror("Unable to Bind");
			return (ddfsStatus(DDFS_FAILURE));
		}

		return (ddfsStatus(DDFS_FAILURE));
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
	ddfsStatus sendData(void *data, int size, void (*fn)(int)) {
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
	ddfsStatus receiveData(void *des, int requestedSize, int *actualSize) {
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
	ddfsStatus checkConnection() {
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
	ddfsStatus subscribe(void (*)(int)) {
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
	ddfsStatus closeConnection() {
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
	ddfsStatus copyData(void *des, int requestedSize, int *actualSize) {
		return (ddfsStatus(DDFS_FAILURE));
	}
};
