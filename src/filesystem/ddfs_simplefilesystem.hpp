/*!
 *    \file  ddfs_simplefilesystem.hpp
 *   \brief  
 *  
 *  <+DETAILED+>
 *  
 *  \author  Harman Patial, harman.patial@gmail.com
 *  
 *  \internal
 *      Compiler:  g++
 *     Copyright:  
 *  
 *  This source code is released for free distribution under the terms of the
 *  GNU General Public License as published by the Free Software Foundation.
 */

#ifndef DDFS_SIMPLEFILESYSTEM
#define DDFS_SIMPLEFILESYSTEM


#include <fstream>
#include <string>
#include <mutex>
#include <condition_variable>
#include <thread>
#include <vector>
#include <queue>

#include "ddfs_filesystem.hpp"
#include "ddfs_simplefilesystemMeta.hpp"
#include "../global/ddfs_status.hpp"
#include "../logger/ddfs_fileLogger.hpp"

using namespace std;

ddfsLogger &global_logger_dsf = ddfsLogger::getInstance();

class ddfsSimpleFilesystem: public ddfsFileSystem<void *> {
public:
	ddfsStatus openFile(string path, int mode, void *handler);
	ddfsStatus closeFile(void * handler);

	ddfsStatus readFile(void *handler, int size, void *buffer);
	ddfsStatus readFile(void * handler, int size, void *buffer, int offset);
	ddfsStatus writeFile(void * handler, int size, void *buffer);
	ddfsStatus writeFile(void * handler, int size, void *buffer, int offset);
	
	ddfsStatus seekFile(void * handler, int offset);

	ddfsStatus createFile(string directory, string fileName, int mode);
	ddfsStatus makedirectory(string directory, string directoryName);

	ddfsStatus deleteFile(void *handler);

	ddfsStatus init();
	ddfsStatus init(string metaFileName);

	ddfsSimpleFilesystem() {}
	~ddfsSimpleFilesystem() {}
private:
	ddfs_simplefilesystemMeta metaData;

	ddfsStatus fillInMemDirectoryTree();
}; 

#endif /* Ending DDFS_SIMPLEFILESYSTEM */

