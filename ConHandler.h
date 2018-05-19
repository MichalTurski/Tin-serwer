//
// Created by michal on 18.05.18.
//

#ifndef SERWER_CONHANDLER_H
#define SERWER_CONHANDLER_H


#include <map>
#include <netinet/in.h>

#include "client.h"

class ConHandler {
private:
    std::map<int, Client*> idClientPairs;
    std::map<uint32_t, Client*> addrClientPairs;//TODO: mutex to protect this structure
    Server *server;

    void registration(int desc, struct in_addr cliAddr);
    void dataExchange(int desc, Client *client);
public:
    ConHandler(std::string fileName);
    ~ConHandler();
    void handle(int desc, struct in_addr cliAddr);
};


#endif //SERWER_CONHANDLER_H
