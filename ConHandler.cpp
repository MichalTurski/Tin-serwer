//
// Created by michal on 18.05.18.
//

#include <fstream>
#include <openssl/err.h>
#include "ConHandler.h"
#include "utils.h"
#include "packet.h"

ConHandler::ConHandler(std::string fileName): exitFlag(false) {
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
                        log(1, "Unable to register client");
                        exit (-1);
                    }
                    idClientPairs[id] = client;
                } else {
                    log(1, "Lack of public key file of device, aborting.");
                    exit (-1);
                }
            }
            return;
        } else {
            log(1, "Config file is empty!");
        }
    } else {
        log(1, "Unable to open configfile!");
    }
    exit (-1);
}
ConHandler::~ConHandler() {
    for (std::map<int, Client*>::iterator it = idClientPairs.begin(); it != idClientPairs.end(); ++it) {
        delete(it->second);
    }
    delete(server);
}
void ConHandler::handle(int desc, struct in_addr &cliAddr) {
    Client *client;
    std::map<uint32_t, Client*>::iterator ipIter;
    struct timeval tv;

    tv.tv_sec = 5;//TODO
    tv.tv_usec = 0;
    if (setsockopt(desc, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof tv) == 0) {
        std::shared_lock<std::shared_timed_mutex> sharedLock(addrClientMutex);
        //sharedLock.lock();
        ipIter = addrClientPairs.find(cliAddr.s_addr);
        if (ipIter != addrClientPairs.end()) {
            sharedLock.unlock();
            //Client have been verified, we can do regular job
            client = ipIter->second;
            //TODO
        } else if (!exitFlag) {
            sharedLock.unlock();
            registration(desc, cliAddr);
        } else {
            log(2, "Client tried to register, but server is exiting. Rejected.");
        }
    } else {
        log(3, "Failed when setting time limit on listening socket.");
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
                std::unique_lock<std::shared_timed_mutex> uniqueLock(addrClientMutex);
                addrClientPairs[cliAddr.s_addr] = client; //verification succeed, now we will be finding client by ip
                uniqueLock.unlock();
            }
        } else {
            log(2, "Client with unrecognized number %d tried to register", id->getId());
        }
    } else if (packet == nullptr) {} else{
        log(3, "Wrong packet type received, expected ID. Unable to verify client.");
    }
}
void ConHandler::dataExchange(int desc, Client *client) {

}
void ConHandler::setExit() {
    exitFlag = true;
}
void ConHandler::getReadyToExit(std::condition_variable **ready, std::mutex **readyM) {
    *ready = &readyToExit;
    *readyM = &readyToExitM;
}
bool ConHandler::clientsRegistered() {
    std::shared_lock<std::shared_timed_mutex> lock(addrClientMutex);
    return (!addrClientPairs.empty());
}

void conHandle(int id, ConHandler &conHandler, int desc, struct in_addr cliAddr) {
    conHandler.handle(desc, cliAddr);
}
