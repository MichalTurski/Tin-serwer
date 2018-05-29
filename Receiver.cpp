//
// Created by michal on 28.05.18.
//

#include "Receiver.h"

Receiver::Receiver(int socDesc): socDesc(socDesc) {
    curr = nullptr;
    sesskey = nullptr;
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
