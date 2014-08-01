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

#include "ddfs_clusterMember.h"
#include "../global/ddfs_status.h"

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
template <typename T_ddfsClusterMember>
class ddfsCluster {
protected:
	int clusterID;
	list<T_ddfsClusterMember> clusterMembers;
	uint64_t paxosProposalNumber;
	~ddfsCluster();
	virtual ddfsStatus leaderElection();
    /* !
    *  \brief  asyncEventHandling
    *
    *  \param[in]   buffer      The pointer to the buffer received.
    *  \param[in]   bufferCount The count of the buffer.
    *
    *  \return      void
    *
    *  \note        This is what would handle to cluster data.
    *               All the data is of a specific format.
    */
    virtual void asyncEventHandling(void *buffer, int bufferCount);
	virtual ddfsStatus addMember(T_ddfsClusterMember);
	virtual ddfsStatus addMembers();
	virtual ddfsStatus deleteMember(T_ddfsClusterMember);
	virtual ddfsStatus deleteMembers();
public:
	static const int s_clusterIDInvalid = -1;
	ddfsCluster();
private:
	ddfsCluster(ddfsCluster const&);     // Don't Implement
	void operator=(ddfsCluster const&); // Don't implement
}; // class end

#endif /* Ending DDFS_CLUSTER_H */
