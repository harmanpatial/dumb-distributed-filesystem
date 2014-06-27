/*
 * @file ddfs_clusterMemberPaxos.cpp
 *
 * @brief Module containing the cluster member class.
 *
 * This is the module that contains cluster member class.
 *
 * @author Harman Patial <harman.patial@gmail.com>
 *
 */

#include <fstream>
#include <string>

#include "ddfs_clusterMemberPaxos.h"
#include "../global/ddfs_status.h"

using namespace std;

ddfsClusterMemberPaxos::ddfsClusterMemberPaxos() {
	clusterID = -1;
	memberID = -1;
	uniqueIdentification = -1;
	memberState = s_clusterMemberUnknown;
	return;
}

ddfsStatus ddfsClusterMemberPaxos::isOnline() {
	clustermemberLock.lock();
	return (ddfsStatus(DDFS_FAILURE));
	clustermemberLock.unlock();
}

ddfsStatus ddfsClusterMemberPaxos::isDead() {
	clustermemberLock.lock();
	return (ddfsStatus(DDFS_FAILURE));
	clustermemberLock.unlock();
}

clusterMemberState ddfsClusterMemberPaxos::getCurrentState() {
	clustermemberLock.lock();
	return memberState;	
	clustermemberLock.unlock();
}

ddfsStatus ddfsClusterMemberPaxos::setCurrentState(clusterMemberState newState) {
	clustermemberLock.lock();
	memberState = newState;
	clustermemberLock.unlock();
	return (ddfsStatus(DDFS_OK));
}

void ddfsClusterMemberPaxos::setMemberID(int newMemberID) {
	clustermemberLock.lock();
	memberID = newMemberID;
	clustermemberLock.unlock();
}

int ddfsClusterMemberPaxos::getMemberID() {
	clustermemberLock.lock();
	return memberID;
	clustermemberLock.unlock();
}

void ddfsClusterMemberPaxos::setUniqueIdentification(int newIdentifier) {
	clustermemberLock.lock();
	uniqueIdentification =  newIdentifier;
	clustermemberLock.unlock();
}
int ddfsClusterMemberPaxos::getUniqueIdentification() {
	return uniqueIdentification;
}

ddfsStatus ddfsClusterMemberPaxos::sendClusterMetaData(ddfsClusterMessagePaxos *message) {


	return (ddfsStatus(DDFS_OK));
}


