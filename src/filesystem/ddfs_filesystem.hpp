/*!
 *    \file  ddfs_filesystem.hpp
 *   \brief  This file contains the interface for implementing the filesystem
 *   		 interface.
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

#ifndef DDFS_FILESYSTEM_HPP
#define DDFS_FILESYSTEM_HPP

tempate <typename T_fileHandler, 
class ddfsFileSystem {
public:
	virtual ddfsStatus open(string path, int mode, T_fileHandler handler) = 0;
	virtual ddfsStatus close(T_fileHandler handler) = 0;

	virtual ddfsStatus read(T_fileHandler handler, int size, void *buffer) = 0;
	virtual ddfsStatus read(T_fileHandler handler, int size, void *buffer, int offset) = 0;
	virtual ddfsStatus write(T_fileHandler handler, int size, void *buffer) = 0;
	virtual ddfsStatus write(T_fileHandler handler, int size, void *buffer, int offset) = 0;
	
	virtual ddfsStatus seek(T_fileHandler handler, int offset) = 0;

	virtual ddfsStatus create(string directory, string fileName, int mode) = 0;
	virtual ddfsStatus mkdir(string directory, string directoryName) = 0;

	virtual ddfsStatus deleteFile(T_fileHandler handler) = 0;
};

#endif /* Ending DDFS_FILESYSTEM_HPP */

