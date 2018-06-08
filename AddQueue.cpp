//
// Created by s17kf on 31.05.18.
//

#include <cstring>
#include "AddQueue.h"
#include "common.h"

AddQueue::AddQueue(const std::string name, int oflags) {
    queueDescriptor = mq_open(name.c_str(), oflags);
    queueName = name;
    //TODO check error
    CHECK(queueDescriptor != (mqd_t)-1);
}

AddQueue::AddQueue(const std::string name, int oflags, int queueMode, int msgSize, int maxMsgs) {

    attributes = new mq_attr();
    attributes->mq_msgsize = msgSize;
    attributes->mq_maxmsg = maxMsgs;

//    std::cout<<01<<std::endl;
    queueDescriptor = mq_open(name.c_str(), oflags, queueMode, attributes);
//    std::cout<<02<<std::endl;
    queueName = name;
    CHECK(queueDescriptor != (mqd_t)-1);

   // lastMsg = new char[getMsgSize()+1];

}

int AddQueue::addMessage(std::string msg) {
    return addMessage(msg.c_str(), msg.size()+1);
}

int AddQueue::addMessage(const char *msg, int msgSize) {
    return mq_send(queueDescriptor, msg, msgSize, 0);
}

//int AddQueue::addMessage(Packet *packet) {
//    hex_print(packet->getBuf(),packet->getBufSize());
//    char msg[packet->getBufSize()];
//    memcpy(&msg, packet->getBuf(), packet->getBufSize());
//
//    switch(msg[0]){
//        case PCK_SET:
//            msg[0] = PCK_Q_SET;
//            break;
//        case PCK_GET:
//            msg[0] = PCK_Q_GET;
//            std::cout<<"GET ready to set to Gonzo"<<std::endl;
//            break;
//        default:
//            std::cout<<"This packet should not be sent to Gonzo"<<std::endl;
//            hex_print(msg,packet->getBufSize());
//            return -1;
//    }
//
////    memcpy(&msg, packet->getBuf(), packet->getBufSize());
//    return addMessage(msg, packet->getBufSize());
//}


std::string AddQueue::getName() {
    return queueName;
}

AddQueue::~AddQueue() {
    CHECK(mq_close(queueDescriptor) != (mqd_t)-1);
    CHECK(mq_unlink(queueName.c_str()) != (mqd_t)-1);
}

long AddQueue::getMessagesInQueue() {
    mq_attr attr;
    CHECK(mq_getattr(queueDescriptor, &attr) == 0);
    return attr.mq_curmsgs;

}
