//
// Created by michal on 18.05.18.
//

#include <fstream>
#include <openssl/err.h>
#include "ConHandler.h"
#include "utils.h"
#include "packet.h"

ConHandler::ConHandler(std::string fileName) {
    std::ifstream configfile(fileName);
    std::string privkeyFile;
    std::string deviceId, devicePubkeyFile;
    int id;
    Client *client;

//    OpenSSL_add_all_algorithms();
//    OpenSSL_add_all_ciphers();
//    ERR_load_crypto_strings();

    if (configfile.is_open()){
        configfile >> privkeyFile;
        if (! privkeyFile.empty()){
            try {
                server = Server(privkeyFile.c_str());
            } catch (...) {
                exit (-1);
            }
            while (configfile >> deviceId) {
                if (configfile >> devicePubkeyFile){
                    try {
                        id = std::stoi(deviceId);
                        client = new Client(id, devicePubkeyFile.c_str());
                    } catch (...) {
                        log(1, "Unable to register client\n");
                        exit (-1);
                    }
                    idClientPairs[id] = client;
                } else {
                    log(1, "Lack of public key file of device, aborting.\n");
                    exit (-1);
                }
            }
            return;
        } else {
            log(1, "Config file is empty!\n");
        }
    } else {
        log(1, "Unable to open configfile!\n");
    }
    exit (-1);
}
ConHandler::~ConHandler() {
    for (std::map<int, Client*>::iterator it = idClientPairs.begin(); it != idClientPairs.end(); ++it) {
        delete(it->second);
    }
}
void ConHandler::handle(int desc, struct in_addr cliAddr) {
    Client *client;
    std::map<uint32_t, Client*>::iterator ipIter;
    std::map<int, Client*>::iterator idIter;

    ipIter = addrClientPairs.find(cliAddr.s_addr);
    if (ipIter != addrClientPairs.end()) {
        //Client have been verified, we can do regular job
        client = ipIter->second;
        //TODO
    } else {
        // Client must be verified
        Packet *packet = Packet::packetFactory(desc, nullptr);
        if (ID *id = dynamic_cast<ID*> (packet)) {
            idIter = idClientPairs.find(id->getId());
            if (idIter != idClientPairs.end()) {
                client = idIter->second;
                if (client->initalize(desc, server)) {
                    log(1, "Client number %d verified successful", id->getId());
                    addrClientPairs[cliAddr.s_addr] = client; //verification succeed, now we will be finding client by ip
                } else {
                    log(2, "Failed to verify client\n");
                }
            } else {
                log(2, "Wrong client number\n");
            }
        } else if (packet == nullptr) {} else{
            log(2, "Wrong packet received, unable to verify client\n");
        }
    }
}
