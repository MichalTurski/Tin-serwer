//
// Created by s17kf on 31.05.18.
//

#include "ReadQueue.h"
#include "common.h"


ReadQueue::ReadQueue(const std::string name, int oflags) {
    queueDescriptor = mq_open(name.c_str(), oflags);
    queueName = name;
    CHECK(queueDescriptor != (mqd_t)-1);

//    CHECK(mq_getattr(queueDescriptor, attributes) == 0);

    lastMsg = new char[getMsgSize()+1];

}

ReadQueue::ReadQueue(const std::string name, int oflags, int queueMode, int msgSize, int maxMsgs) {

    attributes = new mq_attr();
    attributes->mq_msgsize = msgSize;
    attributes->mq_maxmsg = maxMsgs;

//    std::cout<<01<<std::endl;
    queueDescriptor = mq_open(name.c_str(), oflags, queueMode, attributes);
//    std::cout<<02<<std::endl;
    queueName = name;
    CHECK(queueDescriptor != (mqd_t)-1);

    lastMsg = new char[getMsgSize()+1];

}

//std::string ReadQueue::readToString() {
//
//    return readToCharArray();
//
//}

int ReadQueue::readToCharArray(char *dest) {

    struct timespec tm;
    int bytesRead;
//    int bytesRead = mq_receive(queueDescriptor, lastMsg, getMsgSize(), NULL);
    clock_gettime(CLOCK_REALTIME, &tm);
    do {
        tm.tv_sec += 1;
        bytesRead = mq_timedreceive(queueDescriptor, dest, getMsgSize(), NULL, &tm);
    } while (bytesRead <= 0 && errno != EBADF) ;

    return bytesRead;
}

//Packet* ReadQueue::readToPacket() {
//    int bytesRead = mq_receive(queueDescriptor, lastMsg, getMsgSize(), NULL);
//    CHECK(bytesRead >= 0);
//
////    std::cout<<"10"<<std::endl;
//    unsigned char msg[bytesRead];
//    memcpy(msg, lastMsg, bytesRead);
//
////    std::cout<<"11"<<std::endl;
//    Packet *packet = Packet::packetFromQueue(msg, bytesRead);
//
//    return packet;
//
//}



std::string ReadQueue::getName() {
    return queueName;
}

int ReadQueue::getMsgSize() {
    mq_attr attr;

    if (mq_getattr(queueDescriptor, &attr) < 0) {
        return 0;
    } else {
        return attr.mq_msgsize;
    }
}

ReadQueue::~ReadQueue() {
    CHECK(mq_close(queueDescriptor) != (mqd_t)-1);
    CHECK(mq_unlink(queueName.c_str()) != (mqd_t)-1);
    delete(attributes);
    delete(lastMsg);
}

long ReadQueue::getMessagesInQueue() {
    mq_attr attr;
    CHECK(mq_getattr(queueDescriptor, &attr) == 0);
    return attr.mq_curmsgs;

}