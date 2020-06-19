// includes
#include <SoftwareSerial.h>
#include "Adafruit_Soundboard.h"
#include <Servo.h>

//soundboard pin definitions
#define SFX_TX 6
#define SFX_RX 5
#define SFX_RST 8

// software serial
SoftwareSerial ss = SoftwareSerial(SFX_TX, SFX_RX);
Adafruit_Soundboard sfx = Adafruit_Soundboard(&ss, NULL, SFX_RST);

// variables soundboard


// objects
Servo myservo;

// variables
// general
int inputPin = A0;

// calibration
int sensorValue = 0;
int sensorMin = 1023;
int sensorMax = 0;

// pir
int pirInput = 3;
int pirState = LOW;
int pirStatus = 0;
int triggerDelay = 1000;
int triggerReset = 5000;

// smoothing
const int numReadings = 10;
int readings = 0;
int total = 0;
int average = 0;

// servo
int pos = 0;
int servoPin = 4;
int mouthClosed = 20;
int mouthMid = 10;
int mouthOpen = 0;
int moveOpenDelay = 150;
int moveCloseDelay = 100;

// sound playback
uint8_t randomTrack = 0;
unsigned long currentMillis = millis();
unsigned long trackOnTime;
unsigned long trackDuration;

void setup() {
  // soundboard
  ss.begin(9600);
  // initialize serial communication with computer:
  Serial.begin(9600);

  // calibrate the audio input
  // calibrate during the first ten seconds
  sfx.playTrack(randomTrack);
  while (millis() < 10000) {
    sensorValue = analogRead(inputPin);
    // record the maximum sensor value
    if (sensorValue > sensorMax) {
      sensorMax = sensorValue;
    }
    // record the minimum sensor value
    if (sensorValue < sensorMin) {
      sensorMin = sensorValue;
    }
  }
  sfx.stop();
  //Serial.write("SensorMin = ");
  //Serial.println(sensorMin);
  //Serial.write("SensorMax = ");
  //Serial.println(sensorMax);
  //delay(1000);

  // attach servo to pin
  myservo.attach(servoPin);
  myservo.write(mouthClosed);

  // pir
  pinMode(pirInput, INPUT);

  // random seed
  randomSeed(analogRead(A0));
}
void loop() {
  // check for motion
  pirState = digitalRead(pirInput);
  Serial.println(pirState);

  if (pirState == HIGH) {

    // wait some duration after detecting motion
    delay(triggerDelay);

    //pick random track 0-3
    randomTrack = random(4);

    if (randomTrack == 0) {
      trackDuration = 22754;
    } else if (randomTrack == 1) {
      trackDuration = 22757;
    } else if (randomTrack == 2) {
      trackDuration = 24396;
    } else if (randomTrack == 3) {
      trackDuration = 20354;
    }

    //Serial.println(randomTrack);
    sfx.playTrack(randomTrack);
    trackOnTime = millis();
    currentMillis = millis();

    //Serial.println(trackDuration);
    //Serial.println(trackOnTime);
    //Serial.println(currentMillis);
    while (currentMillis - trackOnTime <= trackDuration) {
      for (int i = 0; i <= numReadings; i++) {
        readings = analogRead(inputPin);
        total = total + readings;
      }
      // calculate the average
      sensorValue = total / numReadings;
      // zero total
      total = 0;
      //Serial.write("SensorValue = ");
      //Serial.println(sensorValue);

      // apply the calibration to the sensor reading
      sensorValue = map(sensorValue, sensorMin, sensorMax, 0, 1000);
      // in case the sensor value is outside the range seen during calibration
      sensorValue = constrain(sensorValue, 0, 1000);
      //Serial.write("SensorValueMapped = ");
      //Serial.println(sensorValue);

      // move the mouth based on averaged and smoothed sensor reading
      if (sensorValue <= 10) {
        //myservo.write(mouthClosed);
        //delay(moveCloseDelay);
      } else if (sensorValue > 10 && sensorValue <= 100) {
        myservo.write(mouthMid);
        delay(moveOpenDelay);
        myservo.write(mouthClosed);
        delay(moveCloseDelay);
      } else if (sensorValue > 100) {
        myservo.write(mouthOpen);
        delay(moveOpenDelay);
        myservo.write(mouthClosed);
        delay(moveCloseDelay);
      }
      currentMillis = millis();

    }

    // wait some duration before checking for movement again
    delay(triggerReset);

  }
}
