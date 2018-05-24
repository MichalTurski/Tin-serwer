//
// Created by michal on 11.05.18.
//

#ifndef SERWER_CLIENTMOCK_H
#define SERWER_CLIENTMOCK_H


#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <zconf.h>
#include <iostream>
#include "RNG.h"
#include "privkey.h"
#include "pubkey.h"
#include "packet.h"
#include "utils.h"

bool verifyAgainstServer(int sock, Privkey &privkey) {
    unsigned char sign[256];
    unsigned int signLen;
    Packet *packet = Packet::packetFactory(sock, nullptr);
    if (packet != nullptr) {
        if (CHALL *chall = dynamic_cast<CHALL *>(packet)) {
            if (privkey.sign(chall->getChall(), 8, sign, &signLen) > 0) {
                delete (packet);
                CHALL_RESP *challResp = CHALL_RESP::createFromEncrypted(sign);
                if (challResp->send(sock, nullptr) > 0) {
                    delete (challResp);
                    return true;
                } else {
                    delete (challResp);
                }
            }
        } else {
            delete(packet);
            log(1, "Wrong type of message\n");
        }
    }
    return false;
}

bool verifyAgainstClient(int sock, Pubkey &pubkey) {
    RNG rng;
    Packet *response;
    unsigned char random[8];

    rng.generate(random, 8);
    CHALL *chall = CHALL::createFromRandom(random);
    if (chall->send(sock, nullptr) > 0) {
        response = Packet::packetFactory(sock, nullptr);
        if (CHALL_RESP *challResp = dynamic_cast<CHALL_RESP *> (response)) {
            if (pubkey.verify_sign( random, 8, challResp->getResp(), 256)) {
                delete(chall);
                return true;

            } else {
                log(1, "Failed to verify response");
            }
        } else {
            log(2, "Unexpected packet type\n");
        }
    }
    delete(chall);
    return false;
}


void clientMock() {
    int cliSock;
    struct sockaddr_in dstAddr;
    Packet *packet;
    std::string name = "Nazwa";
    std::string unit = "szt";

    dstAddr.sin_family = AF_INET;
    dstAddr.sin_port = htons(12345);
    dstAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
    cliSock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    if (connect(cliSock, (struct sockaddr*) &dstAddr, sizeof(dstAddr)) < 0) {
        std::cout <<"blad: conect"<<std::endl;
        return ;
    }
    Pubkey pubkey("server-public.pem");
    Privkey privkey("client-private.pem");

    ID id(1);
    id.send(cliSock, nullptr);
    if (verifyAgainstServer(cliSock, privkey)) {
        if (verifyAgainstClient(cliSock, pubkey)) {
            packet = Packet::packetFactory(cliSock, nullptr);
            if (KEY *keyPck = dynamic_cast<KEY *> (packet)) {
                Sesskey sesskey(*keyPck, privkey);
                delete(keyPck);
                for(int i = 0; i<5; i++) {
                    DESC desc(name, unit, 1.2, 1.3);
                    desc.send(cliSock, &sesskey);
                    packet = Packet::packetFactory(cliSock, &sesskey);
                    if(ACK *ack = dynamic_cast<ACK*> (packet)){
                        std::cout << "Service id = "<< (int) ack->getId() << std::endl;
                    }
                }
                EOT eot;
                eot.send(cliSock, &sesskey);
                return;
            } else {
                delete (packet);
            }
        }
    }
    std::cout <<"ClientMock::verifiacion failed\n";
}

#endif //SERWER_CLIENTMOCK_H
