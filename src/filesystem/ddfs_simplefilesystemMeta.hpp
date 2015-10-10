/*!
 *    \file  ddfs_simplefilesystemMeta.hpp
 *   \brief  This class is responsible for managing the metadata file.
 *  
 *  This would also be responsible for the following operations.
 *
 *  1. Periodically copying the metadata file to a secondary server.
 *  2. Periodically doing garbage collection.
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

#ifndef DDFS_SIMPLEFILESYSTEMMETA_HPP
#define DDFS_SIMPLEFILESYSTEMMETA_HPP

#include <iostream>
#include <sstream>
#include <vector>
#include <map>
#include <set>
#include <queue>
#include <stack>
#include <algorithm>
#include <numeric>
#include <cstdio>

#include <stdio.h>
#include <string.h>
#include <stdint.h>

#include "../global/ddfs_status.hpp"
#include "../logger/ddfs_fileLogger.hpp"

using namespace std;

ddfsLogger &global_logger_dsfh = ddfsLogger::getInstance();

struct meta_node;

#define NO	0
#define YES	1

class naryTree {
private:
	struct meta_node {
		bool isDirectory;
		string fileName;   // This is the complete fileName starting from (/)
		int fileOffset;
		vector<struct meta_node *> children;
		struct meta_node *parent;
	};

	struct meta_node *rootNode;

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
			global_logger_dsfh << ddfsLogger::LOG_ERROR << "Should not have happened";
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
			for (unsigned int i = 0; i < temp->children.size(); i++) {
				tempQ.push(temp->children[i]);
			}
		}
		return 0;
	}


public:
	int empty() {
		if(rootNode == NULL) return 1;
		return 0;
	}
	int insertNode(int offset, string newFileName) {
		struct meta_node *parentNode;

        if((newFileName.compare("/") == 0) && (rootNode == NULL)) {
		    rootNode = (struct meta_node *)malloc(sizeof(struct meta_node));
            rootNode->fileName = newFileName;
            rootNode->fileOffset = offset;
            rootNode->parent = NULL;
            return 1;
        }

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

class ddfs_simplefilesystemMeta {

private:
	string metadataFileName;
	FILE *metaFileHandler;
	
	/* 
	 *  One block of Metadata file contains the following. Total Size : 1024 bytes
	 *
	 *  1.  FileName -- 256 bytes/character
	 *  2.  isDirectory.  -- 4 bytes
	 *  3.  Number of files in this directory.  -- 4 bytes.
	 *  4.  Offset of this metaData block. -- 8 bytes.
	 *  5.  Permissions -- 8 bytes.
	 *  6.  Creation Time -- 8 bytes.
	 *  7.  Last Access Time -- 8 bytes.
	 *  9.  Last Modified Time -- 8 bytes.
	 *  10. Reserved -- 336 bytes.
	 *  11. Offset to 1st file in this directory -- 8 bytes.
	 *  12. Offset to 2nd file in this directory -- 8 bytes.
	 *  13. Offset to 3nd file in this directory -- 8 bytes.
	 *  14. Offset to 4th file in this directory -- 8 bytes.
	 *  15. Offset to 5th file in this directory -- 8 bytes.
	 *  16. ---- 
	 *  17. ---- 
	 *  18. Offset to 23th file in this directory -- 8 bytes.
	 *  19. Offset to next block for this directory -- 8 bytes.
	 *
	 *
	 *  If the file is a file and not directory then from 11 onwards
	 *  metadata would be.
	 *
	 *  11. Complete path to Primary copy of the Data. -- 128 bytes.
	 *  12. Complete path to 2nd copy of the Data. -- 128 bytes.
	 *  13. Complete path to 3rd copy of the Data. -- 128 bytes.
	 *  14. Reserved -- 192 bytes.
	 * 
	 */
	static const int metaDatablockSize = 1024;
    static const int fileNameSize = 256;
    static const int numberOfFileInOneBlock = 23;
	struct dData {
		uint64_t filesOffset[numberOfFileInOneBlock];
		uint64_t nextBlockOffset;
	};
	struct fData {
		char primary_copy[128];
		char second_copy[128];
		char third_copy[128];
		uint8_t reserved[192];
	};
	struct metaDataBlock {
		char fileName[fileNameSize];
		uint32_t isDirectory;
		uint32_t numberOfFiles;
		uint64_t offset;
		uint64_t permissions;
		uint64_t creationTime;
		uint64_t lastAccessTime;
		uint64_t lastModifiedTime;
		uint8_t reserved_0[336];
		union {
			dData directoryData;
			fData fileData;
		} data;
	}  __attribute__((packed));
public:
	ddfsStatus init(string newFile) {
		metadataFileName = newFile;
		return (ddfsStatus(DDFS_OK));
	}
	
private:
	naryTree inMemDirectoryTree;

public:
/*  
	DDFS_FILESYSTEM_FILE_DOES_NOT_EXIST,
	DDFS_FILESYSTEM_FILE_PERMISSIONS_DENIED,
	DDFS_FILESYSTEM_CORRUPTED
*/

	ddfsStatus fillInMemDirectoryTree() {
		//FILE * pFile;
		metaDataBlock root;
		int result;

		if(metadataFileName.empty() || !inMemDirectoryTree.empty())
			return (ddfsStatus(DDFS_FILESYSTEM_FILE_DOES_NOT_EXIST));

		if((metaFileHandler = fopen(metadataFileName.c_str(), "r+")) == NULL) {
			return (ddfsStatus(DDFS_FAILURE));
		}

		result = fread((void *) &root, metaDatablockSize, 0, metaFileHandler);
		if(result == 0) {
            /* Filesystem is not yet initalized.
             * Initialize it and exit
             */
            strncpy((char *) &root.fileName, "/", 2);
            root.isDirectory = YES;
            root.numberOfFiles = 0;
			root.offset = 0;
            root.permissions = 0x50505;  // Read, Write permission for all 
            root.creationTime = 1;  // TODO: Fix the timing
            root.lastAccessTime = 1;  // TODO: Fix the timing
            root.lastModifiedTime = 1;  // TODO: Fix the timing
            bzero(&(root.data.directoryData.filesOffset), numberOfFileInOneBlock*sizeof(uint64_t));;
            root.data.directoryData.nextBlockOffset = -1;

            fseek(metaFileHandler, 0, SEEK_SET);
            int res = fwrite(&root, metaDatablockSize, 1, metaFileHandler); 
            if(res != metaDatablockSize)
                return (ddfsStatus(DDFS_FAILURE));
        } else if(result != metaDatablockSize) {
			/* CRITICAL ERROR : FILESYSTEM CORRUPTION
			 * 
			 * This should not happen, we always read-write from metadata file in 512 bytes.
			 * Data in the metaData file is ALWAYS 512 bytes align.
			 * */
			return (ddfsStatus(DDFS_FILESYSTEM_CORRUPTED));
		}

        /* Insert Root Node */
        /* Should do a DFS in the B Tree contained in the metaData file.
         * DFS would consume less memory space as compared to BFS.
        */
        queue<metaDataBlock *> blocksQueue;

		blocksQueue.push(&root);
		metaDataBlock *temp;
		metaDataBlock *temp1;

		while(!blocksQueue.empty()) {
			temp = blocksQueue.front();
			blocksQueue.pop();
			
			inMemDirectoryTree.insertNode(temp->offset, temp->fileName);
			
			for (unsigned int i = 0; i < temp->numberOfFiles; i++) {
            	fseek(metaFileHandler, temp->data.directoryData.filesOffset[i], SEEK_SET);

				temp1 = (metaDataBlock *) malloc(sizeof(metaFileHandler));
			    result = fread((void *) temp1, metaDatablockSize, 0, metaFileHandler);
				if(result != metaDatablockSize) {
					/* CRITICAL ERROR : FILESYSTEM CORRUPTION
					 * 
					 * This should not happen, we always read-write from metadata file in 512 bytes.
					 * Data in the metaData file is ALWAYS 512 bytes align.
					 * */
					return (ddfsStatus(DDFS_FILESYSTEM_CORRUPTED));
    	        }
				blocksQueue.push(temp1);
        	}
		}

		return (ddfsStatus(DDFS_OK));
	} 
/*
	ddfsStatus get() {




	}
*/
};



#endif /* Ending DDFS_SIMPLEFILESYSTEMMETA_HPP */

