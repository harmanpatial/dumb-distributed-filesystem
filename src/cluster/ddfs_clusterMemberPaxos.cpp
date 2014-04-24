/*
 * @file ddfs_clusterMemberPaxos.cpp
 *
 * @breif Module containing the cluster class.
 *
 * This is the module that contains cluster class.
 *
 * @author Harman Patial <harman.patial@gmail.com>
 *
 */

#include <fstream>
#include <string>

#include "ddfs_clusterMemberPaxos.h"
#include "../global/ddfs_status.h"

using namespace std;

ddfsStatus ddfsClusterMemberPaxos::init() {

	return (ddfsStatus(DDFS_FAILURE));
}
#if 0
	ddfsStatus isOnline();
	ddfsStatus isDead();
	ddfsStatus getCurrentState();
	ddfsStatus setCurrentState(ddfsStatus);
	void setMemberID(Integer);
	int getMemberID();
	void setUniqueIdentification(tempate <>);
	int getUniqueIdentification();
#endif