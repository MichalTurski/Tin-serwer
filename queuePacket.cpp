//
// Created by s17kf on 05.06.18.
//

#include <stdexcept>
#include "queuePacket.h"



QueuePacket::QueuePacket(unsigned char *bufor, uint32_t buf_len) : buf_size(buf_len){
    buf = new unsigned char[buf_len];
    memcpy(buf, bufor, buf_len);
}

QueuePacket::QueuePacket(size_t size) {
    buf_size = (uint32_t) size;
    buf = new unsigned char[size];
}

QueuePacket::~QueuePacket() {
    delete buf;
}

QueuePacket* QueuePacket::packetFromQueue(ReadQueue *readQueue) {
//    std::cout<<"packet from queue"<<std::endl;
    QueuePacket *new_packet;
    char bufor[256];
    int read = readQueue->readToCharArray(bufor);
    unsigned char bufor_unsigned[256];
    memcpy(bufor_unsigned,bufor, read);

    switch (bufor_unsigned[0]){
        case (PAK_NAK):
            new_packet = new Q_NAK(bufor_unsigned);
            break;
        case (PAK_EOT):
            new_packet = new Q_EOT(bufor_unsigned);
            break;
        case (PAK_DESC):
            new_packet = new Q_DESC(bufor_unsigned, read);
            break;
        case (PAK_VAL):
            new_packet = new Q_VAL(bufor_unsigned);
            break;
        case (PAK_GET):
            new_packet = new Q_GET(bufor_unsigned);
            break;
        case (PAK_SET):
            new_packet = new Q_SET(bufor_unsigned);
            break;
        case (PAK_EXIT):
            new_packet = new Q_EXIT(bufor_unsigned);
    }




    return new_packet;
}

ssize_t QueuePacket::addToQueue(AddQueue *addQueue) {
    char buf_to_send[buf_size];
    memcpy(buf_to_send, buf, buf_size);
    ssize_t ret = addQueue->addMessage(buf_to_send, buf_size);
    return ret;
}



Q_NAK::Q_NAK(unsigned char id) :QueuePacket(2) {
    buf[0] = PAK_NAK;
    buf[1] = id;
}

unsigned char Q_NAK::getId() const {
    return buf[1];
}

Q_EOT::Q_EOT() :QueuePacket(1) {
    buf[0] = PAK_EOT;
}

Q_DESC::Q_DESC(unsigned char dev_id, unsigned char dev_class, const std::string &name, const std::string &unit,
 float min, float max) :QueuePacket(name.length() + 16) { //1 byte fir header, 1 for device id, 1 for type, 1 for terminating 0, 4 for unit, 4 for min, 4 for max, total 16
    if(unit.size() > 3) {
        delete[] buf;
        throw(std::runtime_error("Unit name is too long."));
    }
    buf[0] = PAK_DESC;
    buf[1] = dev_id;
    buf[2] = dev_class;
    memcpy(&buf[3], name.c_str(), name.size()+1);
    memcpy(&buf[buf_size-12], unit.c_str(), 4);
    memcpy(&buf[buf_size-8], &min, sizeof(float));
    memcpy(&buf[buf_size-4], &max, sizeof(float));
}

unsigned char Q_DESC::getDeviceId() const {
    return buf[1];
}

unsigned char Q_DESC::getDeviceClass() const {
    return  buf[2];
}

const char* Q_DESC::getName() const {
    return reinterpret_cast<char *> (&buf[3]);
}

const char* Q_DESC::getUnit() const {
    return reinterpret_cast<char *>(&buf[buf_size-12]);
}

float Q_DESC::getMin() const {
    float min;
    memcpy(&min, &buf[buf_size-8], sizeof(float));
    return min;
}

float Q_DESC::getMax() const {
    float max;
    memcpy(&max, &buf[buf_size-4], sizeof(float));
    return max;
}

Q_VAL::Q_VAL(unsigned char id, float val, time_t timeStamp) : QueuePacket(10){
    buf[0] = PAK_VAL;
    buf[1] = id;
    memcpy(&buf[2], &val, sizeof(float));
    memcpy(&buf[6], &timeStamp, sizeof(time_t));
}

unsigned char Q_VAL::getServiceId() const {
    return buf[1];
}

float Q_VAL::getValue() const {
    float val;
    memcpy(&val, &buf[2], sizeof(float));
    return val;
}

time_t Q_VAL::getTimestamp() const {
    time_t time_stamp;
    memcpy(&time_stamp, &buf[6], sizeof(time_t));
    return time_stamp;
}

Q_GET::Q_GET(unsigned char id) : QueuePacket(2) {
    buf[0] = PAK_GET;
    buf[1] = id;
}

unsigned char Q_GET::getId() const {
    return buf[1];
}

Q_SET::Q_SET(unsigned char id, float value):QueuePacket(6) {
    buf[0] = PAK_SET;
    buf[1] = id;
    memcpy(&buf[2], &value, sizeof(float));
}

unsigned char Q_SET::getId() const {
    return buf[1];
}

float Q_SET::getValue() const {
    float val;
    memcpy(&val, &buf[2], sizeof(float));
    return val;
}

Q_EXIT::Q_EXIT(unsigned char id) :QueuePacket(2) {
    buf[0] = PAK_EXIT;
    buf[1] = id;
}

















