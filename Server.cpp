//
// Created by michal on 11.05.18.
//

#include <openssl/conf.h>
#include <openssl/err.h>
#include <openssl/rand.h>
#include <csignal>

#include "Server.h"
#include "packet.h"
#include "log.h"
#include "common.h"
#include "queuePacket.h"

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
    QueuePacket *inPacket;
    time_t mTime;
    while (working) {
        inPacket = QueuePacket::packetFromQueue(&readMsgQueue);
        //todo: mutex dla obrony przed destruktorem servera
        if (auto get = dynamic_cast<Q_GET*> (inPacket)) {
            if (get->getId() == 0) {
                //TODO
            } else {
                const Service *service = serviceTable.getService(get->getId());
                if (auto input = dynamic_cast<const Input *> (service)) {
                    Q_VAL val(input->getId(), input->getVal(), input->getTimestamp());
                    val.addToQueue(&sendMsgQueue);
                } else if (auto output = dynamic_cast<const Output *> (service)) {
                    time(&mTime);
                    Q_VAL val(output->getId(), output->getVal(), mTime);
                    val.addToQueue(&sendMsgQueue);
                } else {
                    log(3, "Second server part requested value of non-registered service %d.", get->getId());
                }
            }
        } else if (auto set = dynamic_cast<Q_SET*> (inPacket)) {
            Service *service = serviceTable.getService(get->getId());
            if (auto out = dynamic_cast<Output*> (service)) {
                out->setVal(set->getValue());
            } else if (set == nullptr) {
                log (3, "Second server part tried to set value of non-registered service %d.", get->getId());
            } else {
                log (3, "Second server part tried to set value of input service %d.", get->getId());
            }
        } else if (inPacket == nullptr) {
            if (errno == EBADF) {
                log(3, "Message queue have been closed.");
                raise(SIGINT);
                working = false;
            } else {
                log(3, "Reading from message queue returned with %s.", strerror(errno));
            }
        }
        delete (inPacket);
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
    Q_DESC desc(service->getId(), service->getType(), service->getName(), service->getUnit(),
                service->getMin(), service->getMax());
    desc.addToQueue(&sendMsgQueue);
#endif //NO_MQ
    return result;
}
bool Server::unregisterService(unsigned char id) {
    bool result;
    result = serviceTable.remove(id);
#ifndef NO_MQ
    Q_EXIT exit(id);
    exit.addToQueue(&sendMsgQueue);
#endif //NO_MQ
    return result;
}