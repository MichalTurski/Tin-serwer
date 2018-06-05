//
// Created by michal on 19.05.18.
//

#include "Service.h"
#include "log.h"

Service::Service(unsigned char id, std::string& name, std::string& unit):id(id), name(name), unit(unit) {}
Service* Service::serviceFactory(unsigned char id, unsigned char devClass, const char *name,
                                 const char *unit, float min, float max) {
    Service *service;
    switch (devClass) {
        case (ANALOG_IN):
            service = new AnalogIn(id, name, unit);
            break;
        case (DIGITAL_IN):
            service = new DigitalIn(id, name, unit);
            break;
        case (ANALOG_OUT):
            service = new AnalogOut(id, name, unit, min, max);
            break;
        case (DIGITAL_OUT):
            service = new DigitalOut(id, name, unit);
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
float Service::getMin() const {
    return 0;
}
float Service::getMax() const {
    return 0;
}

float AnalogIn::getVal() const {
    float toRet;
    mutex.lock();
    toRet = val;
    mutex.unlock();
    return toRet;
}
void AnalogIn::setVal(float newVal) {
    mutex.lock();
    val = newVal;
    mutex.unlock();
}
time_t AnalogIn::getTimestamp() const {
    time_t ts;
    mutex.lock();
    ts = timestamp;
    mutex.unlock();
    return ts;
}
void AnalogIn::setTimestamp(time_t time) {
    mutex.lock();
    timestamp = time;
    mutex.unlock();
}

bool DigitalIn::getVal() const {
    bool toRet;
    mutex.lock();
    toRet = val;
    mutex.unlock();
    return toRet;
}
void DigitalIn::setVal(float newVal) {
    mutex.lock();
    val = (newVal > 0.5);
    mutex.unlock();
}
time_t DigitalIn::getTimestamp() const {
    time_t ts;
    mutex.lock();
    ts = timestamp;
    mutex.unlock();
    return ts;
}
void DigitalIn::setTimestamp(time_t time) {
    mutex.lock();
    timestamp = time;
    mutex.unlock();
}

AnalogOut::AnalogOut(unsigned char id, std::string&& name,
                     std::string&& unit, float min, float max): Service(id, name, unit), current(min),
                                                                min(min), max(max), change(false){}
float AnalogOut::getVal() const {
    float toRet;
    mutex.lock();
    toRet = current;
    mutex.unlock();
    return toRet;
}
bool AnalogOut::beginSetting(float *val) {
    float isChange;
    mutex.lock();
    inProgress = expected;
    *val = expected;
    isChange = change;
    mutex.unlock();
    return isChange;
}
void AnalogOut::finalizeSetting() {
    mutex.lock();
    if (change) {
        change = false;
        current = inProgress;
    }
    mutex.unlock();
}
void AnalogOut::setVal(float newVal) {
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
float AnalogOut::getMin() const {
    return min;
}
float AnalogOut::getMax() const {
    return max;
}

DigitalOut::DigitalOut(unsigned char id, std::string&& name,
                       std::string&& unit): Service(id, name, unit), current(false), change(false){}
bool DigitalOut::getVal() const {
    bool toRet;
    mutex.lock();
    toRet = current;
    mutex.unlock();
    return toRet;
}
bool DigitalOut::beginSetting(float *val) {
    float isChange;
    mutex.lock();
    inProgress = expected;
    *val = (float)expected;
    isChange = change;
    mutex.unlock();
    return isChange;
}
void DigitalOut::finalizeSetting() {
    mutex.lock();
    if (change) {
        change = false;
        current = inProgress;
    }
    mutex.unlock();
}
void DigitalOut::setVal(bool newVal) {
    mutex.lock();
    expected = newVal;
    change = true;
    mutex.unlock();
}
