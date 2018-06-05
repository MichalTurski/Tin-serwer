#include <iostream>
#include <thread>
#include <csignal>
#include <unistd.h>

#include "ClientMock.h"
#include "ConHandler.h"
#include "CTPL/ctpl_stl.h"

int initSocket(uint16_t port = 12345) {
    struct sockaddr_in srvAddr;
    int listenSock;

    srvAddr.sin_family = AF_INET;
    srvAddr.sin_port = htons(port);
    srvAddr.sin_addr.s_addr = INADDR_ANY;

    listenSock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (bind(listenSock, (struct sockaddr*) &srvAddr, sizeof(srvAddr)) < 0) {
        log(1, "Error occurred during binding socket on port %d", port);
        return -1;
    }
    if (listen(listenSock, 10) < 0) {
        log(1, "Error occurred during listening on socket on port %d", port);
        return -1;
    }
    log(1, "Server registered on port %d.", port);
    return listenSock;
}

void terminationHandler(sigset_t &sigset, ConHandler &conHandler, std::atomic<bool> &end, int sockfd) {
    siginfo_t siginfo;
    timespec roundRobinTimeout;
    std::condition_variable *readyToExit;
    std::mutex *readyToExitM;
    bool sigReceived = false;

    roundRobinTimeout.tv_nsec = 0;
    roundRobinTimeout.tv_sec = 25;
    conHandler.getReadyToExit(&readyToExit, &readyToExitM);
    while (!sigReceived) {
        if (sigtimedwait(&sigset, &siginfo, &roundRobinTimeout) > 0) {
            sigReceived = true;
            log(1, "Received signal %s, trying do disconnect from all clients.",
                strsignal(siginfo.si_signo));
            conHandler.setExit();
        }
        conHandler.roundRobinWalk();
    }
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

int main(int argc, char **argv) {
    struct sockaddr_in clientAssoc;
    int connectionDesc;
    int socket;
    int opt;
    int verbosity = 2;
    std::string configfile = "configfile.conf";
    std::string logfile = "server.log";
    uint16_t port = 12345;
    socklen_t assocSize;
    std::atomic<bool> end;
    end = false;

    while ((opt = getopt(argc, argv, "v:p:c:l:")) != -1) {
        switch (opt) {
            case 'v':
                verbosity = atoi(optarg);
                if (verbosity < 0 || verbosity > 4){
                    std::cout << "Verbosity must be integer between 0 and 4.\n";
                    return -1;
                }
                break;
            case 'p':
                port = atoi(optarg);
                if (port < 1025 || port > std::numeric_limits<uint16_t>::max()){
                    std::cout << "Port must be integer between 1025 and "<<
                              std::numeric_limits<uint16_t>::max() << ".\n";
                    return -1;
                }
                break;
            case 'c':
                configfile.assign(optarg);
                break;
            case 'l':
                logfile.assign(optarg);
                break;
            default:
            std::cout << "usage:\n" << argv[0] << "[-v verbosity] [-p port] [-c configfile] [-l logfile] \n";
            return -1;
        }
    }
    initLog(logfile, verbosity);
    ConHandler conHandler("configfile.conf");
    socket = initSocket(port);
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

#ifndef NO_CLIENT_MOCK
    mock.join();
#endif //NO_CLIENT_MOCK
#ifndef NO_TERMINATION
    terminationThread.join();
#else
    close(socket);
#endif //NO_TERMINATION
    logClose();
    return 0;
}