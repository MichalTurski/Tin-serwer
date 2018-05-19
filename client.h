#ifndef CLIENT_H
#define CLIENT_H

#include <map>
#include "pubkey.h"
#include "Server.h"
#include "sesskey.h"

class Client {
private:
	int id;
	Pubkey pubkey;
	std::map<unsigned char, DigitalIn*> digInputs;
	std::map<unsigned char, AnalogIn*> analogInputs;
	std::map<unsigned char, DigitalOut*> digOutputs;
	std::map<unsigned char, AnalogOut*> analogOutputs;

    bool verifyClient(int sockDesc) const;
	bool registerServices(int sockDesc, Server &server, const Sesskey &sesskey);
public:
	void unregisterServices(Server &server);
	Client(int id, const char *pubkey);
	~Client();
	bool initalize(int sockDesc, Server &server);
};
#endif //CLIENT_H
