//
// Created by michal on 11.05.18.
//

#include <openssl/conf.h>
#include <openssl/err.h>
#include <openssl/rand.h>
#include "Server.h"
#include "packet.h"
#include "utils.h"

Server::Server(const char *file): privkey(file) {
    OPENSSL_config (nullptr);
    ERR_load_crypto_strings ();
    OpenSSL_add_all_ciphers();
    OpenSSL_add_all_algorithms();
    //CRYPTO_malloc_init();
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

bool Server::verifyServer(int sockDesc) const {
    unsigned char sign[256];
    unsigned int signLen;
    Packet *packet = Packet::packetFactory(sockDesc, nullptr);
    if (packet != nullptr) {
        if (CHALL *chall = dynamic_cast<CHALL *>(packet)) {
            privkey.sign(chall->getChall(), 8, sign, &signLen);
            CHALL_RESP *challResp = CHALL_RESP::createFromEncrypted(sign);
            if (challResp->send(sockDesc, nullptr) > 0) {
                delete challResp;
                return true;
            } else {
                delete challResp;
            }
        } else {
            log(3, "Wrong type of message, expected CHALL.\n");
        }
    } else {
        log(3, "Client has disconected.\n");
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
    return serviceTable.push(service, id);
}
bool Server::unregisterService(unsigned char id) {
    return serviceTable.remove(id);
}