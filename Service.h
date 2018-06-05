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
    mutable std::mutex mutex;
    std::string name;
    std::string unit;
    Service(unsigned char id, std::string &name, std::string& unit);
public:
    static Service* serviceFactory(unsigned char id, unsigned char devClass, const  char *name,
                                   const char *unit, float min, float max);
    virtual ~Service() = default;
    unsigned char getId() const;
    const std::string& getName() const ;
    const std::string& getUnit() const ;
    float getMin() const ;
    float getMax() const ;
};

class AnalogIn: public Service {
private:
    float val;
    time_t timestamp;
public:
    AnalogIn(unsigned char id, std::string&& name, std::string&& unit): Service(id, name, unit),
                                                                        val(0), timestamp(0) {}
    float getVal() const;
    void setVal(float newVal);
    time_t getTimestamp() const;
    void setTimestamp(time_t time);
};

class DigitalIn: public Service {
private:
    bool val;
    time_t timestamp;
public:
    DigitalIn(unsigned char id, std::string&& name, std::string&& unit): Service(id, name, unit),
                                                                         val(false), timestamp(0) {}
    bool getVal() const;
    void setVal(float newVal);
    time_t getTimestamp() const;
    void setTimestamp(time_t time);
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
    float getVal() const;
    bool beginSetting(float *val);
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
    bool getVal() const;
    bool beginSetting(float *val);
    void finalizeSetting();
    void setVal(bool newVal);
};
#endif //SERWER_SERVICE_H
