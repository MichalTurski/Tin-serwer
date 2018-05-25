//
// Created by michal on 19.05.18.
//

#include "ServiceTable.h"

ServiceTable::ServiceTable() {
    services.push_back(nullptr);//service number 0 is reserved
}

unsigned char ServiceTable::reserve() {
    std::list<unsigned char>::iterator reservedIter;
    unsigned char candidate;

    std::unique_lock<std::shared_timed_mutex> lock(mutex);
    if (free.empty()) {
        candidate = services.size();//if free is empty, there are no gaps in services table.
        do {
            for (reservedIter = reserved.begin(); reservedIter != reserved.end(); reservedIter++) {
                if (*reservedIter == candidate) {
                    candidate++;
                    break;
                }
            }
        } while (reservedIter != reserved.end());
    } else {
        candidate = free.front();
        free.pop_front();
    }
    if (candidate > 255) {
        candidate = 0;
    } else {
        reserved.push_back(candidate);
    }
    return (candidate);
}

bool ServiceTable::unreserve(unsigned char num) {
    std::list<unsigned char>::iterator reservedIter;

    std::unique_lock<std::shared_timed_mutex> lock(mutex);
    for (reservedIter = reserved.begin(); reservedIter != reserved.end(); reservedIter++) {
        if (*reservedIter == num) {
            reserved.erase(reservedIter);
            free.push_back(num);
            return true;
        }
    }
    return false;
}
bool ServiceTable::push(Service *service, unsigned char num) {
    std::list<unsigned char>::iterator reservedIter;

    std::unique_lock<std::shared_timed_mutex> lock(mutex);
    for (reservedIter = reserved.begin(); reservedIter != reserved.end(); reservedIter++) {
        if (*reservedIter == num) {
            reserved.erase(reservedIter);
            services[num] = service;
            return true;
        }
    }
    return false;
}
bool ServiceTable::remove(unsigned char num) {
    std::unique_lock<std::shared_timed_mutex> lock(mutex);
    if (num < services.size() && services[num] != nullptr) {
        services[num] = nullptr;
        free.push_back(num);
        return true;
    }
    return false;
}
