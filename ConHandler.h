//
// Created by michal on 18.05.18.
//

#ifndef SERWER_CONHANDLER_H
#define SERWER_CONHANDLER_H


#include <map>
#include <netinet/in.h>
#include <atomic>

#include "client.h"

class ConHandler {
private:
    std::map<int, Client*> idClientPairs;
    std::map<uint32_t, Client*> addrClientPairs;
    mutable std::shared_timed_mutex addrClientMutex;
    Server *server;
    std::atomic<bool> exitFlag;
    std::condition_variable readyToExit;
    std::mutex readyToExitM;

    void registration(int desc, struct in_addr cliAddr);
    void dataExchange(int desc, Client *client);
public:
    ConHandler(std::string fileName);
    ~ConHandler();
    void getReadyToExit(std::condition_variable **ready, std::mutex **readyM);
    void handle(int desc, struct in_addr &cliAddr);
    void setExit();
    bool clientsRegistered();
};

/*
 * This function exists only for calling ConHandler::handle() form ctpl::push()
 * It should be done with lambda, but there are problems with it.
 */
void conHandle(int id, ConHandler &conHandler, int desc, struct in_addr cliAddr);

#endif //SERWER_CONHANDLER_H
