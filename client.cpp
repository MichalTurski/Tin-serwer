//
// Created by michal on 11.05.18.
//

#include <iostream>
#include "client.h"
#include "packet.h"
#include "RNG.h"
#include "utils.h"

RNG rng;

Client::Client(const char *pubkey): pubkey(pubkey), verified(false) {}

void Client::initalize(int sockDesc, Server &server) {
    Packet *packet;

    if (verifyClient(sockDesc)){
        if (server.verifyServer(sockDesc)) {
            Sesskey sesskey;
            unsigned char cipSesskey[256];
            pubkey.encrypt(sesskey.getKeyBuf(), 16, cipSesskey);
            KEY sesskeyPck(cipSesskey);
            if (sesskeyPck.send(sockDesc, nullptr) > 0) {
                do {
                    packet = Packet::packetFactory(sockDesc, &sesskey);
                    if (DESC *desc = dynamic_cast<DESC*> (packet)){
                        std::cout << desc->getName() << std::endl;
                    } else {
                        break;
                    }

                } while(packet);
                return;
            }
        }
    }
    log(2, "Failed to verify client\n");
    return;
}

bool Client::verifyClient(int sockDesc) {
    unsigned char random[8];
    Packet *response;

    rng.generate(random, 8);
    CHALL *chall = CHALL::createFromRandom(random);
    if (chall->send(sockDesc, nullptr) > 0) {
        response = Packet::packetFactory(sockDesc, nullptr);
        if (CHALL_RESP *challResp = dynamic_cast<CHALL_RESP *> (response)) {
            if (pubkey.verify_resp(challResp->getResp(), 256, random, 8)) {
                verified = true;
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
