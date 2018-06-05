#ifndef PACKET_H
#define PACKET_H

#include <cstring>
#include <cstdint>
#include <string>

#include "sesskey.h"


class Sesskey;

class Packet {
protected:
	enum pck_type {
		PCK_ACK	= 0x01,
		PCK_NAK = 0x02,
		PCK_EOT	= 0x03,
		PCK_CHALL = 0x04,
		PCK_CHALL_RESP = 0x05,
		PCK_KEY = 0x06,
		PCK_DESC = 0x07,
		PCK_VAL = 0x08,
		PCK_SET = 0x09,
		PCK_EXIT = 0x0a,
		PCK_ID = 0x0b
	};
	unsigned char *buf;
	uint32_t buf_size;
	Packet(const unsigned char *buf_in, uint32_t buf_len);
	explicit Packet(size_t size) ;
public:
	static Packet *packetFactory(int soc_desc, const Sesskey *sesskey);
    virtual ~Packet();
    virtual ssize_t send(int soc_desc, const Sesskey *sesskey) const = 0;
};

/* This class keeps packets that travels via net encrypted. */
class EncrptedPacket : public Packet {
protected:
	EncrptedPacket(const unsigned char *buf, uint32_t buf_len): Packet(buf, buf_len){}
	explicit EncrptedPacket(size_t size): Packet(size){}
public:
	ssize_t send(int soc_desc, const Sesskey *sesskey) const override ;
};

/* This class keeps packets that travels via net in plaintext or decrypted with RSA. */
class PlainPacket : public Packet {
protected:
	PlainPacket(const unsigned char *buf, uint32_t buf_len): Packet(buf, buf_len){}
//	PlainPacket(unsigned char *buf): Packet(buf){}
	explicit PlainPacket(size_t size): Packet(size){}
public:
	ssize_t send(int soc_desc, const Sesskey *sesskey) const override;
};

class ACK : public EncrptedPacket {
public:
	unsigned char getId() const;
	//ACK(Packet &&packet);
	explicit ACK(unsigned char *buf): EncrptedPacket(buf, 2) {}
	explicit ACK(unsigned char id);
};

class NAK : public EncrptedPacket {
public:
	unsigned char getId() const;
	//NAK(Packet &&packet);
	explicit NAK(unsigned char *buf): EncrptedPacket(buf, 2) {}
	explicit NAK(unsigned char id);
};

class EOT : public EncrptedPacket {
public:
	//EOT(Packet &&packet);
	explicit EOT(unsigned char *buf): EncrptedPacket(buf, 1) {}
	EOT();
};

class CHALL : public PlainPacket {
private:
	explicit CHALL(const unsigned char *buf): PlainPacket(buf, 9) {}
public:
    const unsigned char *getChall() const;
    static CHALL *createFromMessage(const unsigned char *msg);
    static CHALL *createFromRandom(const unsigned char *rand);
};

class CHALL_RESP : public PlainPacket {
private:
	explicit CHALL_RESP(const unsigned char *buf): PlainPacket(buf, 257) {}
public:
	const unsigned char *getResp() const;
	//CHALL_RESP(Packet &&packet);
	static CHALL_RESP *createFromMessage(const unsigned char *msg);
	static CHALL_RESP *createFromEncrypted(const unsigned char *encrypt);
};

class KEY : public PlainPacket {
private:
	explicit KEY(const unsigned char* buf): PlainPacket(buf, 257) {}
public:
    const unsigned char *getKeyBuf() const;
	//KEY(const unsigned char* encoded);
	static KEY *createFromMessage(const unsigned char *msg);
	static KEY *createFromEncrypted(const unsigned char *encrypt);
};

class DESC : public EncrptedPacket {
public:
	unsigned char getDeviceClass() const;
	const char *getName() const;
	const char *getUnit() const;
	float getMin() const;
	float getMax() const;
	DESC(std::string &name, std::string &unit, float min, float max);
	//DESC(Packet &&packet);
	DESC(unsigned char *buf, size_t bufSize): EncrptedPacket(buf, bufSize) {}
};

class VAL : public EncrptedPacket {
public:
	unsigned char getServiceId() const;
	float getValue() const;
	time_t getTimestamp() const;
	explicit VAL(unsigned char *buf): EncrptedPacket(buf, 10) {}
	//VAL(Packet &&packet);
};

class SET : public EncrptedPacket {
public:
	SET(unsigned char id, float value);
};

class EXIT : public EncrptedPacket {
public:
    unsigned char getCode() const;
	explicit EXIT(unsigned char *buf): EncrptedPacket(buf, 2) {}
	//EXIT(Packet &&packet);
	explicit EXIT(unsigned char code);
};

class ID : public PlainPacket {
public:
	unsigned char getId() const;
	explicit ID(unsigned char *buf): PlainPacket(buf, 2) {}
	explicit ID(unsigned char id);
	//ID(Packet &&packet);
};
#endif //PACKET_H
