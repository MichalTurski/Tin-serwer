//
// Created by michal on 19.05.18.
//

#ifndef SERWER_SERVICE_H
#define SERWER_SERVICE_H

#include <mutex>

class Service {
private:
    enum serviceType {
        ANALOG_IN = 0x00,
        ANALOG_OUT = 0x01,
        DIGITAL_IN = 0x02,
        DIGITAL_OUT = 0x03
    };
    unsigned char id;
protected:
    std::mutex mutex;
    std::string name;
    std::string unit;
    Service(unsigned char id, std::string &name, std::string& unit);
public:
    static Service* serviceFactory(unsigned char id, unsigned char devClass, const  char *name,
                                   const char *unit, float min, float max);
    virtual ~Service() = default;
    unsigned char getId() const;
};

class AnalogIn: public Service {
private:
    float val;
public:
    AnalogIn(unsigned char id, std::string&& name, std::string&& unit): Service(id, name, unit),
                                                                        val(0) {}
    float getVal();
    void setVal(float newVal);
};

class DigitalIn: public Service {
private:
    bool val;
public:
    DigitalIn(unsigned char id, std::string&& name, std::string&& unit): Service(id, name, unit),
                                                                         val(false) {}
    bool getVal();
    void setVal(float newVal);
};

class AnalogOut: public Service {
private:
    float current;
    float expected;//this value will be set in next connection
    float inProgress;//this value is being set, if succeed current will be equal to inProgress
    const float min;
    const float max;
    bool change;
public:
    AnalogOut(unsigned char id, std::string&& name, std::string&& unit, float min, float max);
    float getVal();
    float beginSetting();
    void finalizeSetting();
    void setVal(float newVal);
    float getMin() const;
    float getMax() const;
};

class DigitalOut: public Service {
private:
    bool current;
    bool expected;//this value will be set in next connection
    bool inProgress;//this value is being set, if succeed current will be equal to inProgress
    bool change;
public:
    DigitalOut(unsigned char id, std::string&& name, std::string&& unit);
    bool getVal();
    float beginSetting();
    void finalizeSetting();
    void setVal(bool newVal);
};
#endif //SERWER_SERVICE_H
