//
// Created by michal on 11.05.18.
//

#include "client.h"
#include "packet.h"
#include "RNG.h"
#include "log.h"

RNG rng;

Client::Client(uint8_t id, const char *pubkey, ConHandler &conHandler): id(id), pubkey(pubkey),
                                                                        conHandler(conHandler) {}
Client::~Client() {
    for (auto&& i : inputs)
        delete(i.second);
    for (auto&& i : outputs)
        delete(i.second);
}
void Client::unregisterServices(Server &server) {
    std::unique_lock<std::mutex> lock(mutex);
    for (auto&& i : inputs) {
        server.unregisterService(i.second->getId());
        delete(i.second);
        inputs.erase(i.first);
    }
    for (auto&& i : outputs) {
        server.unregisterService(i.second->getId());
        delete(i.second);
        outputs.erase(i.first);
    }
}

bool Client::initalize(int sockDesc, Server &server, Receiver &receiver) {
    std::unique_lock<std::mutex> lock(mutex);
    if (verifyClient(sockDesc, receiver)) {
        if (server.verifyServer(sockDesc, receiver)) {
            Sesskey sesskey;
            unsigned char cipSesskey[256];
            if (pubkey.encrypt(sesskey.getKeyBuf(), 16, cipSesskey)) {
                KEY *sesskeyPck = KEY::createFromEncrypted(cipSesskey);
                if (sesskeyPck->send(sockDesc, nullptr) > 0) {
                    receiver.addSesskey(&sesskey);
                    delete (sesskeyPck);
                    if (registerServices(sockDesc, server, sesskey, receiver)) {
                        used = true;
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

bool Client::verifyClient(int sockDesc, Receiver &receiver) const {
    unsigned char random[8];
    Packet *response;

    rng.generate(random, 8);
    CHALL *chall = CHALL::createFromRandom(random);
    if (chall->send(sockDesc, nullptr) > 0) {
        response = receiver.nextPacket();
        if (auto challResp = dynamic_cast<CHALL_RESP *> (response)) {
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
bool Client::registerServices(int sockDesc, Server &server, const Sesskey &sesskey, Receiver &receiver) {
    Packet *packet;
    std::vector<Service*> services;
    Service *service;
    unsigned char id;
    packet = receiver.nextPacket();
    while (auto desc = dynamic_cast<DESC*> (packet)) {
        id = server.reserveId();
        if (id > 0) {
            service = Service::serviceFactory(id, desc->getDeviceClass(), desc->getName(),
                                              desc->getUnit(), desc->getMin(), desc->getMax());
            services.push_back(service);
            ACK ack(id);
            if (ack.send(sockDesc, &sesskey) > 0) {
                packet = receiver.nextPacket();
                if (dynamic_cast<EOT *> (packet)) {
                    for (auto i : services) {
                        server.addService(i->getId(), i);
                        if (auto&& j = dynamic_cast<Input*> (i))
                            inputs.insert(std::make_pair(j->getId(), j));
                        if (auto&& j = dynamic_cast<Output*> (i))
                            outputs.insert(std::make_pair(j->getId(), j));
                    }
                    return (true);
                }
            } else {
                return (false); //unable to connect with client, exiting
            }
        } else {
            log(1, "Limit of services number have been reached.");
            break;
        }
    }
    log(3, "Registration of services of client %d failed. Sending NAK.", this->id);
    NAK nak((unsigned char)0);
    nak.send(sockDesc, &sesskey);
    for (auto&& i : services) {
        server.unreserveId(i->getId());
        delete (i);
    }
    return (false);
}
uint8_t Client::getId() const {
    return id;
}
bool Client::tryDataExchange(int sockDesc, bool end, Receiver &receiver) {
    Sesskey sesskey;
    unsigned char cipSesskey[256];

    std::unique_lock<std::mutex> lock(mutex);
    if (pubkey.encrypt(sesskey.getKeyBuf(), 16, cipSesskey)) {
        KEY *sesskeyPck = KEY::createFromEncrypted(cipSesskey);
        if (sesskeyPck->send(sockDesc, nullptr) > 0) {
            receiver.addSesskey(&sesskey);
            if (getValues(sockDesc, &sesskey, receiver)) {
                if (end){
                    if (setExit(sockDesc, &sesskey)){
                        log(2, "Data exchange with client %d succeed.", id);
                        used = true;
                        return true;
                    }
                } else {
                    if (setValues(sockDesc, &sesskey, receiver)) {
                        log(2, "Data exchange with client %d succeed.", id);
                        used = true;
                        return true;
                    }
                }
            } else {
                lock.unlock(); //try_unregister() calls unregisterServices() which takes this mutex
                if (tryUnregister(receiver)){
                    log(2, "Data exchange with client %d succeed.", id);
                    used = true;
                    return true;
                }
            }
        }
    }
    return false;
}
bool Client::tryUnregister(Receiver &receiver) {
    std::unique_lock<std::mutex> lock(mutex);
    if (dynamic_cast<EXIT*> (receiver.getPacket())) {
        log(1, "Client %d requested exiting, unregistering it.", id);
        //ConHandler::unregisterClient() calls unregisterServices() which takes this mutex
        lock.unlock();
        conHandler.unregisterClient(id);
        return true;
    }
    return false;
}
bool Client::getValues(int sockDesc, Sesskey *sesskey, Receiver &receiver) {
    Packet *packet;
    std::map<unsigned char, Input*>::iterator inputIter;
    bool succes = true;

    packet = receiver.nextPacket();
    if (dynamic_cast<VAL*> (packet) || dynamic_cast<EOT*> (packet)) {
        while (!(dynamic_cast<EOT *> (packet))) {
            if (auto val = dynamic_cast<VAL *> (packet)) {
                if ((inputIter = inputs.find(val->getServiceId())) != inputs.end()) {
                    inputIter->second->setVal(val->getValue());
                    inputIter->second->setTimestamp(val->getTimestamp());
                    log(4, "Received input measurement [%f, %u] from client %d.",
                        val->getValue(), val->getTimestamp(), id);
                } else {
                    succes = false;
                    log(2, "Client %d sent value of service %d which is not its input", id, val->getServiceId());
                }
                packet = receiver.nextPacket();
            } else {
                log(3, "Wrong packet type received during data exchange with client %d, expected VAl or EOT", id);
                return false;
            }
        }
        if (succes) {
            log(2, "Receiving values from client %d succeed.", id);
        } else {
            log(2, "There were some incorrect values form client %d", id);
        }
        return true;
    }
    return false;
}
bool Client::setValues(int sockDesc, Sesskey *sesskey, Receiver &receiver) {
    Packet *packet;
    float val;

    for (auto &&i :outputs) {
        if (i.second->beginSetting(&val)) {
            SET set(i.second->getId(), val);
            if (!set.send(sockDesc, sesskey)) {
                return false;
            }
            packet = receiver.nextPacket();
            if (!dynamic_cast<ACK *> (packet)) {
                if (dynamic_cast<NAK *> (packet)) {
                    EOT eot;
                    eot.send(sockDesc, sesskey);//don't care about success, closing socket anyway.
                } else if (packet != nullptr) {
                    log(3, "Received wrong message in response to SET, expected ACK or NAK.");
                }
                return false;
            }
        }
    }
    EOT eot;
    if (!eot.send(sockDesc, sesskey)) {
        return false;
    }
    log(2, "Setting values of client %d succeed.", id);
    for (auto &&i :outputs) {
        i.second->finalizeSetting();
    }
    return true;
}
bool Client::setExit(int sockDesc, Sesskey *sesskey) {
    EXIT exit((unsigned char) 0);
    return (exit.send(sockDesc, sesskey) > 0);
}
bool Client::getUsed() {
    std::unique_lock<std::mutex> lock(mutex);
    return used;
}
void Client::setUnused() {
    std::unique_lock<std::mutex> lock(mutex);
    used = false;
}