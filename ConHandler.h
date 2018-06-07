//
// Created by michal on 18.05.18.
//

#ifndef SERWER_CONHANDLER_H
#define SERWER_CONHANDLER_H


#include <map>
#include <netinet/in.h>
#include <atomic>

#include "client.h"

class Client;

class ConHandler {
private:
    std::map<uint8_t, Client*> idClientPairs;
    std::map<uint32_t, Client*> addrClientPairs;
    mutable std::shared_timed_mutex addrClientMutex;
    Server *server;
    std::atomic<bool> exitFlag;
    std::condition_variable readyToExit;
    std::mutex readyToExitM;
#ifndef NO_MQ
    std::thread mqReceiver;
#endif //NO_MQ

    void registration(int sockDesc, struct in_addr cliAddr, Receiver &receiver);
    bool tryDataExchange(int sockDesc, Client *client, Receiver &receiver);
public:
    explicit ConHandler(const std::string &configfileName, const std::string &inMQ,
                        const std::string & outMQ);
    ~ConHandler();
    void getReadyToExit(std::condition_variable **ready, std::mutex **readyM);
    void handle(int desc, struct in_addr &cliAddr);
    void setExit();
    bool clientsRegistered();
    bool unregisterClient(uint8_t id);
    void clockWalk();
};

/*
 * This function exists only for calling ConHandler::handle() form ctpl::push()
 * It should be done with lambda, but there are problems with it.
 */
void conHandle(int id, ConHandler &conHandler, int desc, struct in_addr cliAddr);

#endif //SERWER_CONHANDLER_H
