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
	 * DDFS_OK - Success.
	 * DDFS_NETWORK_RETRY - Retry after some time.
	 * DDFS_HOST_DOWN - Host is down.
	 * DDFS_FAILURE - Failure.
	 */
	virtual DDFS_STATUS sendData(void *d) = 0;
	/*
	 * DDFS_OK - Success.
	 * DDFS_HOST_DOWN - Host is down.
	 * DDFS_FAILURE - Failure.
	 */
	virtual DDFS_STATUS *receiveData() = 0;
	/*
	 * DDFS_OK - Success.
	 * DDFS_HOST_DOWN - Host is down.
	 * DDFS_FAILURE - Failure.
	 */
	virtual DDFS_STATUS checkConnection() = 0;
	/*
	 * DDFS_OK - Success.
	 * DDFS_FAILURE - Failure.
	 */
	virtual DDFS_STATUS subscribe(void (*)(int)) = 0;
	virtual DDFS_STATUS closeConnection() = 0;
	/* For performance sake, we should share memory and
	 * should be doing zero copy. But for now, just copy
	 * data over
	 */
	virtual void * copyData(void *des, int size) = 0;
};
