//
// Created by michal on 18.05.18.
//

#include <fstream>
#include <unistd.h>
#include <openssl/err.h>

#include "ConHandler.h"
#include "log.h"
#include "packet.h"
#include "Receiver.h"

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
#ifndef NO_MQ
                mqReceiver = std::thread(&Server::mqReceiveLoop, server);
#endif //NO_MQ
            } catch (...) {
                exit (-1);
            }
            while (configfile >> deviceId) {
                if (configfile >> devicePubkeyFile){
                    try {
                        id = std::stoi(deviceId);
                        client = new Client((uint8_t)id, devicePubkeyFile.c_str(), *this);
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
    for (auto it = idClientPairs.begin(); it != idClientPairs.end(); ++it) {
        delete(it->second);
    }
    delete(server);
#ifndef NO_MQ
    mqReceiver.join();
#endif //NO_MQ
}
void ConHandler::handle(int desc, struct in_addr &cliAddr) {
    Client *client;
    std::map<uint32_t, Client*>::iterator ipIter;
    Receiver receiver(desc);
    std::shared_lock<std::shared_timed_mutex> sharedLock(addrClientMutex);
    ipIter = addrClientPairs.find(cliAddr.s_addr);
    if (ipIter != addrClientPairs.end()) {
        sharedLock.unlock();
        //In our opinnion client have been verified, we can do regular job
        client = ipIter->second;
        if (!tryDataExchange(desc, client, receiver)) {
            log(2, "Data exchange with client number %d failed, he is trying to verify.", client->getId());
            addrClientPairs.erase(cliAddr.s_addr);//As client thinks he is not verified, we remove it from registered.
            registration(desc, cliAddr, receiver);
        }
    } else {
        sharedLock.unlock();
        receiver.nextPacket();
        registration(desc, cliAddr, receiver);
    }
    close(desc);
}
void ConHandler::registration(int sockDesc, struct in_addr cliAddr, Receiver &receiver) {
    Client *client;
    std::map<uint8_t , Client*>::iterator idIter;

    if (!exitFlag) {
        if (auto id = dynamic_cast<ID *> (receiver.getPacket())) {
            idIter = idClientPairs.find(id->getId());
            if (idIter != idClientPairs.end()) {
                client = idIter->second;
                if (client->initalize(sockDesc, *server, receiver)) {
                    std::unique_lock<std::shared_timed_mutex> uniqueLock(addrClientMutex);
                    addrClientPairs[cliAddr.s_addr] = client; //verification succeed, now we will be finding client by ip
                    uniqueLock.unlock();
                }
            } else {
                log(2, "Client with unrecognized number %d tried to register", id->getId());
            }
        } else {
            log(3, "Wrong packet type received, expected ID. Unable to verify client.");
        }
    } else {
        log(2, "Client tried to register, but server is exiting. Rejected.");
    }
}
bool ConHandler::tryDataExchange(int sockDesc, Client *client, Receiver &receiver) {
    bool exitFlagOld;
    if (client->tryDataExchange(sockDesc, (exitFlagOld = exitFlag), receiver)) {
        if (exitFlagOld) {
            log(1, "Succeed in disconnecting client number %d.", client->getId());
            if (!unregisterClient(client->getId()))
                log(3, "Failed to unregister client.");
            std::shared_lock<std::shared_timed_mutex> sharedLock(addrClientMutex);
            if (addrClientPairs.empty()){
                readyToExit.notify_one();
            }
        }
        return true;
    }
    return false;
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
bool ConHandler::unregisterClient(uint8_t id) {
    std::map<uint32_t , Client*>::iterator iter;
    Client *client = idClientPairs[id];
    std::shared_lock<std::shared_timed_mutex> sharedLock(addrClientMutex);
    for (iter = addrClientPairs.begin(); iter != addrClientPairs.end(); ++iter) {
        if (iter->second == client){
            sharedLock.unlock();
            std::unique_lock<std::shared_timed_mutex> uniqueLock(addrClientMutex);
            addrClientPairs.erase(iter->first);
            uniqueLock.unlock();
            client->unregisterServices(*server);
            return true;
        }
    }
    return false;
}
void ConHandler::roundRobinWalk() {
    Client *client;
    std::map<uint32_t, Client*>::iterator iter;
    std::shared_lock<std::shared_timed_mutex> sharedLock(addrClientMutex);
    for (iter = addrClientPairs.begin(); iter != addrClientPairs.end(); ++iter) {
        if (iter->second->getUsed()){
            iter->second->setUnused();
        } else {
            client = iter->second;
            log(1, "Client %d is not active, disconnecting it.", client->getId());
            sharedLock.unlock();
            std::unique_lock<std::shared_timed_mutex> uniqueLock(addrClientMutex);
            addrClientPairs.erase(iter->first);
            uniqueLock.unlock();
            client->unregisterServices(*server);
            sharedLock.lock();
        }
    }
}

void conHandle(int id, ConHandler &conHandler, int desc, struct in_addr cliAddr) {
    (void)id;
    conHandler.handle(desc, cliAddr);
}
