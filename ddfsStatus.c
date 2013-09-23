/*
 * ddfs_network.c
 *
 * Status used across the DDFS modules.
 *
 * Author Harman Patial <harman.patial@gmail.com>
 */

void *ddfs_printStatus(DDFS_STATUS status) {

	switch(status) {
	case DDFS_OK:
		return "Good";
		break;
	case DDFS_HOST_DOWN:
		return "Host is Down";
		break;
	case DDFS_NETWORK_NO_DATA:
		return "Network : No Data";
		break;
	case DDFS_NETWORK_RETRY:
		return "Network : Try Again";
		break;
	case DDFS_NETWORK_UNDERRUN:
		return "Network : Less data than expected";
		break;
	case DDFS_NETWORK_OVERRUN:
		return "Network : More data than expected";
		break;
	case DDFS_FAILURE:
		return "Failure";
		break;
	default:
		return "Bad Status";
		break;
	}
}
