//
// Created by michal on 19.05.18.
//

#include "Service.h"
#include "utils.h"

Service::Service(unsigned char id, std::string& name, std::string& unit):id(id), name(name), unit(unit) {}
Service* Service::serviceFactory(unsigned char id, unsigned char devClass, const char *name,
                                 const char *unit, float min, float max) {
    Service *service;
    switch (devClass) {
        case (ANALOG_IN):
            service = new AnalogIn(id, name, unit);
            break;
        case (DIGITAL_IN):
            service = new DigitalOut(id, name, unit);
            break;
        case (ANALOG_OUT):
            service = new AnalogOut(id, name, unit, min, max);
            break;
        case (DIGITAL_OUT):
            service = new DigitalOut(id, name, unit);
            break;
        default:
            log(2, "Not recognised service type\n");
            service = nullptr;
    }
    return service;
}
unsigned char Service::getId() const {
    return id;
}

float AnalogIn::getVal() {
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

bool DigitalIn::getVal() {
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


AnalogOut::AnalogOut(unsigned char id, std::string&& name,
                     std::string&& unit, float min, float max): Service(id, name, unit), current(min),
                                                                min(min), max(max), change(false){}
float AnalogOut::getVal() {
    float toRet;
    mutex.lock();
    toRet = current;
    mutex.unlock();
    return toRet;
}
float AnalogOut::beginSetting() {
    float newVal;
    mutex.lock();
    inProgress = expected;
    newVal = expected;
    mutex.unlock();
    return newVal;
}
void AnalogOut::finalizeSetting() {
    mutex.lock();
    change = false;
    current = inProgress;
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
bool DigitalOut::getVal() {
    bool toRet;
    mutex.lock();
    toRet = current;
    mutex.unlock();
    return toRet;
}
float DigitalOut::beginSetting() {
    float newVal;
    mutex.lock();
    inProgress = expected;
    newVal = (float)expected;
    mutex.unlock();
    return newVal;
}
void DigitalOut::finalizeSetting() {
    mutex.lock();
    change = false;
    current = inProgress;
    mutex.unlock();
}
void DigitalOut::setVal(bool newVal) {
    mutex.lock();
    expected = newVal;
    change = true;
    mutex.unlock();
}
