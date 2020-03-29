/*
 Name:		SlowSawtooth.ino
 Created:	3/28/2020 10:44:36 PM
 Author:	Sam Verstraete
*/

#include <TimerFour.h>

// the setup function runs once when you press reset or power the board
void setup() {
    Timer4.initialize(100); //10kHz
    Serial.begin(115200);
}

// the loop function runs over and over again until power down or reset
void loop() {
    for (int i = 0; i < 1023; i++) {
        Timer4.pwm(9, i);
        Serial.println(i);
        delay(10);
    }
}
