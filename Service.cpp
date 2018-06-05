//
// Created by michal on 19.05.18.
//

#include "Service.h"
#include "log.h"

Service::Service(unsigned char id, serviceType type, std::string& name, std::string& unit, float min, float max):
        id(id), type(type), name(name), unit(unit), min(min), max(max), val(min) {}
Service* Service::serviceFactory(unsigned char id, unsigned char srvType, const char *name,
                                 const char *unit, float min, float max) {
    Service *service;
    switch (srvType) {
        case (ANALOG_IN):
        case (DIGITAL_IN):
            service = new Input(id, static_cast<serviceType>(srvType), name, unit, min, max);
            break;
        case (ANALOG_OUT):
        case (DIGITAL_OUT):
            service = new Output(id, static_cast<serviceType>(srvType), name, unit, min, max);
            break;
        default:
            log(3, "Not recognised service type\n");
            service = nullptr;
    }
    return service;
}
unsigned char Service::getId() const {
    return id;
}
const std::string& Service::getName() const {
    return name;
}
const std::string& Service::getUnit() const {
    return unit;
}
float Service::getVal() const {
    float toRet;
    mutex.lock();
    toRet = val;
    mutex.unlock();
    return toRet;
}
float Service::getMin() const {
    return min;
}
float Service::getMax() const {
    return max;
}
uint8_t Service::getType() const {
    return type;
}

void Input::setVal(float newVal) {
    mutex.lock();
    val = newVal;
    mutex.unlock();
}
time_t Input::getTimestamp() const {
    time_t ts;
    mutex.lock();
    ts = timestamp;
    mutex.unlock();
    return ts;
}
void Input::setTimestamp(time_t time) {
    mutex.lock();
    timestamp = time;
    mutex.unlock();
}

Output::Output(unsigned char id, serviceType type, std::string&& name, std::string&& unit, float min, float max):
        Service(id, type, name, unit, min, max), change(false){}
bool Output::beginSetting(float *val) {
    bool isChange;
    mutex.lock();
    inProgress = expected;
    *val = expected;
    isChange = change;
    mutex.unlock();
    return isChange;
}
void Output::finalizeSetting() {
    mutex.lock();
    if (change) {
        change = false;
        val = inProgress;
    }
    mutex.unlock();
}
void Output::setVal(float newVal) {
    mutex.lock();
    if (newVal > max)
        expected = max;
    else if (newVal < min)
        expected = min;
    else
        expected = newVal;
    change = true;
    mutex.unlock();
}
