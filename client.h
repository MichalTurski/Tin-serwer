#ifndef CLIENT_H
#define CLIENT_H

#include "pubkey.h"
#include "Server.h"

class Client {
private:
	Pubkey pubkey;
	bool verified;
//	int id;

    bool verifyClient(int sockDesc);
    bool getService(int sockDesc);

public:
	Client(const char *pubkey);
	void initalize(int sockDesc, Server &server);
};
#endif //CLIENT_H
