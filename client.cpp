//
// Created by michal on 11.05.18.
//

#include <iostream>
#include "client.h"
#include "packet.h"
#include "RNG.h"
#include "utils.h"

RNG rng;

Client::Client(int id, const char *pubkey): id(id), pubkey(pubkey) {}

bool Client::initalize(int sockDesc, const Server &server) {
    Packet *packet;

    if (verifyClient(sockDesc)){
        if (server.verifyServer(sockDesc)) {
            Sesskey sesskey;
            unsigned char cipSesskey[256];
            pubkey.encrypt(sesskey.getKeyBuf(), 16, cipSesskey);
            KEY *sesskeyPck = KEY::createFromEncrypted(cipSesskey);
            if (sesskeyPck->send(sockDesc, nullptr) > 0) {
                delete(sesskeyPck);
                packet = Packet::packetFactory(sockDesc, &sesskey);
                while (DESC *desc = dynamic_cast<DESC*> (packet)){
                    std::cout << desc->getName() << std::endl;
                    delete(packet);
                    packet  = Packet::packetFactory(sockDesc, &sesskey);
                    if (dynamic_cast<EOT*> (packet)){
                        std::cout << "all descriptors received\n";
                        delete(packet);
                        return (true);
                    }
                }
            }
            delete(sesskeyPck);
        }
    }
    log(2, "Failed to verify client\n");
    return (false);
}

bool Client::verifyClient(int sockDesc) const {
    unsigned char random[8];
    Packet *response;

    rng.generate(random, 8);
    /*random[0] = 'a';
    random[1] = 'l';
    random[2] = 'a';*/
    CHALL *chall = CHALL::createFromRandom(random);
    if (chall->send(sockDesc, nullptr) > 0) {
        response = Packet::packetFactory(sockDesc, nullptr);
        if (CHALL_RESP *challResp = dynamic_cast<CHALL_RESP *> (response)) {
            if (pubkey.verify_resp(challResp->getResp(), 256, random, 8)) {
                delete chall;
                return true;

            } else {
                log(1, "Failed to verify response");
            }
        } else {
            log(2, "Unexpected packet type\n");
        }
    }
    delete chall;
    return false;
}
