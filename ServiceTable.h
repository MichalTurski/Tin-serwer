//
// Created by michal on 19.05.18.
//

#ifndef SERWER_SERVICETABLE_H
#define SERWER_SERVICETABLE_H


#include <vector>
#include <list>
#include <shared_mutex>
#include "Service.h"

class ServiceTable {
private:
    std::vector<Service*> services;
    std::list<unsigned char> free;
    std::list<unsigned char> reserved;
    mutable std::shared_timed_mutex mutex;
public:
    ServiceTable();
    unsigned char reserve();
    bool unreserve(unsigned char id);
    bool push(Service *service, unsigned char id);
    bool remove(unsigned char id);
};


#endif //SERWER_SERVICETABLE_H
