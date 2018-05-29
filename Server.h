//
// Created by michal on 11.05.18.
//

#ifndef SERWER_SERVER_H
#define SERWER_SERVER_H

#include "privkey.h"
#include "ServiceTable.h"
#include "Receiver.h"

class Server {
private:
    Privkey privkey;
    ServiceTable serviceTable;
public:
    Server(const char *file);
//    Server();
    ~Server();
//    void loadPrivkey(const char file);
    bool verifyServer(int sockDesc, Receiver &receiver) const;
    unsigned char reserveId();
    bool unreserveId(unsigned char id);
    bool addService(unsigned char id, Service *service);
    bool unregisterService(unsigned char id);
};


#endif //SERWER_SERVER_H
