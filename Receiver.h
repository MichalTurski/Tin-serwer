//
// Created by michal on 28.05.18.
//

#ifndef SERWER_RECEIVER_H
#define SERWER_RECEIVER_H


#include "packet.h"

class Receiver {
    const int socDesc;
    const Sesskey *sesskey;
    Packet *curr;
public:
    Receiver(int sockDesc);
    ~Receiver();
    void addSesskey(const Sesskey *sesskey);
    Packet *getPacket();
    Packet *nextPacket();
};


#endif //SERWER_RECEIVER_H
