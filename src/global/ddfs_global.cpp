/*
 * @file ddfs_global.cpp
 *
 * Module for setting global variables.
 *
 * This is the module that contains global variables.
 *
 * @author Harman Patial <harman.patial@gmail.com>
 */

#include <string>

#include "ddfs_global.hpp"
#include "./../cluster/ddfs_clusterPaxos.hpp"

ddfsLogger& ddfsGlobal::global_logger = ddfsLogger::getInstance();
int ddfsGlobal::initialization_done = 0;


/* Function : initialize()
 *
 * NOTE :
 * After the call returns, client can issue Distributed 
 * Filesystem operations.
 */
ddfsStatus ddfsGlobal::initialize() {
	/* Open log file and write the starting message */

	if(initialization_done == 1)
		return (ddfsStatus(DDFS_OK));

	global_logger = ddfsLogger::getInstance();

	// ddfsLogger::

	// Writing warnings or errors to file is very easy and C++ style
	global_logger << ddfsLogger::LOG_WARNING << "DDFS(" << major_version << "."
				<< minor_version  << "." << patch_version
				<< ") -- Initialization complete.\n";

	/* TODO :
	 *	 
	 * Call the cluster interface to initialize the cluster.
	 * It is the responsiblity of the cluster interface to use the 
	 * network interface to open server/client connections and send/recieve
	 * network packets.
	 */
//	ddfsCluster cluster = new ddfsCluster();

/* The below commented out code would be in the cluster module */
//	cluster.init();

	/* Init done */
	ddfsGlobal::initialization_done = 1;

	return (ddfsStatus(DDFS_OK));
}
