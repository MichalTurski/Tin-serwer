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
#include "ConHandler.h"

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
    struct sockaddr_in clientAssoc;
    socklen_t assocSize;
    //Server server("privkey.pem");
    //Client client("pubkey.pem");
    ConHandler conHandler("configfile.conf");
    int socket = initSocket();
    pthread_t thread;
    pthread_create(&thread, NULL, clientMock, NULL);

    assocSize = sizeof(clientAssoc);
    int sock = accept(socket, (struct sockaddr*)&clientAssoc, &assocSize);
//    client.initalize(sock, server);
    conHandler.handle(sock, clientAssoc.sin_addr);
    close(sock);
    close(socket);

    pthread_join(thread, NULL);

    return 0;
}