#include <iostream>
#include <netinet/in.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <zconf.h>
#include <fstream>
#include <thread>
#include <csignal>

#include "RNG.h"
#include "packet.h"
#include "privkey.h"
#include "Server.h"
#include "client.h"
#include "ClientMock.h"
#include "ConHandler.h"
#include "CTPL/ctpl_stl.h"

int initSocket() {
    struct sockaddr_in srvAddr;
    int listenSock;

    srvAddr.sin_family = AF_INET;
    srvAddr.sin_port = htons(12345);
    srvAddr.sin_addr.s_addr = INADDR_ANY;

    listenSock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (bind(listenSock, (struct sockaddr*) &srvAddr, sizeof(srvAddr)) < 0) {
        std::cout <<"blad: bind"<<std::endl;
        return -1;
    }
    if (listen(listenSock, 10) < 0) {
        std::cout <<"blad: listen"<<std::endl;
        return -1;
    }
    return listenSock;
}

void sighandler(int signo) {
    log(1, "Received signal %d, exiting.", signo);
}

void terminationHandler(sigset_t &sigset, ConHandler &conHandler, std::atomic<bool> &end, int sockfd) {
    //struct sigaction sa;
    int signo;
    std::condition_variable *readyToExit;
    std::mutex *readyToExitM;
    conHandler.getReadyToExit(&readyToExit, &readyToExitM);
    sigwait(&sigset, &signo);
    log(1, "Received signal %s, trying do disconet from all clients.", strsignal(signo));
    conHandler.setExit();
    if (conHandler.clientsRegistered()) {
        std::unique_lock<std::mutex> lock(*readyToExitM);
        if(readyToExit->wait_for(lock, std::chrono::seconds(15)) == std::cv_status::timeout) {
            log(1, "Some clients are not responding, exiting anyway.");
        } else {
            log(1, "Succeed in disconnecting all clients, exiting.");
        }
    } else {
        log(1, "There are no registered clients. Exiting.");
    }
//    log(1, "Exiting.");
    end = true;
    shutdown(sockfd, SHUT_RDWR);
}

int main() {
    struct sockaddr_in clientAssoc;
    int connectionDesc;
    int socket;
    socklen_t assocSize;
    std::atomic<bool> end;
    end = false;
    ConHandler conHandler("configfile.conf");
    socket = initSocket();
    if (socket < 0) {
        exit (-1);
    }
#ifndef NO_TERMINATION
    sigset_t termset;
    sigemptyset(&termset);
    sigaddset(&termset, SIGINT);
    pthread_sigmask(SIG_BLOCK, &termset, nullptr);
    std::thread terminationThread(terminationHandler, std::ref(termset),
                                  std::ref(conHandler), std::ref(end), socket);
#endif //NO_TERMINATION
    ctpl::thread_pool tp(4);
#ifndef NO_CLIENT_MOCK
    pthread_t mock;
    pthread_create(&mock, NULL, clientMock, NULL);
#endif //NO_CLIENT_MOCK
    assocSize = sizeof(clientAssoc);
    while(!end) {
        connectionDesc = accept(socket, (struct sockaddr *) &clientAssoc, &assocSize);
        if (connectionDesc > 0) {
            conHandler.handle(connectionDesc, clientAssoc.sin_addr);
            tp.push(conHandle, std::ref(conHandler), connectionDesc, clientAssoc.sin_addr);
        }
    }
    tp.stop(true);
    close(connectionDesc);
    close(socket);

#ifndef NO_CLIENT_MOCK
    pthread_join(mock, NULL);
#endif //NO_CLIENT_MOCK
#ifndef NO_TERMINATION
    terminationThread.join();
#endif //NO_TERMINATION

    return 0;
}