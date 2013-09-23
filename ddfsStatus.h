/*
 * ddfs_network.h
 *
 * Status used across the DDFS modules.
 *
 * Author Harman Patial <harman.patial@gmail.com>
 */
enum DDFS_STATUS {
	DDFS_OK,
	DDFS_HOST_DOWN,
	DDFS_NETWORK_NO_DATA,
	DDFS_NETWORK_RETRY,
	DDFS_NETWORK_UNDERRUN,
	DDFS_NETWORK_OVERRUN,
	DDFS_FAILURE
};

void *ddfs_printStatus(DDFS_STATUS status);
