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
Client::~Client() {
    for (auto&& i : digInputs)
        delete(i.second);
    for (auto&& i : analogInputs)
        delete(i.second);
    for (auto&& i : digOutputs)
        delete(i.second);
    for (auto&& i : analogOutputs)
        delete(i.second);
}
void Client::unregisterServices(Server &server) {
    for (auto&& i : digInputs) {
        server.unregisterService(i.second->getId());
        delete (i.second);
    }
    for (auto&& i : analogInputs) {
        server.unregisterService(i.second->getId());
        delete(i.second);
    }
    for (auto&& i : digOutputs) {
        server.unregisterService(i.second->getId());
        delete(i.second);
    }
    for (auto&& i : analogOutputs) {
        server.unregisterService(i.second->getId());
        delete(i.second);
    }
}

bool Client::initalize(int sockDesc, Server &server) {
    Packet *packet;

    if (verifyClient(sockDesc)) {
        if (server.verifyServer(sockDesc)) {
            Sesskey sesskey;
            unsigned char cipSesskey[256];
            pubkey.encrypt(sesskey.getKeyBuf(), 16, cipSesskey);
            KEY *sesskeyPck = KEY::createFromEncrypted(cipSesskey);
            if (sesskeyPck->send(sockDesc, nullptr) > 0) {
                delete(sesskeyPck);
                if (registerServices(sockDesc, server, sesskey)) {
                    log(1, "Registered client number %d.", id);
                    return true;
                }
            } else {
                delete(sesskeyPck);
            }
        }
    }
    log(2, "Failed to register client number %d", id);
    return (false);
}

bool Client::verifyClient(int sockDesc) const {
    unsigned char random[8];
    Packet *response;

    rng.generate(random, 8);
    CHALL *chall = CHALL::createFromRandom(random);
    if (chall->send(sockDesc, nullptr) > 0) {
        response = Packet::packetFactory(sockDesc, nullptr);
        if (CHALL_RESP *challResp = dynamic_cast<CHALL_RESP *> (response)) {
            if (pubkey.verify_sign(random, 8, challResp->getResp(), 256)) {
                delete chall;
                log(3, "Client number %d verified against server.", id);
                return true;

            } else {
                log(1, "Failed to verify challenge response, someone may be tying to do something nasty.");
            }
        } else {
            log(3, "Unexpected packet type, expected response to challenge.");
        }
    }
    delete chall;
    return false;
}
bool Client::registerServices(int sockDesc, Server &server, const Sesskey &sesskey) {
    Packet *packet;
    std::vector<Service*> services;
    Service *service;
    unsigned char id;
    packet = Packet::packetFactory(sockDesc, &sesskey);
    while (DESC *desc = dynamic_cast<DESC*> (packet)){
        std::cout << desc->getName() << std::endl;
        id = server.reserveId();
        if (id > 0) {
            service = Service::serviceFactory(id, desc->getDeviceClass(), desc->getName(),
                                              desc->getUnit(), desc->getMin(), desc->getMax());
            delete(packet);
            services.push_back(service);
            ACK ack(id);
            if (ack.send(sockDesc, &sesskey) > 0) {
                packet = Packet::packetFactory(sockDesc, &sesskey);
                if (dynamic_cast<EOT *> (packet)) {
                    delete (packet);
                    for (auto &&i : services) {
                        server.addService(i->getId(), i);
                        if (auto j = dynamic_cast<DigitalIn*> (i))
                            digInputs.insert(std::make_pair(j->getId(), j));
                        if (auto&& j = dynamic_cast<AnalogIn*> (i))
                            analogInputs.insert(std::make_pair(j->getId(), j));
                        if (auto&& j = dynamic_cast<DigitalOut*> (i))
                            digOutputs.insert(std::make_pair(j->getId(), j));
                        if (auto&& j = dynamic_cast<AnalogOut*> (i))
                            analogOutputs.insert(std::make_pair(j->getId(), j));
                    }
                    return (true);
                }
            }
        } else {
            delete(packet);
            log(1, "Limit of services number have been reached.");
            break;
        }
    }
    NAK nak((unsigned char)0);
    nak.send(sockDesc, &sesskey);
    for (auto&& i : services) {
        server.unreserveId(i->getId());
        delete (i);
    }
    delete (packet);
    return (false);
}
