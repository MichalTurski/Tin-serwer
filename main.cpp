#include <iostream>
#include <thread>
#include <csignal>
#include <unistd.h>

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
        if(readyToExit->wait_for(lock, std::chrono::seconds(10)) == std::cv_status::timeout) {
            log(1, "Some clients are not responding, exiting anyway.");
        } else {
            log(1, "Succeed in disconnecting all clients, exiting.");
        }
        lock.unlock();
    } else {
        log(1, "There are no registered clients. Exiting.");
    }
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
    sigset_t termset;
    sigemptyset(&termset);
    sigaddset(&termset, SIGINT);
    pthread_sigmask(SIG_BLOCK, &termset, nullptr);
#ifndef NO_TERMINATION
    std::thread terminationThread(terminationHandler, std::ref(termset),
                                  std::ref(conHandler), std::ref(end), socket);
#endif //NO_TERMINATION
    sigaddset(&termset, SIGPIPE);
    pthread_sigmask(SIG_BLOCK, &termset, nullptr);
#ifndef NO_THREAD_POOL
    ctpl::thread_pool tp(4);
#endif //NOTHREAD_POOL
#ifndef NO_CLIENT_MOCK
    std::thread mock(clientMock);
#endif //NO_CLIENT_MOCK
    assocSize = sizeof(clientAssoc);
    while(!end) {
        connectionDesc = accept(socket, (struct sockaddr *) &clientAssoc, &assocSize);
        if (connectionDesc >= 0) {
#ifndef NO_THREAD_POOL
            tp.push(conHandle, std::ref(conHandler), connectionDesc, clientAssoc.sin_addr);
#else
            conHandler.handle(connectionDesc, clientAssoc.sin_addr);
#endif //NOTHREAD_POOL
        }
    }
#ifndef NO_THREAD_POOL
    tp.stop(true);
#endif //NOTHREAD_POOL
    close(socket);

#ifndef NO_CLIENT_MOCK
    mock.join();
#endif //NO_CLIENT_MOCK
#ifndef NO_TERMINATION
    terminationThread.join();
#endif //NO_TERMINATION

    return 0;
}