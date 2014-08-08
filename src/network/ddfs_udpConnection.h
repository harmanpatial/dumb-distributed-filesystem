/*
 * @file ddfs_udpConnection.h
 *
 * @brief Module for managing network communication.
 *
 * This is the module that would be responsible for network
 * data management.
 *
 * Author Harman Patial <harman.patial@gmail.com>
 */

#ifndef DDFS_UDPCONNECTION_H
#define DDFS_UDPCONNECTION_H

#include <iostream>
#include <vector>
#include <array>
#include <queue>
#include <thread>
#include <strings.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>

using std::string;
using std::queue;
using std::array;
using std::mutex;

#include "ddfs_network.h"
#include "../cluster/ddfs_clusterMessagesPaxos.h"
#include "../logger/ddfs_fileLogger.h"
#include "../global/ddfs_status.h"

#define DDFS_SERVER_PORT	5000
#define MAX_CLUSTER_NODES	4
#define MAX_UDP_CONNECTIONS	MAX_CLUSTER_NODES

struct responseQueue_T;

class ddfsSubscriptionClass {
private:
    std::vector<void (*)(int)> subscriptionFns;
public:
    void addSubscription(void (*subscribeFn)(int)) {
        subscriptionFns.push_back(subscribeFn);
    }

    void removeAllSubscription() {
       for(int i=0; i < subscriptionFns.size(); i++){
                subscriptionFns.erase(subscriptionFns.begin()+i);
        }
    }

    int removeSubscription(void (*subscribeFn)(int)) {
       for(int i=0; i < subscriptionFns.size(); i++){
           if(subscriptionFns[i] == subscribeFn) {
                subscriptionFns.erase(subscriptionFns.begin()+i);
                return 0;
            }
        }
        return -1;
    }

    void callSubscription(int value) {
       for(int i=0; i < subscriptionFns.size(); i++){
           subscriptionFns[i](value);
        }
    }
};

typedef struct {
    std::queue <requestQEntry *> *dataBuffer;
    std::mutex rLock;
    struct responseQueue_T *corrResponseQueue;
}requestQueue;

typedef struct responseQueue_T {
    std::queue <responseQEntry *> *dataBuffer;
    std::mutex rLock;
    requestQueue *corrRequestQueue;

    /* Subscription is bound to response queues.
     * Each component should create it's req-rsp queue
     * when they want to transfer data with this particular
     * node.
     */
    ddfsSubscriptionClass subscriptions;
}responseQueue;

class ddfsUdpConnection : protected Network<string>{
public:
	ddfsUdpConnection();
	~ddfsUdpConnection();

#if 0
	/* TODO : Why do we need this ?? */
	static const string client_list[MAX_CLUSTER_NODES];
#endif	
	ddfsStatus openConnection(string nodeUniqueID);
    ddfsStatus setupQueues(std::queue <requestQEntry *> *reqQueue,
                        std::queue <responseQEntry *> *rspQueue, void *privatePtr);
	ddfsStatus sendData(void *data, int size, void *privatePtr);
	ddfsStatus receiveData(void *des, int requestedSize,
				int *actualSize);
	ddfsStatus checkConnection();
	ddfsStatus subscribe(void (*)(int), void *privatePtr);
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
//    pthread_t bk_thread;
    static void* bk_routine(void *);
private:
    int serverSocketFD;
    int clientSocketFD;
    string remoteNodeHostName;
    sockaddr_in destinationAddr;
    uint32_t destinationAddrSize;

    /* Request/Response Queues */
    std::mutex queuesLock;

    std::array <requestQueue *, 128> requestQueues;
    std::array <responseQueue *, 128> responseQueues;

    /* There is no need for keeping outstanding command list
     * If we are unable to send to data due to send/recieve
     * errors, upper layer is responsible for retrying/recovering
     * of the data.
     */
    std::queue <requestQEntry> *outstandingMessage;
    std::mutex outMessageLock;

    /* Vector thread list */
    std::vector<std::thread> bkThreads;

    std::vector<bool> terminateThreads;

    /* TODO: Following fields are only for analysis */
    class networkAnalysis;

    /* Just a temp buffer for the current incoming message */
    uint64_t tempBuffer[4096];
};

#endif /* Ending DDFS_UDPCONNECTION_H */
