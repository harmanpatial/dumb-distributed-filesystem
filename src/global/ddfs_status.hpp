/*
 * ddfsStatus.h 
 *
 * Status used across the DDFS modules.
 *
 * Author Harman Patial <harman.patial@gmail.com>
 */
#ifndef DDFS_STATUS_H
#define DDFS_STATUS_H

enum DDFS_STATUS {
	DDFS_OK,
	DDFS_HOST_DOWN,
	DDFS_NETWORK_NO_DATA,
	DDFS_NETWORK_RETRY,
	DDFS_NETWORK_UNDERRUN,
	DDFS_NETWORK_OVERRUN,
	DDFS_GENERAL_PARAM_INVALID,
	DDFS_CLUSTER_INSUFFICIENT_NODES,
    DDFS_CLUSTER_ALREADY_MEMBER,
	DDFS_FILESYSTEM_FILE_DOES_NOT_EXIST,
	DDFS_FILESYSTEM_FILE_PERMISSIONS_DENIED,
	DDFS_FILESYSTEM_CORRUPTED,
	DDFS_FAILURE
};

class ddfsStatus {

private:
	DDFS_STATUS status;
public:
	explicit ddfsStatus(DDFS_STATUS local_status);
	/**
 	 *   statusToString
 	 *
 	 * Print the human readable string for the DDFS status.
 	 *
 	 */
	std::string statusToString();
	/* Do not use this outside this class member function */
	DDFS_STATUS getStatus();
	bool compareStatus(ddfsStatus);
}; // class end

#endif /* Ending DDFS_STATUS_H */
