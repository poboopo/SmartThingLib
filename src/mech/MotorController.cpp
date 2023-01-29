#include <mech/MotorController.h>

#define ACCURACY 100

#define MIN_SPEED 100
#define MAX_SPEED 150

#define TIMEOUT 5000

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

    ESP_LOGI(MOTOR_CONTROLLER_TAG, "p1: %u | p2: %u | pot: %u", motorFirstPin, motorSecondPin, potPin);
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
    int16_t currentState = analogRead(_potPin);
    if ((currentState > POT_MAX && turnToPosition > POT_MAX ) || (currentState < POT_MIN && turnToPosition < POT_MIN)) {
        return true;
    }

    if (turnToPosition > POT_MAX) {
        turnToPosition = POT_MAX;
    } else if (turnToPosition < POT_MIN) {
        turnToPosition = POT_MIN;
    }

    int16_t diff = currentState - turnToPosition;
    /*
        right - positive
        left - negative
    */
    int8_t direction = sign(diff); 
    if (direction * (currentState - turnToPosition) <  ACCURACY) {
        return true;
    }
    
    if (direction > 0) {
        digitalWrite(_motorSecondPin, HIGH);
    } else {
        digitalWrite(_motorSecondPin, LOW);
    }

    diff = direction * diff;
    uint8_t maxSpeed = calculateMaxSpeed(diff);
    diff = diff / 2;

    uint8_t speed = 0;
    int16_t startPosition = currentState;

    long started = millis();
    while (direction * (currentState - turnToPosition) >  ACCURACY && millis() - started < TIMEOUT) {
        currentState = analogRead(_potPin);

        if (direction > 0) {
            speed = 255 - calculateCurrentSpeed(startPosition - currentState, diff, maxSpeed);
        } else {
            speed = calculateCurrentSpeed(currentState - startPosition, diff, maxSpeed);
        }

        analogWrite(_motorFirstPin, speed);
    } 
    stop();
    return direction * (currentState - turnToPosition) <  ACCURACY;
}

void MotorController::setAngle(uint8_t angle) {
    int16_t turnToPosition = map(angle, 0, 180, POT_MIN, POT_MAX);
    setPosition(turnToPosition);
}

uint8_t MotorController::currentAngle() {
    uint16_t potValue = analogRead(_potPin);
    return map(potValue, POT_MIN, POT_MAX, 0, 180);
}

uint16_t MotorController::currentPosition() {
    return analogRead(_potPin);
}

int8_t MotorController::sign(int16_t value) {
    if (value < 0) {
        return -1;
    } else {
        return 1;
    }
}

void MotorController::stop() {
    analogWrite(_motorFirstPin, LOW);
    digitalWrite(_motorSecondPin, LOW);
}