//
// Created by michal on 28.05.18.
//

#include <sys/socket.h>
#include "Receiver.h"

Receiver::Receiver(int socDesc): socDesc(socDesc) {
    curr = nullptr;
    sesskey = nullptr;
    struct timeval tv;

    tv.tv_sec = 5;
    tv.tv_usec = 0;
    setsockopt(socDesc, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof tv);
}

Receiver::~Receiver() {
    delete curr;
}

void Receiver::addSesskey(const Sesskey *sesskey) {
    this->sesskey = sesskey;
}

Packet* Receiver::getPacket() {
    return curr;
}

Packet* Receiver::nextPacket() {
    delete curr;
    curr = Packet::packetFactory(socDesc, sesskey);
    return curr;
}
