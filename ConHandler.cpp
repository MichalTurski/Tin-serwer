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

    if (configfile.is_open()){
        configfile >> privkeyFile;
        if (! privkeyFile.empty()){
            try {
                server = new Server(privkeyFile.c_str());
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
    delete(server);
}
void ConHandler::handle(int desc, struct in_addr cliAddr) {
    Client *client;
    std::map<uint32_t, Client*>::iterator ipIter;
    struct timeval tv;

    tv.tv_sec = 5;//TODO
    tv.tv_usec = 0;
    if (setsockopt(desc, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof tv) == 0) {
        ipIter = addrClientPairs.find(cliAddr.s_addr);
        if (ipIter != addrClientPairs.end()) {
            //Client have been verified, we can do regular job
            client = ipIter->second;
            //TODO
        } else {
            registration(desc, cliAddr);
        }
    } else {
        log(3, "Failed when setting time limit on listening socket.\n");
    }
}
void ConHandler::registration(int desc, struct in_addr cliAddr) {
    Client *client;
    std::map<int, Client*>::iterator idIter;

    Packet *packet = Packet::packetFactory(desc, nullptr);
    if (ID *id = dynamic_cast<ID*> (packet)) {
        idIter = idClientPairs.find(id->getId());
        if (idIter != idClientPairs.end()) {
            client = idIter->second;
            if (client->initalize(desc, *server)) {
                addrClientPairs[cliAddr.s_addr] = client; //verification succeed, now we will be finding client by ip
            }
        } else {
            log(2, "Client with unrecognized number %d tried to register\n", id->getId());
        }
    } else if (packet == nullptr) {} else{
        log(3, "Wrong packet type received, expected ID. Unable to verify client.\n");
    }
}
void ConHandler::dataExchange(int desc, Client *client) {

}
