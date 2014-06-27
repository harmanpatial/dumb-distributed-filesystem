/*
 * ddfs_status.cpp
 *
 * Status used across the DDFS modules.
 *
 * Author Harman Patial <harman.patial@gmail.com>
 */

#include <iostream>
#include "ddfs_status.h"

using namespace std;

ddfsStatus::ddfsStatus(DDFS_STATUS local_status) {
	status = local_status;
}

std::string ddfsStatus::statusToString() {
	switch(status) {
	case DDFS_OK:
		return (std::string("Good"));
		break;
	case DDFS_HOST_DOWN:
		return (std::string("Network : Host is Down"));
		break;
	case DDFS_NETWORK_NO_DATA:
		return (std::string("Network : No Data"));
		break;
	case DDFS_NETWORK_RETRY:
		return (std::string("Network : Try Again"));
		break;
	case DDFS_NETWORK_UNDERRUN:
		return (std::string("Network : Less data than expected"));
		break;
	case DDFS_NETWORK_OVERRUN:
		return (std::string("Network : More data than expected"));
		break;
	case DDFS_GENERAL_PARAM_INVALID:
		return (std::string("General: The paramter passed was invalid"));
		break;
	case DDFS_FAILURE:
		return (std::string("Failure"));
		break;
	default:
		return (std::string("Unkown Status"));
		break;
	}
}

int ddfsStatus::compareStatus(ddfsStatus statusInQuestion) {
	if(statusInQuestion.getStatus() == status)
		return 1;

	return 0;
}

DDFS_STATUS ddfsStatus::getStatus() {
	return status;
}
