/*
 * @file ddfs_udpConnection.h
 *
 * Module for managing network communication.
 *
 * This is the module that would be responsible for network
 * data management.
 *
 * Author Harman Patial <harman.patial@gmail.com>
 */

#ifndef DDFS_UDPCONNECTION_H
#define DDFS_UDPCONNECTION_H

#include <iostream>
#include <strings.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <unistd.h>

#include "ddfs_network.h"
#include "../logger/ddfs_fileLogger.h"
#include "../global/ddfs_status.h"

#define DDFS_SERVER_PORT	5000
#define MAX_CLUSTER_NODES	4
#define MAX_UDP_CONNECTIONS	MAX_CLUSTER_NODES

class UdpConnection : public Network {
public:
	UdpConnection();
	~UdpConnection();

#if 0
	/* TODO : Why do we need this ?? */
	static const string client_list[MAX_CLUSTER_NODES];
#endif	
	ddfsStatus openConnection(bool, string specific_data);
	ddfsStatus sendData(void *data, int size, void (*fn)(int));
	ddfsStatus receiveData(void *des, int requestedSize,
				int *actualSize);
	ddfsStatus checkConnection();
	ddfsStatus subscribe(void (*)(int));
	ddfsStatus closeConnection();
	ddfsStatus copyData(void *des, int requestedSize,
			int *actualSize);

	/* NOTE : The attributes specific to this class */
	/* Some kind of async routine needs to be
	 * registered with this interface, that can be called when
	 * some interesting event happens on the netork.
	 *
	 * Interesting events include : 
	 * -- Able to successfully login to a node in the cluster.
	 * -- Lost connection with a existing node in the cluster.
	 * -- Any change in the status of the connection.
	 */
	pthread_t bk_thread;
	static void* bk_routine(void *);
private:
	int server_sockfd_k;
	int client_sockfd[MAX_UDP_CONNECTIONS];
	int client_status[MAX_UDP_CONNECTIONS];
};

#endif /* Ending DDFS_UDPCONNECTION_H */
