//
// Created by michal on 11.05.18.
//

#include <openssl/conf.h>
#include <openssl/err.h>
#include <openssl/rand.h>

#include "Server.h"
#include "packet.h"
#include "log.h"
#include "common.h"

Server::Server(const char *file): privkey(file)
#ifndef NO_MQ
                                  ,sendMsgQueue("/queueFromGonzo", O_WRONLY),
                                  readMsgQueue("/queueToGonzo", O_WRONLY)
#endif// NO_MQ
{
    OPENSSL_config (nullptr);
    ERR_load_crypto_strings ();
//    mqReceiver = std::thread(mqReceiveLoop, this);
//    OpenSSL_add_all_ciphers();
//    OpenSSL_add_all_algorithms();
//    CRYPTO_malloc_init();
}
/*Server::Server() {
    OPENSSL_config (nullptr);
    ERR_load_crypto_strings ();
    OpenSSL_add_all_ciphers();
    OpenSSL_add_all_algorithms();
    CRYPTO_malloc_init();
}*/
Server::~Server() {

//    ERR_free_strings ();
//    RAND_cleanup ();
//    EVP_cleanup ();
//    CONF_modules_free ();
//    ERR_remove_state (0);
}

/*static void Server::mqReceiveStatic(void *serverPtr) {
    Server *server = static_cast<Server*>(serverPtr);
    server->mqReceiveLoop();
}
void Server::initComunication() {
    mqReceiver = std::thread(&Server::mqReceiveStatic, this);
}*/

#ifndef NO_MQ
void Server::mqReceiveLoop() {
    bool working = true;
    Packet *packet;
    while (working) {
        packet = readMsgQueue.readToPacket();
        if (auto get = dynamic_cast<GET*> (packet)) {
            const Service *service = serviceTable.getService(get->getId());
            if (auto digIn = dynamic_cast<const DigitalIn*> (service)) {
                VAL val(digIn->getId(), digIn->getVal(), digIn->getTimestamp());
                val.setPacketID(PCK_Q_VAL);
                sendMsgQueue.addMessage(&val);
            } else if (auto anIn = dynamic_cast<const AnalogIn*> (service)) {
                VAL val(anIn->getId(), anIn->getVal(), anIn->getTimestamp());
                val.setPacketID(PCK_Q_VAL);
                sendMsgQueue.addMessage(&val);
            } else if (auto digOut = dynamic_cast<const DigitalOut*> (service)) {
                uint32_t ugly_hack = 0;
                VAL val(digOut->getId(), digOut->getVal(), (time_t)ugly_hack);
                val.setPacketID(PCK_Q_VAL);
                sendMsgQueue.addMessage(&val);
            } else if (auto anOut = dynamic_cast<const AnalogOut*> (service)) {
                uint32_t ugly_hack = 0;
                VAL val(anOut->getId(), anOut->getVal(), (time_t)ugly_hack);
                val.setPacketID(PCK_Q_VAL);
                sendMsgQueue.addMessage(&val);
            } else {
                log (3, "Second server part requested value of non-registered service %d.", get->getId());
            }
        } else if (auto set = dynamic_cast<SET*> (packet)) {
            Service *service = serviceTable.getService(get->getId());
            if (auto digOut = dynamic_cast<DigitalOut*> (service)) {
                digOut->setVal((bool) set->getValue());
            } else if (auto anOut = dynamic_cast<AnalogOut*> (service)) {
                anOut->setVal(set->getValue());
            } else if (set == nullptr) {
                log (3, "Second server part tried to set value of non-registered service %d.", get->getId());
            } else {
                log (3, "Second server part tried to set value of input service %d.", get->getId());
            }
        } else if (packet == nullptr) {
            if (errno == EBADF) {
                log(3, "Message queue have been closed.");
                working = false;
            } else {
                log(3, "Reading from message queue returned with %s.", strerror(errno));
            }
        }
    }
}
#endif //NO_MQ

bool Server::verifyServer(int sockDesc, Receiver &receiver) const {
    unsigned char sign[256];
    unsigned int signLen;
    Packet *packet = receiver.nextPacket();
    if (packet != nullptr) {
        if (CHALL *chall = dynamic_cast<CHALL *>(packet)) {
            privkey.sign(chall->getChall(), 8, sign, &signLen);
            CHALL_RESP *challResp = CHALL_RESP::createFromEncrypted(sign);
            if (challResp->send(sockDesc, nullptr) > 0) {
                delete challResp;
                log(3, "Server verification against client succeed.");
                return true;
            } else {
                delete challResp;
            }
        } else {
            log(3, "Wrong type of message, expected CHALL.");
        }
    } else {
        log(3, "Client has disconected.");
    }
    return false;
}

unsigned char Server::reserveId() {
    return serviceTable.reserve();
}
bool Server::unreserveId(unsigned char id) {
    return serviceTable.unreserve(id);
}
bool Server::addService(unsigned char id, Service *service) {
    bool result;
    result = serviceTable.push(service, id);
#ifndef NO_MQ
    DESC desc(service->getName(), service->getUnit(), service->getMin(), service->getMax());
    desc.setPacketID(PCK_Q_DESC);
    sendMsgQueue.addMessage(&desc);
#endif //NO_MQ
    return result;
}
bool Server::unregisterService(unsigned char id) {
    bool result;
    result = serviceTable.remove(id);
#ifndef NO_MQ
    EXIT exit(id);
    exit.setPacketID(PCK_Q_EXIT);
    sendMsgQueue.addMessage(&exit);
#endif //NO_MQ
    return result;
}