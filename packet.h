#ifndef PACKET_H
#define PACKET_H

#include <cstdlib>
#include <ctime>
#include <cstdint>
#include <cstring>

#include "sesskey.h"

#define PCK_ACK		0x01
#define PCK_NAK		0x02
#define PCK_EOT		0x03
#define PCK_CHALL	0x04
#define PCK_CHALL_RESP	0x05
#define PCK_KEY		0x06
#define PCK_DESC	0x07
#define PCK_VAL		0x08
#define PCK_SET		0x09
#define PCK_EXIT	0x0a
#define PCK_ID		0x0b

class Sesskey;

class Packet {
protected:
	unsigned char *buf;
	uint32_t buf_size;
//	Packet(unsigned char *buf): buf(buf) {}
	Packet(const unsigned char *buf_in, uint32_t buf_len);
	Packet(int size);
public:
	static Packet *packetFactory(int soc_desc, const Sesskey *sesskey);
//    Packet(int soc_desc, const Sesskey *sesskey);
    ~Packet();
    virtual ssize_t send(int soc_desc, const Sesskey *sesskey) const = 0;
};

/* This class keeps packets that travels via net encrypted. */
class EncrptedPacket : public Packet {
protected:
	EncrptedPacket(const unsigned char *buf, uint32_t buf_len): Packet(buf, buf_len){}
	EncrptedPacket(int size): Packet(size){}
public:
	ssize_t send(int soc_desc, const Sesskey *sesskey) const override ;
};

/* This class keeps packets that travels via net in plaintext or decrypted with RSA. */
class PlainPacket : public Packet {
protected:
	PlainPacket(const unsigned char *buf, uint32_t buf_len): Packet(buf, buf_len){}
//	PlainPacket(unsigned char *buf): Packet(buf){}
	PlainPacket(int size): Packet(size){}
public:
	ssize_t send(int soc_desc, const Sesskey *sesskey) const override;
};

class ACK : public EncrptedPacket {
public:
	unsigned char getId() const;
	//ACK(Packet &&packet);
	ACK(unsigned char *buf): EncrptedPacket(buf, 2) {}
	ACK(unsigned char id);
};

class NAK : public EncrptedPacket {
public:
	unsigned char getId() const;
	//NAK(Packet &&packet);
	NAK(unsigned char *buf): EncrptedPacket(buf, 2) {}
	NAK(unsigned char id);
};

class EOT : public EncrptedPacket {
public:
	//EOT(Packet &&packet);
	EOT(unsigned char *buf): EncrptedPacket(buf, 1) {}
	EOT();
};

class CHALL : public PlainPacket {
private:
	CHALL(const unsigned char *buf): PlainPacket(buf, 9) {}
public:
    const unsigned char *getChall() const;
    static CHALL *createFromMessage(const unsigned char *msg);
    static CHALL *createFromRandom(const unsigned char *rand);
};

class CHALL_RESP : public PlainPacket {
private:
	CHALL_RESP(const unsigned char *buf): PlainPacket(buf, 257) {}
public:
	const unsigned char *getResp() const;
	//CHALL_RESP(Packet &&packet);
	static CHALL_RESP *createFromMessage(const unsigned char *msg);
	static CHALL_RESP *createFromEncrypted(const unsigned char *encrypt);
};

class KEY : public PlainPacket {
public:
    const unsigned char *getKeyBuf() const;
	KEY(const unsigned char* encoded);
};

class DESC : public EncrptedPacket {
public:
	unsigned char getDeviceClass() const;
	const char *getName() const;
	const char *getUnit() const;
	float getMin() const;
	float getMax() const;
	//DESC(Packet &&packet);
	DESC(unsigned char *buf): EncrptedPacket(buf, 17) {}
};

class VAL : public EncrptedPacket {
public:
	unsigned char getServiceId() const;
	float getValue() const;
	time_t getTimestamp() const;
	VAL(unsigned char *buf): EncrptedPacket(buf, 10) {}
	//VAL(Packet &&packet);
};

class SET : public EncrptedPacket {
public:
	SET(unsigned char id, float value);
};

class EXIT : public EncrptedPacket {
public:
    unsigned char getCode() const;
	EXIT(unsigned char *buf): EncrptedPacket(buf, 2) {}
	//EXIT(Packet &&packet);
	EXIT(unsigned char code);
};

class ID : public PlainPacket {
public:
	unsigned char getId() const;
	ID(unsigned char *buf): PlainPacket(buf, 2) {}
	//ID(Packet &&packet);
};
#endif //PACKET_H
