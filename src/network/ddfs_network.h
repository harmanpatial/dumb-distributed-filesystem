/**
 * @file ddfs_network.h
 *
 * Module for managing communication between nodes.
 *
 * This is the module that would be responsible for communication 
 * beween the cluster nodes.
 * Most network protocol work on client/server model. In this case,
 * this module would manage both the connection.
 *
 *________________________________________________
 *  localNode           remoteNode
 *------------------------------------------------
 *
 * Server   <------->   Client
 * Client   <------->   Server
 *
 * Primary responsibility of this module :
 *
 * 1. Open/Close connection.
 * 2. Send/Receive data from network.
 * 3. Notify other modules about the lost of other nodes in the network/cluster.
 * 4. Network congestion detection and prevention(If applicable).
 *
 * This is the module that would be responsible for network
 * data management.
 *
 * One network instance is created per remote node.
 * Connection is between localNode <----> remoteNode.
 *
 * There is one to one relation between udpConnection and MemberPaxos.
 * For the local MemberPaxos there is no network instance.
 *
 * TODO:
 * 1. Look into zero copy implementation.
 *
 * Author Harman Patial <harman.patial@gmail.com>
 */

#ifndef DDFS_NETWORK_H
#define DDFS_NETWORK_H
#include "../global/ddfs_status.h"

enum DDFS_NETWORK_TYPE {
	DDFS_NETWORK_TCP,
	DDFS_NETWORK_UDP,
	DDFS_NETWORK_FC,
	DDFS_NETWORK_ISCSI
};

template <typename T_remoteNodeUniqueID>
class Network {
public:
	/* @sa openConnection				*/
	/**
	 * @sa openConnection
	 *
	 * @brief - Open a UDP connection.
	 *
	 * @param   data		Pointer to the data that needs to be send
	 *
	 * Open a UDP connection that would be used to
	 * communicate with other nodes in the DFS.
	 *
	 * @return DDFS_OK	Success
	 * @return DDFS_FAILURE	Failure
	 */
	virtual ddfsStatus openConnection(T_remoteNodeUniqueID nodeUniqueID) = 0;
	/*	sendData			*/
	/**
	 * @brief   Send data across.
	 *
	 * Send data from the connection that has been
	 * previously established.
	 *
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
	virtual ddfsStatus sendData(void *data, int size, void *privatePtr) = 0;
	/*	receiveData			*/
	/**
	 *
	 * @brief Receive data from this network connection.
	 *
	 * @param   des			The pointer to the data region
	 * @param   requestedSize	The size of the data to send
	 * @param   actualSize		Actual size of data currently received
	 *
	 * @return  DDFS_OK			Success
	 * @return  DDFS_HOST_DOWN		Host is down
	 * @return  DDFS_NETWORK_UNDERRUN	Received data is less than requested for.
	 * 			   		actualSize would be filled with the actual data size
	 * @return  DDFS_NETWORK_OVERRUN	Received data is more than requested for.
	 * 			  		actualSize would be filled with the actual data size.
	 * @return  DDFS_FAILURE		Failure.
	 */
	virtual ddfsStatus receiveData(void *des, int requestedSize, int *actualSize) = 0;

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
	virtual ddfsStatus checkConnection() = 0;

	/*	subscribe			*/
	/**
	 * @brief   Subscribe the connection.
	 *
	 * Subscribe to the connection.
	 * If data is received from this connection, the
	 * registered callback function would be invoked.
	 *
	 * @note .
	 *
	 * @return   DDFS_OK		Success
	 * @return   DDFS_FAILURE	Failure
	 */
	virtual ddfsStatus subscribe(void (*)(int),  void *privatePtr) = 0;
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
	virtual ddfsStatus closeConnection() = 0;
	/*	copyData			*/
	/**
	 * @brief   Copy inbound data in the connection.
	 *
	 * Copy the data being received by the connection.
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
	virtual ddfsStatus copyData(void *des, int requestedSize, int *actualSize) = 0;

	DDFS_NETWORK_TYPE network_type;
//public:
//	Network();
//	~Network();
};

#endif /* DDFS_NETWORK_H */
