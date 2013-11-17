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
#include "./../logger/ddfs_fileLogger.h"

ddfsLogger& ddfsGlobal::global_logger = ddfsLogger::getInstance();
int ddfsGlobal::initialization_done = 0;

ddfsStatus ddfsGlobal::initialize(string log_file) {
	/* Open log file and write the starting message */
	char buf[64];

	if(ddfsGlobal::initialization_done == 1)
		return (ddfsStatus(DDFS_OK));

	global_logger = ddfsLogger::getInstance();

	// Writing warnings or errors to file is very easy and C++ style
	global_logger << ddfsLogger::LOG_WARNING << "DDFS(" << major_version << "."
				<< minor_version  << "." << patch_version  << ") -- STARTING.\n";

	ddfsGlobal::initialization_done = 1;

	return (ddfsStatus(DDFS_OK));
}
