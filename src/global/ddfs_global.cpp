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

using namespace std;

#include "ddfs_global.h"

#include "./../network/ddfs_udpConnection.h"
#include "./../logger/ddfs_fileLogger.h"

ddfsLogger& ddfsGlobal::global_logger = ddfsLogger::getInstance();
int ddfsGlobal::initialization_done = 0;

ddfsStatus ddfsGlobal::initialize() {
	/* Open log file and write the starting message */
	char buf[64];

	if(initialization_done == 1)
		return (ddfsStatus(DDFS_OK));

	global_logger = ddfsLogger::getInstance();

	// Writing warnings or errors to file is very easy and C++ style
	global_logger << ddfsLogger::LOG_WARNING << "DDFS(" << major_version << "."
				<< minor_version  << "." << patch_version
				<< ") -- Initialization complete.\n";

	/* TODO : Call the network interface to open server/client
	 * 	  connections.
	 *
	 * 	  Also, call the cluster interface to initialize the cluster
	 * 	  interface.
	 *
	 * 	  Before this call returns, library should be ready to perform
	 * 	  DFS operations.
	 */
	UdpConnection *clientConnection = new UdpConnection();
	UdpConnection *serverConnection = new UdpConnection();

	serverConnection->openConnection(false);
	clientConnection->openConnection(true);



	/* Init done */
	ddfsGlobal::initialization_done = 1;


	return (ddfsStatus(DDFS_OK));
}
