#include <iostream>
#include <netinet/in.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <zconf.h>
#include "RNG.h"
#include "packet.h"
#include "privkey.h"
#include "Server.h"
#include "client.h"
#include "ClientMock.h"

int initSocket() {
    struct sockaddr_in srvAddr;
    int listenSock;

    srvAddr.sin_family = AF_INET;
    srvAddr.sin_port = htons(1234);
    srvAddr.sin_addr.s_addr = inet_addr("0.0.0.0");

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

int main() {
    Server server("privkey.pem");
    Client client("pubkey.pem");
    int socket = initSocket();
    pthread_t thread;
    pthread_create(&thread, NULL, clientMock, NULL);

    int sock = accept(socket, NULL, NULL);
    client.initalize(sock, server);
    close(sock);
    close(socket);


    pthread_join(thread, NULL);



    //TODO:: dopisać privkey.cpp i storzyć maszynę stanów która zrealizuje pojedyncze połącznie
    return 0;
}