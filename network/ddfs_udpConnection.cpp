/*
 * ddfs_udpConnection.cpp
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
#include <stdio>

class UdpConnection : public Network {
protected:
	/*
	 * DDFS_OK - Success.
	 * DDFS_FAILURE - Failure.
	 */
	DDFS_STATUS openConnection() {
		return DDFS_FAILURE;
	}
	/*
	 * DDFS_OK - Success.
	 * DDFS_NETWORK_RETRY - Retry after some time.
	 * DDFS_HOST_DOWN - Host is down.
	 * DDFS_FAILURE - Failure.
	 */
	DDFS_STATUS sendData(void *d) {
		return DDFS_FAILURE;
	}
	/*
	 * DDFS_OK - Success.
	 * DDFS_HOST_DOWN - Host is down.
	 * DDFS_FAILURE - Failure.
	 */
	DDFS_STATUS *receiveData() {
		return DDFS_FAILURE;
	}
	/*
	 * DDFS_OK - Success.
	 * DDFS_HOST_DOWN - Host is down.
	 * DDFS_FAILURE - Failure.
	 */
	DDFS_STATUS checkConnection() = 0 {
		return DDFS_FAILURE;
	}
	/*
	 * DDFS_OK - Success.
	 * DDFS_FAILURE - Failure.
	 */
	virtual DDFS_STATUS subscribe(void (*)(int)) {
		return DDFS_FAILURE;
	}
	/*
	 * DDFS_OK - Success.
	 * DDFS_FAILURE - Failure.
	 */
	virtual DDFS_FAILURE closeConnection() {
		return DDFS_FAILURE;
	}
	/* For performance sake, we should share memory and
	 * should be doing zero copy. But for now, just copy
	 * data over
	 */
	virtual void * copyData(void *des, int size) = 0 {
		return DDFS_FAILURE;
	}


};
