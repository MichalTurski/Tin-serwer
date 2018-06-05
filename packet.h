#ifndef PACKET_H
#define PACKET_H

#include <cstring>
#include <cstdint>
#include <string>

#include "sesskey.h"
#include "consts.h"

class Sesskey;

class Packet {
protected:
    unsigned char *buf;
    uint32_t buf_size;
//	Packet(unsigned char *buf): buf(buf) {}
    Packet(const unsigned char *buf_in, uint32_t buf_len);
    Packet(size_t size) ;
public:
    unsigned char * getBuf(){return buf;}
    uint32_t getBufSize(){ return buf_size;}
    static Packet *packetFactory(int soc_desc, const Sesskey *sesskey);
    static Packet *packetFromQueue(unsigned char *buf_in, uint32_t buf_in_size);
    //    Packet(int soc_desc, const Sesskey *sesskey);
    virtual ~Packet();
    virtual ssize_t send(int soc_desc, const Sesskey *sesskey) const = 0;

    void setPacketID(unsigned char id);
};

/* This class keeps packets that travels via net encrypted. */
class EncrptedPacket : public Packet {
protected:
    EncrptedPacket(const unsigned char *buf, uint32_t buf_len): Packet(buf, buf_len){}
    EncrptedPacket(size_t size): Packet(size){}
public:
    ssize_t send(int soc_desc, const Sesskey *sesskey) const override ;
};

/* This class keeps packets that travels via net in plaintext or decrypted with RSA. */
class PlainPacket : public Packet {
protected:
    PlainPacket(const unsigned char *buf, uint32_t buf_len): Packet(buf, buf_len){}
//	PlainPacket(unsigned char *buf): Packet(buf){}
    PlainPacket(size_t size): Packet(size){}
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
private:
    KEY(const unsigned char* buf): PlainPacket(buf, 257) {}
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
    DESC(const std::string &name, const std::string &unit, float min, float max);
    //DESC(Packet &&packet);
    DESC(unsigned char *buf, size_t bufSize): EncrptedPacket(buf, bufSize) {}
};

class VAL : public EncrptedPacket {
public:
    unsigned char getServiceId() const;
    float getValue() const;
    time_t getTimestamp() const;
    VAL(unsigned char *buf): EncrptedPacket(buf, 10) {}
    VAL(unsigned char id, float value, time_t timestamp);
    //VAL(Packet &&packet);
};

class SET : public EncrptedPacket {
public:
    SET(unsigned char *buf): EncrptedPacket(buf, 6){}
    SET(unsigned char id, float value);
    float getValue() const;
    unsigned char getId() const;
};

class GET : public EncrptedPacket {
public:
    GET(unsigned char *buf): EncrptedPacket(buf, 2){}
    GET(unsigned char id);
    unsigned char getId();
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
    ID(unsigned char id);
    //ID(Packet &&packet);
};
#endif //PACKET_H
