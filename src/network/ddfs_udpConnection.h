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

template<typename T>
class ddfsSubscriptionClass {
private:
    std::vector<T*> subscribedInstances;
public:
    void addSubscription(T* owner) {
        subscribedInstances.push_back(owner);
    }

    void removeAllSubscription() {
       for(int i=0; i < subscribedInstances.size(); i++){
                subscribedInstances.erase(subscribedInstances.begin()+i);
        }
    }

    int removeSubscription(T *owner) {
       for(int i=0; i < subscribedInstances.size(); i++){
           if(subscribedInstances[i] == owner) {
                subscribedInstances.erase(subscribedInstances.begin()+i);
                return 0;
            }
        }
        return -1;
    }

    void callSubscription(int value) {
       for(int i=0; i < subscribedInstances.size(); i++){
           subscribedInstances[i]->callback(value);
        }
    }
};

template <typename T_sub>
class responseQueue {
public:
    std::queue <responseQEntry *> *dataBuffer;
    std::mutex rLock;
    int index;

    /* Subscription is bound to response queues.
     * Each component should create it's req-rsp queue
     * when they want to transfer data with this particular
     * node.
     */
    ddfsSubscriptionClass <T_sub> subscriptions;
};

class requestQueue {
public:
    std::queue <requestQEntry *> *dataBuffer;
    std::mutex rLock;
    int index;
};

template <typename T_sub>
class ddfsUdpConnection : protected Network<string, T_sub>{
public:
    ddfsUdpConnection()
    {
    //network_type = DDFS_NETWORK_UDP;
    serverSocketFD = -1;
    clientSocketFD = -1;

    /* Pre allocating memory for storing the header */
    bzero(tempBuffer, sizeof(uint64_t)*4096);

    //global_logger_tem = ddfsLogger::getInstance();

    }

	~ddfsUdpConnection() {}

	ddfsStatus openConnection(string nodeUniqueID, int serverSocket)
    {
	struct sockaddr_in serverAddr;
    int lengthServerAddr;
	struct sockaddr_in clientAddr;
    int lengthClientAddr;

    /* Open the socket for localNode client */
    if ((clientSocketFD = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1) {
        global_logger_tem << ddfsLogger::LOG_WARNING
                    << "networkCommunication :: Unable to open client socket."
                    << strerror(errno) << "\n";
        return (ddfsStatus(DDFS_FAILURE));
    }

    clientAddr.sin_family = AF_INET;
    clientAddr.sin_port = 0; // Randomly select port
    clientAddr.sin_addr.s_addr = inet_addr("localhost");
    bzero(&(clientAddr.sin_zero),8);

    lengthClientAddr = sizeof(clientAddr);
    /* Bind socket with the client */
    if (::bind(clientSocketFD,(struct sockaddr *)&clientAddr,
        sizeof(struct sockaddr)) == -1)
    {
        global_logger_tem << ddfsLogger::LOG_WARNING << "UDP::Client :: Unable to bind socket. "
                        << strerror(errno) <<"\n";
        return (ddfsStatus(DDFS_FAILURE));
    }

    /* Setting the destination socket addr */
    destinationAddr.sin_family = AF_INET;
    destinationAddr.sin_addr.s_addr = inet_addr(remoteNodeHostName.c_str());
    destinationAddr.sin_port = htons(DDFS_SERVER_PORT);
    destinationAddrSize = sizeof(destinationAddr);
    
    if ( serverSocket != -1 ) {
        serverSocketFD = serverSocket;
        global_logger_tem << ddfsLogger::LOG_WARNING << "Server :: Server socket is already open.\n";
        return (ddfsStatus(DDFS_OK));
    }

	/* Open the server socket and wait for the client connections.
     * Server connection at each node opens at well defined port DDFS_SERVER_PORT.
     */
    if ((serverSocketFD = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1) {
        global_logger_tem << ddfsLogger::LOG_WARNING << "Server :: Unable to open socket.\n";
        close(clientSocketFD);
        return (ddfsStatus(DDFS_FAILURE));
    }

    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(DDFS_SERVER_PORT);
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    bzero(&(serverAddr.sin_zero),8);

    lengthServerAddr = sizeof(serverAddr);

    /* Bind socket with the server */
    if (::bind(serverSocketFD,(struct sockaddr *)&serverAddr, sizeof(struct sockaddr)) == -1)
    {
        global_logger_tem << ddfsLogger::LOG_WARNING << "UDP::Server :: Unable to bind socket. "
                        << strerror(errno) <<"\n";
        close(clientSocketFD);
        close(serverSocketFD);
        return (ddfsStatus(DDFS_FAILURE));
    }

    global_logger_tem << ddfsLogger::LOG_INFO << "Successfully created the server socket " <<
                    "to handle incoming messages";

    /* Copy the node IP to this instance */
    remoteNodeHostName = nodeUniqueID;

    /* Start the thread to handle the incoming traffic from
     * the remote node(remoteNodeHostName) in the cluster.
     */
    bkThreads.push_back(std::thread(&ddfsUdpConnection::bk_routine, (void *) this)); 

    global_logger_tem << ddfsLogger::LOG_INFO << "UDP::Both client and server connections opened."
                << strerror(errno) <<"\n";
    
	return (ddfsStatus(DDFS_OK));
    }

    ddfsStatus setupQueues(std::queue <requestQEntry *> *reqQueueBuffer,
                        std::queue <responseQEntry *> *rspQueueBuffer, void *privatePtr)
    {
    int i=0;
    requestQueue *reqQueue = new requestQueue();
    responseQueue<T_sub> *rspQueue = new responseQueue<T_sub>();

    reqQueue->dataBuffer = reqQueueBuffer;
    rspQueue->dataBuffer = rspQueueBuffer;

    queuesLock.lock();
    
    while(i<128) {
        if(requestQueues[i] == NULL)
            break;
        i++;
    }

    if(i == 128) {
        global_logger_tem << ddfsLogger::LOG_WARNING << "UDP:: BT : Max. number of req/rsp queues reached : 128."
                        << "\n";
        free(reqQueue);
        free(rspQueue);
        return ddfsStatus(DDFS_OK);
    }

    reqQueue->index = i;
    rspQueue->index = i;

    requestQueues[i] = reqQueue;
    responseQueues[i] = rspQueue;

    /* This pointer would be passed to us in the sendData */
    privatePtr = reqQueue;

    queuesLock.unlock();

    return ddfsStatus(DDFS_OK);
    }

	ddfsStatus sendData(void *data, int size, void *privatePtr)
    {
    int returnValue = 0;
    requestQEntry *entry = NULL;
    requestQueue *rQueueInstance = (requestQueue *) privatePtr;


    while(rQueueInstance->dataBuffer->empty() == false) {
        rQueueInstance->rLock.lock();
        entry = rQueueInstance->dataBuffer->front();
        rQueueInstance->dataBuffer->pop();
        rQueueInstance->rLock.unlock();
            
        /* Send the data to the other node */
        returnValue = sendto(clientSocketFD, entry->data,
                            entry->totalLength, 0,
                            (struct sockaddr *) &destinationAddr, destinationAddrSize);
        if(returnValue == -1) {
            global_logger_tem << ddfsLogger::LOG_INFO << "UDP::Send : Unable to send data."
                <<  entry->uniqueID
                << strerror(errno) << "\n";
            /* TODO : Push a failure entry in the response queue with this uniqueID */
            return (ddfsStatus(DDFS_FAILURE));
        }
    }

    free(entry);
    
	return (ddfsStatus(DDFS_OK));
    }

	ddfsStatus receiveData(void *des, int requestedSize,
				int *actualSize)
    {
    /* No need to implement this as data tranfer is done through
     * request and response queues. */
	return (ddfsStatus(DDFS_FAILURE));
    }

	ddfsStatus checkConnection()
    {
    /* Should create a test queue and send a special DDFS packet
     * to the server.
     *
     * Alternative to this is the heartbeat or maybe it could be
     * complementary to it.
     */
	return (ddfsStatus(DDFS_FAILURE));
    }

	ddfsStatus subscribe(T_sub* owner, void *privatePtr)
    {
    requestQEntry *entry = NULL;
    responseQueue<T_sub> *rspQInstance = NULL;
    requestQueue *reqQInstance = (requestQueue *) privatePtr;

    if (privatePtr == NULL) {
            global_logger_tem << ddfsLogger::LOG_INFO << "UDP::Send: Null privatePtr passed."
                <<  entry->uniqueID
                << strerror(errno) << "\n";
            return (ddfsStatus(DDFS_FAILURE));
    }

    rspQInstance = responseQueues[reqQInstance->index];

    rspQInstance->subscriptions.addSubscription(owner);

	return (ddfsStatus(DDFS_OK));
    }

	ddfsStatus closeConnection()
    {

	/* Terminate the background threads. */
    int size, i = 0;

    size = bkThreads.size();

    while(i < size) {
            terminateThreads[i++] = true;
    }

    /* Wait for the threads to terminate */
    while(i < size) {
            bkThreads[i].join();
    }

    /* Remove all the threads from the queue */
    while ( bkThreads.empty() == false )
            bkThreads.pop_back();

    /* Notify all the components that have subscription */
    i=0;
    while((i<128)) {
        if(responseQueues[i] == NULL)
            continue;

        /* Call all the subscription fn for this resposen queue. */
        responseQueues[i]->subscriptions.callSubscription(-1);
    }

    /* Wait for 10 seconds for the subscribed components to
     * perform internal cleanup.
     */
    sleep(10);

    /* Clean all the memory allocated for the req/rsp queues.
     * Don't free the dataBuffer as that is allocated by the
     * upper componenets.
     */
    i =0;
    while((i<128)) {
        if(responseQueues[i] == NULL)
            continue;

        /* Remove all the subscription fn for this resposen queue. */
        responseQueues[i]->subscriptions.removeAllSubscription();
    }

    /* Close the connections */
    close(serverSocketFD);
    serverSocketFD = -1;

    close(clientSocketFD);
    clientSocketFD = -1;

    remoteNodeHostName.replace(remoteNodeHostName.begin(),
                            remoteNodeHostName.end(), 1, '\0');

    destinationAddrSize = -1;

    bzero(tempBuffer, 4096);

	return (ddfsStatus(DDFS_OK));
    }

	ddfsStatus copyData(void *des, int requestedSize,
			int *actualSize)
    {
	return (ddfsStatus(DDFS_FAILURE));
    }

    ddfsStatus getServerSocket(int *retServerSocket)
    {
    *retServerSocket = serverSocketFD;
	return (ddfsStatus(DDFS_OK));
    }

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
    static void* bk_routine(void *arg)
    {
    ddfsUdpConnection *udpInstance = (ddfsUdpConnection *) arg;
	struct sockaddr_in serverAddr;
    int serverAddrLen = sizeof(struct sockaddr_in);
    struct hostent *hp;
    int returnLength = 0;
	int ret = 0;

	udpInstance->global_logger_tem << ddfsLogger::LOG_WARNING << "UDP:: Started the background thread. "
						<< strerror(errno) <<"\n";

    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(DDFS_SERVER_PORT);
    bzero(&(serverAddr.sin_zero),8);

    hp = gethostbyname(udpInstance->remoteNodeHostName.c_str());
    if(!hp) {
	    udpInstance->global_logger_tem << ddfsLogger::LOG_WARNING << "UDP:: BT : Fail to get IP address from Host Name. "
						<< strerror(errno) <<"\n";
        return 0;
    }

    memcpy((void *)&serverAddr.sin_addr, hp->h_addr_list[0], hp->h_length);

    /* Keep on listening to the port inorder to recieve data.
     */
	while(1) {
        /* Server port is already opened.
         * Start listening to the port.
         */
        returnLength = 0;
        ret = recvfrom(udpInstance->serverSocketFD, (void *) udpInstance->tempBuffer,
                        sizeof(ddfsClusterHeader), 0,
                        (struct sockaddr *) &serverAddr, (socklen_t *)&serverAddrLen);

        if(ret == -1) {
	        udpInstance->global_logger_tem << ddfsLogger::LOG_WARNING << "UDP:: BT : Fail to get IP address from Host Name. "
						<< strerror(errno) <<"\n";
            continue;
        }
 
        returnLength = ret;

        if(ret < sizeof(ddfsClusterHeader)) {
	        udpInstance->global_logger_tem << ddfsLogger::LOG_WARNING << "UDP:: BT : Data recieved is less that clusterheader from Host Name. "
						<< strerror(errno) <<"\n";
            continue;
        }

        /* Periodically keep on trying to connect to the set of nodes
         * that has been configured.
         */
        {
            udpInstance->global_logger_tem << ddfsLogger::LOG_WARNING
                << "UDP::Client :: Unable to connect socket. "
                << strerror(errno) <<"\n";
                continue;
        }
	} /* while loop end */

	pthread_exit(NULL);
    }

    /* Logger instance */
    ddfsLogger &global_logger_tem = ddfsLogger::getInstance();

private:
    int serverSocketFD;
    int clientSocketFD;
    string remoteNodeHostName;
    sockaddr_in destinationAddr;
    uint32_t destinationAddrSize;

    /* Request/Response Queues */
    std::mutex queuesLock;

    std::array <requestQueue *, 128> requestQueues;
    std::array <responseQueue <T_sub> *, 128> responseQueues;

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
    //class networkAnalysis;

    /* Just a temp buffer for the current incoming message */
    uint64_t tempBuffer[4096];
};

#endif /* Ending DDFS_UDPCONNECTION_H */
