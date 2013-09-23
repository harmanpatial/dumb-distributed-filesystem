/*
 * ddfs_network.h
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
	/*
	 * DDFS_OK - Success.
	 * DDFS_FAILURE - Failure.
	 */
	virtual DDFS_STATUS openConnection() = 0;
	/*
	 * Name : sendData
	 *
	 * Send data across from this network connection.
	 *
	 * Parameter :
	 *
	 * data : The pointer to the data region.
	 * size : The size of the data to send.
	 * fn : callback function that would be called when
	 * 	host receives response for this data.
	 *
	 * DDFS_OK - Success.
	 * DDFS_NETWORK_RETRY - Retry after some time.
	 * DDFS_HOST_DOWN - Host is down.
	 * DDFS_FAILURE - Failure.
	 */
	virtual DDFS_STATUS sendData(void *data, int size, void (*fn)(int)) = 0;
	/*
	 * Name : receiveData
	 *
	 * Receive data from this network connection.
	 *
	 * Parameter :
	 *
	 * des : The pointer to the data region.
	 * requestedSize : The size of the data to send.
	 * actualSize : Actual size of data currently received.
	 *
	 * Return :
	 * DDFS_OK - Success.
	 * DDFS_HOST_DOWN - Host is down.
	 * DDFS_NETWORK_UNDERRUN - Received data is less than requested for.
	 * 			   actualSize would be filled with the actual data size.
	 * DDFS_NETWORK_OVERRUN - Received data is more than requested for.
	 * 			  actualSize would be filled with the actual data size.
	 * DDFS_FAILURE - Failure.
	 */
	virtual DDFS_STATUS *receiveData(void *des, int requestedSize, int *actualSize) = 0;
	/*
	 * Name : checkConnection
	 *
	 * Check the current state of the connection.
	 *
	 * Parameter :
	 *
	 * des : The pointer to the data region.
	 * requestedSize : The size of the data to send.
	 * actualSize : Actual size of data currently received.
	 *
	 * DDFS_OK - Success.
	 * DDFS_HOST_DOWN - Host is down.
	 * DDFS_FAILURE - Failure.
	 */
	virtual DDFS_STATUS checkConnection() = 0;
	/*
	 * Name : subscribe
	 *
	 * Subscribe to this network connection.
	 * Register a callback function that would be called anytime
	 * data is received from this network connection.
	 *
	 * Return :
	 * DDFS_OK - Success.
	 * DDFS_FAILURE - Failure.
	 */
	virtual DDFS_STATUS subscribe(void (*)(int)) = 0;
	/*
	 * Name : closeConnection
	 *
	 * Close a network connection.
	 *
	 * Return:
	 * VMK_OK - Success.
	 * VMK_FAILURE - Failure.
	 */
	virtual DDFS_STATUS closeConnection() = 0;
	/*
	 * Name : closeConnection
	 *
	 * For performance sake, we should share memory and
	 * should be doing zero copy. But for now, just copy
	 * data over.
	 *
	 * NOTE :
	 * Not sure what purpose does this interface fulfills.
	 *
	 * TODO :
	 * Write a better description for this call.
	 *
	 */
	virtual DDFS_STATUS copyData(void *des, int requestedSize, int *actualSize) = 0;
};
