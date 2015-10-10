/*!
 * \file ddfs_cluster.h 
 *
 * \brief Module containing the cluster class.
 *
 * This is the module that contains cluster class.
 *
 * \author Harman Patial <harman.patial@gmail.com>
 *
 */
#ifndef DDFS_CLUSTER_H
#define DDFS_CLUSTER_H

#include <fstream>
#include <string>
#include <list>

using std::list;

#include "ddfs_clusterMember.hpp"
#include "../global/ddfs_status.hpp"

/**
 * \class ddfsCluster
 *
 * \brief Cluster class for DDFS.
 *
 * A cluster implementation for the DDFS.
 *
 * \note This would implement all the cluster related functions.
 * 	 A singleton class.
 *
 * \note 
 *	 
 */
template <typename T_ddfsClusterMember, typename T_ddfsMemberIdentification = int>
class ddfsCluster {
public:
	int clusterID;
	list<T_ddfsClusterMember> clusterMembers;
	uint64_t paxosProposalNumber;
	virtual ddfsStatus leaderElection() = 0;
    /* !
    *  \brief  asyncEventHandling
    *
    *  \param[in]   buffer      The pointer to the buffer received.
    *  \param[in]   bufferCount The count of the buffer.
    *
    *  \return      void
    *
    *  \note        This is what would handle cluster data.
    *               All the data is of a specific format.
    */
    virtual void asyncEventHandling(void *buffer, int bufferCount) = 0;
    virtual ddfsStatus addMember(T_ddfsMemberIdentification) = 0;
    virtual ddfsStatus addMembers() = 0;
    virtual ddfsStatus removeMember(T_ddfsMemberIdentification) = 0;
    virtual ddfsStatus removeMembers() = 0;
public:
	static const int s_clusterIDInvalid = -1;
	ddfsCluster() {}
	virtual ~ddfsCluster() {};
private:
	ddfsCluster(ddfsCluster const&);     // Don't Implement
	void operator=(ddfsCluster const&); // Don't implement
}; // class end

#endif /* Ending DDFS_CLUSTER_H */
