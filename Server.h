//
// Created by michal on 11.05.18.
//

#ifndef SERWER_SERVER_H
#define SERWER_SERVER_H


#include "privkey.h"

class Server {
private:
    Privkey privkey;
public:
    Server(const char *file);
    bool verifyServer(int sockDesc);
};


#endif //SERWER_SERVER_H
