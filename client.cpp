//
// Created by michal on 11.05.18.
//

#include <iostream>
#include "client.h"
#include "packet.h"
#include "RNG.h"
#include "utils.h"

RNG rng;

Client::Client(int id, const char *pubkey, ConHandler &conHandler): id(id), pubkey(pubkey), conHandler(conHandler) {}
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
            if (pubkey.encrypt(sesskey.getKeyBuf(), 16, cipSesskey)) {
                KEY *sesskeyPck = KEY::createFromEncrypted(cipSesskey);
                if (sesskeyPck->send(sockDesc, nullptr) > 0) {
                    delete (sesskeyPck);
                    if (registerServices(sockDesc, server, sesskey)) {
                        log(1, "Registered client number %d.", id);
                        return true;
                    }
                } else {
                    delete (sesskeyPck);
                }
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
uint8_t Client::getId() const {
    return id;
}
bool Client::tryDataExchange(int sockDesc, bool end, Packet **unused) {
    Sesskey sesskey;
    unsigned char cipSesskey[256];
    *unused = nullptr;
    if (pubkey.encrypt(sesskey.getKeyBuf(), 16, cipSesskey)) {
        KEY *sesskeyPck = KEY::createFromEncrypted(cipSesskey);
        if (sesskeyPck->send(sockDesc, nullptr) > 0) {
            if (getValues(sockDesc, &sesskey, unused)){
                if (end){
                    if (setExit(sockDesc, &sesskey)){
                        log(2, "Data exchange with client %d succeed.", id);
                        return true;
                    }
                } else {
                    if (setValues(sockDesc, &sesskey)) {
                        log(2, "Data exchange with client %d succeed.", id);
                        return true;
                    }
                }
            } else {
                if (tryUnregister(sockDesc, &sesskey, unused)){
                    log(2, "Data exchange with client %d succeed.", id);
                    return true;
                }
            }
        }
    }
    return false;
}
bool Client::tryUnregister(int sockDesc, Sesskey *sesskey, Packet **unused) {
    Packet *packet;

    if (dynamic_cast<EXIT*> (*unused)){
        delete(*unused);
        *unused = nullptr;
        ACK ack((unsigned char) 0);
        if (ack.send(sockDesc, sesskey)) {
            packet = Packet::packetFactory(sockDesc, sesskey);
            if (dynamic_cast<EOT*> (packet)){
                log(1, "Client &d requested exiting, unregistering it.", id);
                conHandler.unregisterClient(id);
                return true;
            } else {
                log(3, "Client &d requested exiting, responded ACK, expected EOT, but not received.", id);
            }
        }
    }
    return false;
}
bool Client::getValues(int sockDesc, Sesskey *sesskey, Packet **unused) {
    Packet *packet;
    std::map<unsigned char, DigitalIn*>::iterator digInIter;
    std::map<unsigned char, AnalogIn*>::iterator anInIter;

    packet = Packet::packetFactory(sockDesc, sesskey);
    if (dynamic_cast<VAL*> (packet) || dynamic_cast<EOT*> (packet)) {
        while (!(dynamic_cast<EOT *> (packet))) {
            if (auto val = dynamic_cast<VAL *> (packet)) {
                if ((digInIter = digInputs.find(val->getServiceId())) != digInputs.end()) {
                    digInIter->second->setVal(val->getValue());
                    digInIter->second->setTimestamp(val->getTimestamp());
                } else if ((anInIter = analogInputs.find(val->getServiceId())) != analogInputs.end()) {
                    anInIter->second->setVal(val->getValue());
                    anInIter->second->setTimestamp(val->getTimestamp());
                } else {
                    delete (packet);
                    log(2, "Client %d sent value of service %d which is not its input", id, val->getServiceId());
                    return false;
                }
                delete (packet);
                packet = Packet::packetFactory(sockDesc, sesskey);
            } else {
                delete (packet);
                log(3, "Wrong packet type received during data exchange with client %d, expected VAl or EOT", id);
                return false;
            }
        }
        log(2, "Receiving values from client %d succeed.",id);
        delete(packet);
        return true;
    } else {
        *unused = packet;
    }
    return false;
}
bool Client::setValues(int sockDesc, Sesskey *sesskey) {
    Packet *packet;
    float val;

    for (auto &&i :digOutputs) {
        if (i.second->beginSetting(&val)) {
            SET set(i.second->getId(), val);
            if (!set.send(sockDesc, sesskey)) {
                return false;
            }
            packet = Packet::packetFactory(sockDesc, sesskey);
            if (!dynamic_cast<ACK *> (packet)) {
                if (packet != nullptr || !dynamic_cast<NAK *> (packet)) {
                    log(3, "Received wrong message in response to SET, expected ACK or NAK.");
                }
                free(packet);
                return false;
            }
            free(packet);
        }
    }
    for (auto &&i :analogOutputs) {
        if (i.second->beginSetting(&val)) {
            SET set(i.second->getId(), val);
            if (!set.send(sockDesc, sesskey)) {
                return false;
            }
            packet = Packet::packetFactory(sockDesc, sesskey);
            if (!dynamic_cast<ACK *> (packet)) {
                if (packet != nullptr || !dynamic_cast<NAK *> (packet)) {
                    log(3, "Received wrong message in response to SET, expected ACK or NAK.");
                }
                free(packet);
                return false;
            }
            free(packet);
        }
    }
    EOT eot;
    if (!eot.send(sockDesc, sesskey)) {
        return false;
    }
    log(2, "Setting values of client %d succeed.", id);
    for (auto &&i :digOutputs) {
        i.second->finalizeSetting();
    }
    for (auto &&i :analogOutputs) {
        i.second->finalizeSetting();
    }
    return true;
}
bool Client::setExit(int sockDesc, Sesskey *sesskey) {
    Packet *packet;

    EXIT exit((unsigned char) 0);
    if (!exit.send(sockDesc, sesskey)) {
        return false;
    }
    packet = Packet::packetFactory(sockDesc, sesskey);
    if (!dynamic_cast<ACK *> (packet)) {
        delete (packet);
        log(3, "Received wrong message in response to EXIT, expected ACK.");
        return false;
    }
    delete (packet);
    EOT eot;
    if (!eot.send(sockDesc, sesskey)) {
        return false;
    }
    return true;
}
