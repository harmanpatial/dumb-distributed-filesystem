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

#include "../global/ddfs_status.hpp"
#include "../logger/ddfs_fileLogger.hpp"
#include "ddfs_filesystem.hpp"

using namespace std;

ddfsLogger &global_logger_dsf = ddfsLogger::getInstance();

struct meta_node;


class naryTree {
private:
	struct meta_node {
		bool isDirectory;
		string fileName;   // This is the complete fileName starting from (/)
		int fileOffset;
		vector<struct meta_node *> children;
		struct meta_node *parent;
	};

	int __deleteNode(struct meta_node * tobeDeletedNode) {
		bool found = false;
		struct meta_node *parentNode = tobeDeletedNode->parent;

		for(auto it = parentNode->children.begin(); it != parentNode->children.end();) {
			if((*it)->fileOffset == tobeDeletedNode->fileOffset) {
				free(*it);
				parentNode->children.erase(it);
				found = true;
				break;
			} else
				++it;
		}

		if(found == false) {
			global_logger_dsf << ddfsLogger::LOG_ERROR << "Should not have happened";
			return 0;
		}
		return 1;
	}

	int __findNode(int fOffset, string fFileName, bool findOffset, struct meta_node **searchNode) {
		struct meta_node *temp;
		queue<struct meta_node *> tempQ;

		if(!rootNode)
			return 0;

		tempQ.push(rootNode);

		while(!tempQ.empty()) {
			temp = tempQ.front();
			tempQ.pop();
			if(findOffset == true) {
				if(temp->fileOffset == fOffset) {
					searchNode = &temp;
					return 1;
				}
			} else if(findOffset == false) {
				if(temp->fileName.compare(fFileName) == 0) {
					searchNode = &temp;
					return 1;
				}
			}
			for (int i = 0; i < temp->children.size(); i++) {
				tempQ.push(temp->children[i]);
			}
		}
		return 0;
	}


public:
	struct meta_node *rootNode;
	int insertNode(int offset, string newFileName) {
		struct meta_node *parentNode;

		int pos = newFileName.rfind(newFileName);
    	string parentFileName(newFileName, 0, pos);

		if(__findNode(0, parentFileName, false, &parentNode) == 0)
			return 0;

		struct meta_node *newNode = (struct meta_node *)malloc(sizeof(struct meta_node));

		newNode->fileName = newFileName;
		newNode->fileOffset = offset;
		newNode->parent = parentNode;

		parentNode->children.push_back(newNode);
		return 1;
	}

	// All of the children of these meta_node would be deleted
	int removeNode(int tobeDeletedFileOffset) {
		struct meta_node *specificNode;

		if(__findNode(tobeDeletedFileOffset, "", true, &specificNode) == 0)
			return 0;

		return __deleteNode(specificNode);

	}
	// All of the children of these meta_node would be deleted
	int removeNode(string tobeDeletedFileName) {
		struct meta_node *specificNode;

		if(__findNode(0, tobeDeletedFileName, false, &specificNode) == 0)
			return 0;

		return __deleteNode(specificNode);
	}

	int findNode(int findFileOffset, string resultFileName) {
		struct meta_node *specificNode;

		if(__findNode(findFileOffset, NULL, true, &specificNode) == 0)
			return 0;
		
		resultFileName = specificNode->fileName;
		return 1;
	}

	int findNode(string findFileName, int *offsetValue) {
		struct meta_node *specificNode;

		if(__findNode(0, findFileName, false, &specificNode) == 0)
			return 0;
		
		*offsetValue = specificNode->fileOffset;
		return 1;
	}
};

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

	void init();
	void init(string metaFileName);

	ddfsSimpleFilesystem() {}
	~ddfsSimpleFilesystem() {}
private:
	string metadataFileName;
	static const int metaDatablockSize = 256;

}; 

#endif /* Ending DDFS_SIMPLEFILESYSTEM */

