#include <Arduino.h>
#include <MyServo.h>

#define MOTOR_FIRST_PIN 26
#define MOTOR_SECOND_PIN 25
#define POT_PIN 36
#define BUTTON 18

MyServo servo;

void setup() {
    Serial.begin(115200);
    servo = MyServo(MOTOR_FIRST_PIN, MOTOR_SECOND_PIN, POT_PIN); 
    pinMode(BUTTON, INPUT_PULLUP);
}

void loop() {
    if(!digitalRead(BUTTON)) {
        servo.setAngle(0);
        delay(1000);
        servo.setAngle(180);
        delay(1000);
    } else {
        delay(500);
    }
}