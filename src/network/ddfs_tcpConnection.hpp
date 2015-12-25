/*
 * @file ddfs_tcpConnection.h
 *
 * @brief Module for managing network communication.
 *
 * This is the module that would be responsible for network
 * data management.
 *
 * Author Harman Patial <harman.patial@gmail.com>
 */

#ifndef DDFS_TCPCONNECTION_H
#define DDFS_TCPCONNECTION_H

#include <iostream>
#include <vector>
#include <array>
#include <queue>
#include <thread>
#include <strings.h>
#include <string>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <cstring>

using namespace std;

#include "ddfs_network.hpp"
//#include "../cluster/ddfs_clusterMessagesPaxos.h"
#include "../logger/ddfs_fileLogger.hpp"
#include "../global/ddfs_status.hpp"

#define DDFS_SERVER_PORT    53327
#define MAX_CLUSTER_NODES    4
#define MAX_TCP_CONNECTIONS    MAX_CLUSTER_NODES

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

    void callSubscription(void *data, int size) {
        for(int i=0; i < subscribedInstances.size(); i++){
            subscribedInstances[i]->callback(data, size);
        }
    }
};

template <typename T_sub>
class responseQueue {
public:
    std::queue <responseQEntry> dataBuffer;
    std::mutex rLock;
    int responseQIndex;
    int correspondingRequestQIndex;
    bool in_use;

    /* Subscription is bound to response queues.
     * Each component should create it's req-rsp queue
     * when they want to transfer data with this particular
     * node.
     */
    ddfsSubscriptionClass <T_sub> subscriptions;
};

struct request {
    void *data;
    int size;
};

class requestQueue {
public:
    std::queue <struct request *> pipe;
    std::mutex rLock;
    bool in_use;
    int requestQIndex;
    int correspondingResponseQIndex;
};

enum DDFS_NETWORK_TYPE {
    DDFS_NETWORK_TCP,
    DDFS_NETWORK_UDP,
    DDFS_NETWORK_FC,
    DDFS_NETWORK_ISCSI
};

template <typename T_sub>
class ddfsTcpConnection : public Network<string, T_sub, DDFS_NETWORK_TYPE>{
private:
    static std::mutex openConnectionLock;
    static vector<pair<int, struct sockaddr_in> > openConnections;

public:
    ddfsTcpConnection() {}
    ~ddfsTcpConnection() {}

    void printBuffer(void *data, int size, string printMessage) {
        global_logger_tem << ddfsLogger::LOG_INFO << printMessage << ": \n";
        string tempPrintBuffer;
        int i=0;
        uint8_t *printData = (uint8_t *) data;

        while(i < size) {
            if((size-i) > 16) {
                for(int j=0; j < 16; j++, i++) {
                    tempPrintBuffer.append(to_string(*(printData+i)));
                    tempPrintBuffer.append("  ");
                }
            }
            else {
                int newLength = size-i;
                for(int j=0; j < newLength; j++, i++) {
                    tempPrintBuffer.append(to_string(*(printData+i)));
                    tempPrintBuffer.append("  ");
                }
            }
            global_logger_tem << ddfsLogger::LOG_INFO << tempPrintBuffer << "\n";
            tempPrintBuffer.clear();
        }
    }

    ddfsStatus init() {
        serverSocketFD = -1;
        //clientSocketFD = -1;

        isNodeLocal = false;

        /* Pre allocating memory for storing the header */
        bzero(tempBuffer, g_temp_buffer_size); 
        this->setNetworkType(DDFS_NETWORK_TCP);
        return (ddfsStatus(DDFS_OK));
    }

    bool isConnectionOpen() {
        vector<pair<int, struct sockaddr_in> >::iterator iter;
        bool found = false;

        if(isNodeLocal == true) {
            global_logger_tem << ddfsLogger::LOG_INFO << "TCP: isConnectionOpen: This should not be called for the local Node.\n";
            return false;
        }

        if(serverSocketFD != -1)
            return true;

        openConnectionLock.lock();
        if(serverSocketFD == -1) {
            for (iter = openConnections.begin(); iter != openConnections.end(); ) {
                global_logger_tem << ddfsLogger::LOG_INFO << "first : " << iter->first << ". second : " << inet_ntoa(iter->second.sin_addr) << "\n";
                string connHostName(inet_ntoa(iter->second.sin_addr));
                if(!connHostName.compare(remoteNodeHostName)) {
                    serverSocketFD = iter->first;
                    openConnections.erase(iter);
                    found = true;
                    global_logger_tem << ddfsLogger::LOG_INFO <<
                            "Found the connection with " << remoteNodeHostName << ". socket : " << serverSocketFD << "\n";
                    break;
                } else
                    iter++;
           }
        }

        openConnectionLock.unlock();
 
       return found;
    }

    ddfsStatus openConnection(string nodeUniqueID, bool doNotConnect)
    {
        struct sockaddr_in serverAddr;

        /* Copy the node IP to this instance */
        remoteNodeHostName = nodeUniqueID;
        std::string localhost("localhost");

        global_logger_tem << ddfsLogger::LOG_INFO << "TCPCONNECTION :: Hostname : " << remoteNodeHostName << "\n";

        if(localhost.compare(remoteNodeHostName)) {
            global_logger_tem << ddfsLogger::LOG_INFO << "This tcpConnection Instance is for a remote node.\n";

            if(doNotConnect == false) {
                global_logger_tem << ddfsLogger::LOG_INFO << "Open a remote connection. \n";

                /* Setting the destination socket addr */
                destinationAddr.sin_family = AF_INET;

                destinationAddr.sin_addr.s_addr = inet_addr(remoteNodeHostName.c_str());
                destinationAddr.sin_port = htons(DDFS_SERVER_PORT);
                destinationAddrSize = sizeof(destinationAddr);

                /*  Open the server socket and wait for the client connections.
                 * Server connection at each node opens at well defined port DDFS_SERVER_PORT.
                 */
                if ((serverSocketFD = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
                    global_logger_tem << ddfsLogger::LOG_WARNING << "Server :: Unable to open socket.\n";
                    //close(clientSocketFD);
                    return (ddfsStatus(DDFS_FAILURE));
                }

                if(connect(serverSocketFD, (struct sockaddr *) &destinationAddr, sizeof(destinationAddr)) == -1) {
                    global_logger_tem << ddfsLogger::LOG_WARNING << "Server :: Unable to connect to socket."
                                    << strerror(errno) << "\n";
                    //close(clientSocketFD);
                    return (ddfsStatus(DDFS_FAILURE));
                }

                global_logger_tem << ddfsLogger::LOG_INFO << "Connected to node :" << remoteNodeHostName << "\n";
            }
        } else { /* This tcpConnection Instance is for local port */
            global_logger_tem << ddfsLogger::LOG_INFO << "This tcpConnection Instance is for a local node. "
                        << "Open a server port. \n";
            global_logger_tem << ddfsLogger::LOG_INFO << "localhost : Setting the local port.\n";

            isNodeLocal = true;

            //lengthServerAddr = sizeof(serverAddr);
            if ((serverSocketFD = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
                global_logger_tem << ddfsLogger::LOG_WARNING << "Server :: Unable to open socket.\n";
                //close(clientSocketFD);
                return (ddfsStatus(DDFS_FAILURE));
            }

            memset(&serverAddr, 0, sizeof(serverAddr));
            serverAddr.sin_family = AF_INET;
            serverAddr.sin_addr.s_addr = INADDR_ANY;
            serverAddr.sin_port = htons(DDFS_SERVER_PORT);
            bzero(&(serverAddr.sin_zero),8);

            /* Bind socket with the server */
            if (::bind(serverSocketFD,(struct sockaddr *)&serverAddr, sizeof(struct sockaddr)) == -1)
            {
                global_logger_tem << ddfsLogger::LOG_WARNING << "TCP::Server :: Unable to bind socket. "
                                << strerror(errno) <<"\n";
                close(serverSocketFD);
                return (ddfsStatus(DDFS_FAILURE));
            }

            global_logger_tem << ddfsLogger::LOG_INFO << "Successfully created the server socket " <<
                            "to handle incoming messages\n";

        }

        /* Start the thread to handle the incoming traffic from
         * the remote node(remoteNodeHostName) in the cluster.
         */
        global_logger_tem << ddfsLogger::LOG_INFO << "About to create a thread.\n";
        bkThreads = std::thread(&ddfsTcpConnection::bk_routine, this);
        //std::thread t1(ddfsTcpConnection::bk_routine, (void *) this);
        global_logger_tem << ddfsLogger::LOG_INFO << "TCP:: Server port opened.\n";

        return (ddfsStatus(DDFS_OK));
    }

    // The default behavior is one request and one response queue.
    // TODO : Should allow creation of multiple request/response queues.
    ddfsStatus setupPortal(void **privatePtr)
    {
        int i=0;

        /*  For local Node, no need to setup request/response queues */
        if(isNodeLocal == true) {
            return (ddfsStatus(DDFS_OK));
        }

        queuesLock.lock();
    
        while(i<g_max_req_queues) {
            if((requestQueues[i].in_use == false) || (responseQueues[i].in_use == false))
                break;
            i++;
        }

        if(i == g_max_req_queues) {
            global_logger_tem << ddfsLogger::LOG_WARNING << "TCP(" << remoteNodeHostName << "): Max. number of req/rsp queues reached : " << 
                            g_max_req_queues << ".\n";
            return ddfsStatus(DDFS_FAILURE);
        }

        global_logger_tem << ddfsLogger::LOG_WARNING << "TCP(" << remoteNodeHostName << "): Req/Respose Queue Set : Index : " << i << "\n";

        requestQueues[i].in_use = true;
        responseQueues[i].in_use = true;

        requestQueues[i].correspondingResponseQIndex = i;
        queuesLock.unlock();

        responseQueueIndex = i;

        global_logger_tem << ddfsLogger::LOG_WARNING << "TCP(" << remoteNodeHostName << "): Response Queue Index is " << responseQueueIndex << "\n";

        /* This pointer would be passed to us in the sendData */
        *privatePtr = &(requestQueues[i]);

        global_logger_tem << ddfsLogger::LOG_INFO
                << "network :: setupPortal.\n";

        isConnectionOpen();

        return ddfsStatus(DDFS_OK);
    }

    /* Only put data on queue, don't actually send it  */
    /* privatePtr : This is the pointer to the request queue where
     *              data needs to be placed.
     */
    ddfsStatus sendData(void *data, int size, void *privatePtr)
    {
        int returnValue = 0;
        requestQueue *rQueueInstance = (requestQueue *) privatePtr;

        global_logger_tem << ddfsLogger::LOG_INFO
                << "network :: sendData.\n";

        if(rQueueInstance->in_use == false)
            return ddfsStatus(DDFS_FAILURE);

        if(isConnectionOpen() == false)
            return (ddfsStatus(DDFS_FAILURE));

        struct request * entry = new request;
        //ddfsClusterHeader *clusterH = (ddfsClusterHeader *) data;

        entry->data = data;
        entry->size = size;

        rQueueInstance->pipe.push(entry);

        while(rQueueInstance->pipe.empty() == false) {
            rQueueInstance->rLock.lock();
            entry = rQueueInstance->pipe.front();
            rQueueInstance->pipe.pop();
            rQueueInstance->rLock.unlock();
            
            printBuffer(entry->data, entry->size, "TCP::Send: Network Packet: ");

            /* Send the data to the other node */
            returnValue = sendto(serverSocketFD, entry->data,
                                entry->size, 0,
                                (struct sockaddr *) &destinationAddr, destinationAddrSize);
            if(returnValue == -1) {
                global_logger_tem << ddfsLogger::LOG_INFO << "TCP::Send : Unable to send data."
                    << strerror(errno) << "\n";
                return (ddfsStatus(DDFS_FAILURE));
            } else {
                break;
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
        /*
         * Alternative to this is the heartbeat or maybe it could be
         * complementary to it.
         */
        if(isConnectionOpen() == false)
            return (ddfsStatus(DDFS_FAILURE));

        return (ddfsStatus(DDFS_OK));
    }

    ddfsStatus subscribe(T_sub* owner, void *privatePtr)
    {
        responseQueue<T_sub> *rspQInstance = NULL;
        requestQueue *reqQInstance = (requestQueue *) privatePtr;

        if (privatePtr == NULL) {
            global_logger_tem << ddfsLogger::LOG_INFO << "TCP::Subscribe: Null privatePtr passed.\n";
            return (ddfsStatus(DDFS_FAILURE));
        }

        rspQInstance = &responseQueues[reqQInstance->correspondingResponseQIndex];
        global_logger_tem << ddfsLogger::LOG_INFO << "TCP(" << remoteNodeHostName << "): subscribing with response Q : " << reqQInstance->correspondingResponseQIndex << "\n";

        rspQInstance->subscriptions.addSubscription(owner);

        return (ddfsStatus(DDFS_OK));
    }

    ddfsStatus closeConnection()
    {
        /* Terminate the background threads. */
        int i = 0;

        if(isNodeLocal == true) {
            bkThreads.join();
            return (ddfsStatus(DDFS_OK));
        } else {

            /* Notify all the components that have subscription */
            i=0;
            while((i<g_max_req_queues)) {
                if(responseQueues[i].in_use == false)
                    continue;

                /* Call all the subscription fn for this resposen queue. */
                //responseQueues[i]->subscriptions.callSubscription(-1);
            }

            /* Wait for 10 seconds for the subscribed components to
            * perform internal cleanup.
            */
            sleep(10);

            /* Clean all the memory allocated for the req/rsp queues.
             * Don't free the dataBuffer as that is allocated by the
             * upper componenets.
             */
            i=0;
            while((i<ddfsTcpConnection::g_max_rsp_queues)) {
                if(responseQueues[i].in_use == false)
                continue;

                /* Remove all the subscription fn for this resposen queue. */
                responseQueues[i].subscriptions.removeAllSubscription();
            }
        }

        /* Close the connections */
        close(serverSocketFD);
        serverSocketFD = -1;

        //close(clientSocketFD);
        //clientSocketFD = -1;

        remoteNodeHostName.replace(remoteNodeHostName.begin(),
                                remoteNodeHostName.end(), 1, '\0');

        destinationAddrSize = -1;

        bzero(tempBuffer, g_temp_buffer_size);
        tempBuffer[0] = 'H';
        tempBuffer[1] = 'A';
        tempBuffer[2] = 'R';
        tempBuffer[3] = 'M';
        tempBuffer[4] = 'A';
        tempBuffer[5] = 'N';

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
    void bk_routine()
    {
        //ddfsTcpConnection *tcpInstance = (ddfsTcpConnection *) arg;
        struct sockaddr_in clientAddr;
        int serverAddrLen = sizeof(struct sockaddr_in);
        std::string localhost("localhost");
        int ret = 0;

        global_logger_tem << ddfsLogger::LOG_WARNING << "TCP:: Started the background thread.\n";

        if(!localhost.compare(remoteNodeHostName)) {  /* Thread for local Port */
            global_logger_tem << ddfsLogger::LOG_INFO << "TCP(" << remoteNodeHostName << "):: Background thread for localNode.\n";
            while(1) {
                socklen_t clilen;

                global_logger_tem << ddfsLogger::LOG_INFO << "TCP(" << remoteNodeHostName << "):: BT : Listening for a connection.\n";
                ret = listen(serverSocketFD,5);
                if (ret == -1) {
                    global_logger_tem << ddfsLogger::LOG_INFO << "ERROR on listen. error " << strerror(errno) << "\n";
                    continue;
                }

                int newsockfd = accept(serverSocketFD, (struct sockaddr *) &clientAddr, &clilen);
                if (newsockfd < 0) { 
                    global_logger_tem << ddfsLogger::LOG_INFO << "ERROR on accept. error " << strerror(errno) << "\n";
                    continue;
                }

                global_logger_tem << ddfsLogger::LOG_INFO << "Accepted a new connection from "
                               << inet_ntoa(clientAddr.sin_addr) << " and port " << clientAddr.sin_port << "\n";

                /* This newsockfd should be passed to the appropriate ddfsTcpConnection object.
                 * Parse the clientAddr and then decide on which ddfsTcpConnection to pass this socket to.
                 */

                openConnectionLock.lock();
                openConnections.push_back(make_pair(newsockfd, clientAddr));
                openConnectionLock.unlock();

                continue;
            }
        } else { /* Thread for remoteNode. */
            bool connectionEstablishedRightNow = false;
            while(1) {
                if(isConnectionOpen() == false) {
                    global_logger_tem << ddfsLogger::LOG_WARNING << " TCP(" << remoteNodeHostName << "):: BT : Connection to " << remoteNodeHostName << " is not up yet !!. "
                                    << "Sleeping for 2 second.\n";
                    sleep(2);
                    connectionEstablishedRightNow = false;
                    continue;
                }

                if(connectionEstablishedRightNow == false) {
                    global_logger_tem << ddfsLogger::LOG_INFO << "TCP(" << remoteNodeHostName << "):: BT : Connection established with " << remoteNodeHostName << "\n";
                    connectionEstablishedRightNow = true;
                }
                global_logger_tem << ddfsLogger::LOG_INFO << "TCP(" << remoteNodeHostName << "):: BT: Temp Buffer before recieve data.\n";
                
                for (int i = 0; i < 6; i++) {
                        global_logger_tem << ddfsLogger::LOG_INFO << "    " << tempBuffer[i] << "\n";
                }

                ret = recvfrom(serverSocketFD, (void *) tempBuffer,
                                sizeof(ddfsClusterHeader), MSG_PEEK,
                                (struct sockaddr *) &clientAddr, (socklen_t *)&serverAddrLen);

                if(ret == -1) {
                    global_logger_tem << ddfsLogger::LOG_WARNING << "TCP(" << remoteNodeHostName << "):: BT : Fail to get IP address from Host Name. "
                                << strerror(errno) <<"\n";
                    continue;
                } else if(ret == 0) {
                    global_logger_tem << ddfsLogger::LOG_WARNING << "TCP(" << remoteNodeHostName << "):: BT : Connection closed by pair. "
                                << strerror(errno) <<"\n";
                    continue;
                }
                global_logger_tem << ddfsLogger::LOG_INFO << "TCP:: BT : " << remoteNodeHostName << ": Recieved data of size " << ret << "\n";
#if 0 
                global_logger_tem << ddfsLogger::LOG_INFO << "TCP:: BT : " << remoteNodeHostName << ": Temp Buffer : ";
                for (int i = 0; i < ret; i++) {
                        global_logger_tem << ddfsLogger::LOG_INFO << "i = " << i << "       ** ";
                        global_logger_tem << ddfsLogger::LOG_INFO << "    " << tempBuffer[i] << "\n";
                }
#endif
                printBuffer(tempBuffer, ret, "TCP:: BT: Recieved Network Packet: ");

                if(ret < sizeof(ddfsClusterHeader)) {
                    global_logger_tem << ddfsLogger::LOG_WARNING << "TCP(" << remoteNodeHostName << "):: BT : Data recieved is less that clusterheader from Host Name.\n";
                    continue;
                }

                requestQEntry *newRequest = (requestQEntry *) tempBuffer;
                ddfsClusterHeader *clusterH = (ddfsClusterHeader *) newRequest->data;

                global_logger_tem << ddfsLogger::LOG_INFO << "TCP(" << remoteNodeHostName << "):: BT: v: " << clusterH->version << ". tOS: " << clusterH->typeOfService << "\n";
                global_logger_tem << ddfsLogger::LOG_INFO << "TCP(" << remoteNodeHostName << "):: BT: tl:" << clusterH->totalLength << ".ID: " << clusterH->uniqueID << "\n";
                global_logger_tem << ddfsLogger::LOG_INFO << "TCP(" << remoteNodeHostName << "):: BT: sizeof(ddfsClusterHeader) : " << sizeof(ddfsClusterHeader) << "\n";

                /* We alloc it here and it would be deallocated by whoever is the consumer of
                 * this response entry */
                /* totalLengthOfTheMessage -- Total Length of the message including the header  */
                int totalLengthOfTheMessage = clusterH->totalLength;
                void *totalMessage = malloc(totalLengthOfTheMessage);
                memcpy(totalMessage, tempBuffer, sizeof(ddfsClusterHeader));

                /* Parse the ddfsHeader and know how much data is expected with
                 * this ddfs Message. Alloc memory and 
                 *
                 * Place the ddfsHeader as well as the data on the response queue.
                 */
                if((totalLengthOfTheMessage - sizeof(ddfsClusterHeader)) > 0) {
                    ret = recvfrom(serverSocketFD, (void *) (((uint8_t *) totalMessage) + sizeof(ddfsClusterHeader)),
                                    (totalLengthOfTheMessage - sizeof(ddfsClusterHeader)), 0,
                                    (struct sockaddr *) &clientAddr, (socklen_t *)&serverAddrLen);

                    if(ret == -1) {
                        global_logger_tem << ddfsLogger::LOG_WARNING << "TCP(" << remoteNodeHostName << "):: BT : Fail to get IP address from Host Name. "
                                << strerror(errno) <<"\n";
                        continue;
                    }
                }
#if 0
            /* Find for which instance of tcp connection this data is for */
            typename vector <ddfsTcpConnection <T_sub> *>::iterator iter;
            ddfsTcpConnection<T_sub> *rightTCPConnection = NULL;

            for (iter = allNetworkConnections.begin(); iter != allNetworkConnections.end(); iter++) {

                if(!strncpy(inet_ntoa(serverAddr.sin_addr), (*iter)->remoteNodeHostName.c_str(), sizeof(serverAddr.sin_addr.s_addr)))
                    rightTCPConnection = (*iter);
            }
#endif

                responseQEntry newEntry;

                newEntry.typeOfService = clusterH->typeOfService;
                newEntry.totalLength = clusterH->totalLength;
                newEntry.data = totalMessage;

                global_logger_tem << ddfsLogger::LOG_INFO << "TCP(" << remoteNodeHostName << "):: BT : Placing data on Response Queue no. " << responseQueueIndex << "\n";
                global_logger_tem << ddfsLogger::LOG_INFO << "TCP(" << remoteNodeHostName << "):: BT : in use = " << responseQueues[responseQueueIndex].in_use << "\n";

                responseQueues[responseQueueIndex].rLock.lock();
                responseQueues[responseQueueIndex].dataBuffer.push(newEntry);
                global_logger_tem << ddfsLogger::LOG_INFO << "TCP(" << remoteNodeHostName << "):: BT : Placed the data. Calling subscribers.\n";
                responseQueues[responseQueueIndex].subscriptions.callSubscription(&newEntry.data, newEntry.totalLength);
                responseQueues[responseQueueIndex].rLock.unlock();

                global_logger_tem << ddfsLogger::LOG_INFO << "TCP(" << remoteNodeHostName << "):: BT : End of while loop. \n";

            } /* while loop end */
        } /* else loop end */

        global_logger_tem << ddfsLogger::LOG_WARNING << "TCP(" << remoteNodeHostName << "):: BT : Exiting the receiver thread.\n";

        pthread_exit(NULL);
    }  /* End of bk_routine() */

    //vector<ddfsTcpConnection<T_sub> *> allNetworkConnections;

private:
    int serverSocketFD;
    //int clientSocketFD;
    string remoteNodeHostName;
    sockaddr_in destinationAddr;
    uint32_t destinationAddrSize;
    int responseQueueIndex;

    /* Request/Response Queues */
    std::mutex queuesLock;

    static const int g_max_req_queues = 128;
    static const int g_max_rsp_queues = 128;
    array <requestQueue, g_max_req_queues> requestQueues;
    array <responseQueue <T_sub>, g_max_rsp_queues> responseQueues;

    /* There is no need for keeping outstanding command list
     * If we are unable to send to data due to send/recieve
     * errors, upper layer is responsible for retrying/recovering
     * of the data.
     */
    //std::queue <requestQEntry> outstandingMessage;
    std::mutex outMessageLock;

    /* Vector thread list */
    std::thread bkThreads;

    std::vector<bool> terminateThreads;

    /* Logger instance */
    ddfsLogger &global_logger_tem = ddfsLogger::getInstance();

    /* TODO: Following fields are only for analysis */
    //class networkAnalysis;
    bool isNodeLocal;

    /* Temporary buffer for the current incoming message */
    static const int g_temp_buffer_size = 4096;
    uint8_t tempBuffer[g_temp_buffer_size];
};

template <typename T_sub>
std::mutex ddfsTcpConnection<T_sub>::openConnectionLock; 

template <typename T_sub>
vector<pair<int, struct sockaddr_in> > ddfsTcpConnection<T_sub>::openConnections;

#endif /* Ending DDFS_TCPCONNECTION_H */
