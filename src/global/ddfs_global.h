/*
 * @file ddfsGlobal.h
 *
 * Module for setting global variables.
 *
 * This is the module that contains global variables.
 *
 * @author Harman Patial <harman.patial@gmail.com>
 */
#ifndef DDFS_GLOBAL_H
#define DDFS_GLOBAL_H

#include <iostream>
#include <string>

#include "ddfs_status.h"
#include "./../logger/ddfs_fileLogger.h"

/**
 * @class ddfsGlobal
 *
 * @brief The class is used to initialize the DDFS.
 *
 * This class is used to initialize the various global
 * parameter of the DDFS. This is also used to initialize
 * the logger.
 *
 * @note No object of this class is allowed.
 */
class ddfsGlobal {
public:
	/*		initialize		*/
	/**
	 *
	 * @note If log_file is NULL, the default file name would be created.
	 *
	 * @param[in]  log_file		Full path of the log file
	 *
	 * @return  DDFS_OK		Success
	 * @return  DDFS_FAILURE	Failure, if not able to create/open log file.
	 */
	static ddfsStatus initialize(string log_file);

private:
	static ddfsLogger &global_logger;

	ddfsGlobal();
	ddfsGlobal(ddfsGlobal const&);     // Don't Implement
	void operator=(ddfsGlobal const&); // Don't implement

	/*! \var int major_version
	 * Major Version Number
	 */
	const static int major_version = 0;
	/*! \var int minor_version
	 * Minor Version Number
	 */
	const static int minor_version = 9;
	/*! \var int patch_version
	 * Patch Version Number
	 */
	const static int patch_version = 0;
	~ ddfsGlobal();
};

#endif /* Ending DDFS_GLOBAL_H */
