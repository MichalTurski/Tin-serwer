//
// Created by michal on 11.05.18.
//

#ifndef SERWER_SERVER_H
#define SERWER_SERVER_H

#include <thread>
#include "privkey.h"
#include "ServiceTable.h"
#include "Receiver.h"
#include "AddQueue.h"
#include "ReadQueue.h"

class Server {
private:
    Privkey privkey;
    ServiceTable serviceTable;
    std::thread mqReceiver;
#ifndef NO_MQ
    AddQueue sendMsgQueue;
    ReadQueue readMsgQueue;
#endif //NO_MQ
public:
    explicit Server(const std::string &privkeyFile,const std::string &inMQ,
                       const std::string &outMQ);
    ~Server();
#ifndef NO_MQ
    void mqReceiveLoop();
#endif //NO_MQ
    bool verifyServer(int sockDesc, Receiver &receiver) const;
    unsigned char reserveId();
    bool unreserveId(unsigned char id);
    bool addService(unsigned char id, Service *service);
    bool unregisterService(unsigned char id);
};


#endif //SERWER_SERVER_H
