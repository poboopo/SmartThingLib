#include <Arduino.h>

class MyServo {
private:
    uint8_t _motorFirstPin;
    uint8_t _motorSecondPin;
    uint8_t _potPin;

    int8_t sign(int16_t);
    void stop();
public:
    MyServo(uint8_t, uint8_t, uint8_t);
    MyServo();
    ~MyServo();

    void setAngle(uint8_t angle);
    void setPosition(uint16_t turnToPosition);
    uint8_t currentAngle();
};
