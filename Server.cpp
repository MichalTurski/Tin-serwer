//
// Created by michal on 11.05.18.
//

#include "Server.h"
#include "packet.h"
#include "utils.h"

Server::Server(const char *file): privkey(file) {}

bool Server::verifyServer(int sockDesc) const {
    unsigned char ciphered[256];
    Packet *packet = Packet::packetFactory(sockDesc, nullptr);
    if (packet != nullptr) {
        if (CHALL *chall = dynamic_cast<CHALL *>(packet)) {
            privkey.encrypt(chall->getChall(), 8, ciphered);
            CHALL_RESP *challResp = CHALL_RESP::createFromEncrypted(ciphered);
            if (challResp->send(sockDesc, nullptr) > 0) {
                delete challResp;
                return true;
            } else {
                delete challResp;
            }
        } else {
            log(1, "Wrong type of message\n");
        }
    } else {
        log(2, "Client has disconected\n");
    }
    return false;
}
