#include <motor/MotorController.h>
#include <motor/utils.h>

#define MIN_SPEED 100
#define MAX_SPEED 200

#define TIMEOUT 3000

MotorController::MotorController(){
}

MotorController::~MotorController() {
    stop();
}

MotorController::MotorController(uint8_t motorFirstPin, uint8_t motorSecondPin, uint8_t potPin) {
    _motorFirstPin = motorFirstPin;
    _motorSecondPin = motorSecondPin;
    _potPin = potPin;

    pinMode(_motorFirstPin, OUTPUT);
    pinMode(_motorSecondPin, OUTPUT);
}

void MotorController::setAccuracy(uint16_t accuracy) {
    _accuracy = accuracy;
}

uint8_t validateSpeed(int16_t speed) {
    if (speed > MAX_SPEED) {
        return MAX_SPEED;
    } else if (speed < MIN_SPEED) {
        return MIN_SPEED;
    }
    return speed;
}

uint8_t calculateMaxSpeed(uint16_t diff) {
    return validateSpeed((MAX_SPEED * diff) / POT_MAX);
}

uint8_t calculateCurrentSpeed(int16_t x, int16_t diff, int16_t maxSpeed) {   
    if (SMOOTH_START) {
        return validateSpeed((x * maxSpeed * (2 * diff - x)) / (diff * diff) + MIN_SPEED);
    } else {
        return MAX_SPEED;
    }
}

bool MotorController::setPosition(uint16_t turnToPosition) {
    int16_t currentState = getPosition();
    if ((currentState > POT_MAX && turnToPosition > POT_MAX ) || (currentState < POT_MIN && turnToPosition < POT_MIN)) {
        return true;
    }
    turnToPosition = constrain(turnToPosition, POT_MIN, POT_MAX);
    if (abs(currentState - turnToPosition) <  _accuracy) {
        return true;
    }

    long started = millis();
    int pwm = 0;
    while (abs(currentState - turnToPosition) > _accuracy && millis() - started < TIMEOUT) {
        currentState = getPosition();
        float temp_ki = kI;
        if(abs(pwm) == 255) 
            temp_ki = 0.f;

        pwm = pid(currentState, turnToPosition, kP, temp_ki, kD, (millis()-started)/1000.f);
        pwm = constrain(pwm, -255, 255);
        setPWM(pwm);
    } 
    stop();
    return abs(currentState - turnToPosition) <  _accuracy;
}

void MotorController::setPWM(int16_t pwm) {
    if (pwm >= 0) {
        analogWrite(_motorFirstPin, abs(pwm));
        analogWrite(_motorSecondPin, LOW);
    } else {
        analogWrite(_motorFirstPin, LOW);
        analogWrite(_motorSecondPin, abs(pwm));
    }
}

void MotorController::setAngle(uint8_t angle) {
    int16_t turnToPosition = map(angle, 0, 180, POT_MIN, POT_MAX);
    setPosition(turnToPosition);
}

uint16_t MotorController::getPosition() {
    static uint16_t oldState = 0;
    int16_t currentState = analogRead(_potPin);
    currentState = lpFilter(oldState, currentState, ALPHA);
    oldState = currentState;
    return currentState;
}

void MotorController::stop() {
    analogWrite(_motorFirstPin, LOW);
    analogWrite(_motorSecondPin, LOW);
}