#ifndef CLIENT_H
#define CLIENT_H

#include "pubkey.h"
#include "Server.h"

class Client {
private:
	Pubkey pubkey;
	int id;

    bool verifyClient(int sockDesc) const;
    bool getService(int sockDesc);

public:
	Client(int id, const char *pubkey);
	bool initalize(int sockDesc, const Server &server);
};
#endif //CLIENT_H
