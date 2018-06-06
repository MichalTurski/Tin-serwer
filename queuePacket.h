//
// Created by s17kf on 05.06.18.
//

#ifndef SERWER_QUEUEPACKET_H
#define SERWER_QUEUEPACKET_H

#include <iostream>
#include <cstdint>

#include "ReadQueue.h"
#include "AddQueue.h"

class QueuePacket {
protected:
    enum packet_type{
//        PAK_ACK = 0x11,
        PAK_NAK = 0x12,
        PAK_EOT = 0x13,
        PAK_DESC = 0x14,
        PAK_VAL = 0x15,
        PAK_GET = 0x16,
        PAK_SET = 0x17,
        PAK_EXIT = 0x18
    };

    unsigned char * buf;
    uint32_t buf_size;

    QueuePacket(unsigned char *bufor, uint32_t buf_len);
    explicit  QueuePacket(size_t size);
public:
    static QueuePacket *packetFromQueue(ReadQueue *readQueue);
    virtual ~QueuePacket();
    virtual ssize_t  addToQueue(AddQueue *addQueue);
};


class Q_NAK : public QueuePacket{
public:
    unsigned char getId() const;
    explicit Q_NAK(unsigned char *buf): QueuePacket(buf, 2) {}
    explicit Q_NAK(unsigned char id);
};


class Q_EOT : public QueuePacket{
public:
    explicit Q_EOT(unsigned char *buf) : QueuePacket(buf, 1) {};
    Q_EOT();
};

class Q_DESC : public QueuePacket{
public:
    unsigned char getDeviceId() const;
    unsigned char getDeviceClass() const;
    const char *getName() const;
    const char *getUnit() const;
    float getMin() const;
    float getMax() const;
    Q_DESC(unsigned char dev_id, unsigned char dev_class, const std::string &name, const std::string &unit,
            float min, float max);
    Q_DESC(unsigned char *buf, size_t buf_len): QueuePacket(buf, buf_len) {}

};

class Q_VAL : public QueuePacket{
public:
    unsigned char getServiceId() const;
    float getValue() const;
    time_t getTimestamp() const;

    explicit Q_VAL(unsigned char *buf): QueuePacket(buf, 10){}
    Q_VAL(unsigned char id, float val, time_t timeStamp);
};


class Q_GET : public QueuePacket{
public:
    unsigned char getId() const;
    Q_GET(unsigned char *buf): QueuePacket(buf, 2){}
    Q_GET(unsigned char id);
};

class Q_SET : public QueuePacket{
public:
    unsigned char getId() const;
    float getValue() const;
    Q_SET(unsigned char * buf):QueuePacket(buf, 6){}
    Q_SET(unsigned char id, float value);
};

class Q_EXIT : public QueuePacket{
public:
    unsigned char getId() const;
    explicit Q_EXIT(unsigned char *buf) : QueuePacket(buf, 2){}
    explicit Q_EXIT(unsigned char id);
};












#endif //SERWER_QUEUEPACKET_H
