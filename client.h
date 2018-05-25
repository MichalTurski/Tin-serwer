#ifndef CLIENT_H
#define CLIENT_H

#include <map>
#include "pubkey.h"
#include "Server.h"
#include "sesskey.h"
#include "ConHandler.h"

class ConHandler;

class Client {
private:
	uint8_t id;
	Pubkey pubkey;
	ConHandler &conHandler;

	std::map<unsigned char, DigitalIn*> digInputs;
	std::map<unsigned char, AnalogIn*> analogInputs;
	std::map<unsigned char, DigitalOut*> digOutputs;
	std::map<unsigned char, AnalogOut*> analogOutputs;

    bool verifyClient(int sockDesc) const;
	bool registerServices(int sockDesc, Server &server, const Sesskey &sesskey);
	bool getValues(int sockDesc, Sesskey *sesskey, Packet **unused);
	bool setValues(int sockDesc, Sesskey *sesskey);
	bool setExit(int sockDesc, Sesskey *sesskey);
	bool tryUnregister(int sockDesc, Sesskey *sesskey, Packet **unused);
public:
	void unregisterServices(Server &server);
	Client(uint8_t id, const char *pubkey, ConHandler &conHandler);
	~Client();
	bool initalize(int sockDesc, Server &server);
	uint8_t getId() const;
	bool tryDataExchange(int sockDesc, bool end, Packet **unused);
};
#endif //CLIENT_H
