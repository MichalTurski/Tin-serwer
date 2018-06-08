//
// Created by michal on 19.05.18.
//

#ifndef SERWER_SERVICE_H
#define SERWER_SERVICE_H

#include <mutex>

class Service {
private:
    unsigned char id;
protected:
    enum serviceType {
        ANALOG_IN = 0x00,
        ANALOG_OUT = 0x01,
        DIGITAL_IN = 0x02,
        DIGITAL_OUT = 0x03
    };

    mutable std::mutex mutex;
    serviceType type;
    std::string name;
    std::string unit;
    const float min;
    const float max;
    float val;
    Service(unsigned char id, serviceType type, std::string &name, std::string& unit, float min,
            float max);
public:
    static Service* serviceFactory(unsigned char id, unsigned char srvType, const  char *name,
                                   const char *unit, float min, float max);
    virtual ~Service() = default;
    unsigned char getId() const;
    const std::string& getName() const;
    const std::string& getUnit() const;
    float getVal() const;
    float getMin() const;
    float getMax() const;
    uint8_t getType() const;
};

class Input: public Service {
private:
    time_t timestamp;
public:
    Input(unsigned char id, serviceType type, std::string&& name, std::string&& unit, float min, float max):
            Service(id, type, name, unit, min, max), timestamp(0) {}
    void setVal(float newVal);
    time_t getTimestamp() const;
    void setTimestamp(time_t time);
};

class Output: public Service {
private:
    float expected;//this value will be set in next connection
    float inProgress;//this value is being set, if succeed current will be equal to inProgress
    bool change;
public:
    Output(unsigned char id, serviceType type, std::string&& name, std::string&& unit, float min, float max);
    bool beginSetting(float *val);
    void finalizeSetting();
    void setVal(float newVal);
};

#endif //SERWER_SERVICE_H
