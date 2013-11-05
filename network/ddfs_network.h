/**
 * \file ddfs_network.h
 *
 * Module for managing network communication.
 *
 * This is the module that would be responsible for network
 * data management.
 * Primary responsibility of this module :
 *
 * 1. Open/Close connection.
 * 2. Send/Receive data from network.
 * 3. Notify other modules about the lost of other nodes in the network/cluster.
 * 4. Network congestion detection and prevention.
 *
 * TODO:
 * 1. Return type should be a of class type.
 * 2. Look into zero copy implementation.
 *
 * Author Harman Patial <harman.patial@gmail.com>
 */

class Network {
	Network();
	~Network();
protected:
	/* @sa openConnection				*/
	/**
	 * @sa openConnection
	 *
	 * @brief - Open a UDP connection.
	 *
	 * Open a UDP connection that would be used to
	 * communicate with other nodes in the DFS.
	 *
	 * @return DDFS_OK	Success
	 * @return DDFS_FAILURE	Failure
	 */
	virtual ddfsStatus openConnection() = 0;
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
	virtual ddfsStatus sendData(void *data, int size, void (*fn)(int)) = 0;
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
	virtual ddfsStatus subscribe(void (*)(int)) = 0;
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
};
