#include <Wire.h>
#include <Zumo32U4.h>

Zumo32U4ButtonB buttonB;
Zumo32U4Buzzer buzzer;
Zumo32U4Motors motors;
Zumo32U4LineSensors lineSensors;
Zumo32U4ProximitySensors proxSensors;
Zumo32U4Encoders encoders;
Zumo32U4IMU imu;

//setup sensors
#define NUM_SENSORS 3
unsigned int lineSensorValues[NUM_SENSORS];
bool proxLeftActive;
bool proxFrontActive;
bool proxRightActive;

//setup Thresholds
int QTR_THRESHOLD_TRACK_LEFT  =  210; 
int QTR_THRESHOLD_TRACK_RIGHT =  300; 

int QTR_THRESHOLD_LEFT   = 580;// was 550
int QTR_THRESHOLD_MIDDLE = 140; 
int QTR_THRESHOLD_RIGHT  = 400; 

//TODO SET TO 0 
int driveMode = 1;

// Drive variables
bool left_track = false;
bool motor_on = true;

// sensor count downs
unsigned long startMillis;
unsigned long currentMillis;
const unsigned long period = 150; 

// Message system for checking messages on movement have been recorded
char importantMessageBuffer[32]; 
bool importantReceived = true;
int movementCount = 0;

#include "MessageHandler.h"
#include "Turnsensor.h"
#include "Travel.h"
#include "ModeTwo.h"
#include "ModeThree.h"
#include "ModeOne.h"




void readEncoders(bool printReadings){

    int16_t countsLeft = encoders.getCountsLeft();
    int16_t countsRight = encoders.getCountsRight();

    bool errorLeft = encoders.checkErrorLeft();
    bool errorRight = encoders.checkErrorRight();

    if(printReadings){
      printEncoders(countsLeft, countsRight, errorLeft, errorRight);
    }
}

void handleIncomingMessage(){
    Serial.println("type: ");
    Serial.println(commandType);
    Serial.println("Incoming: ");
    Serial.println(incomingChars);

    if(commandType == 'M'){

      switch (incomingChars[0]){
          case '1':
            driveMode = 1;
            modeTwoTakeOver = false;
            break;
          case '2':
            state = FindLeft;
            previous_state = Starting;
            driveMode = 2;
            break;
          case '3':
            state = FindLeft;
            previous_state = Starting;
            modeTwoTakeOver = false;
            driveMode = 3;
            break;
          case '4':
            motor_on = !motor_on;
            Serial.print("motor changed");
            break;
          default:
            driveMode = 0;
            break;
      }
    }

    if(commandType == 'U'){
      FORWARD_SPEED = (int)incomingChars;
    }
    
    if(commandType == 'I'){
     int sliders [2];
      parseSliders(sliders, 2);
      QTR_THRESHOLD_TRACK_LEFT = sliders[0];
      QTR_THRESHOLD_TRACK_RIGHT = sliders[1];
    }

    if(commandType == 'O'){
      int sliders [3];
      parseSliders(sliders, 3);
      QTR_THRESHOLD_LEFT  = sliders[0];
      QTR_THRESHOLD_MIDDLE = sliders[1];
      QTR_THRESHOLD_RIGHT = sliders[2];
    }

    if(commandType == 'Q'){
      if((int)incomingMessage == movementCount){
        importantReceived = true;
      }else{
        Serial.println("error with movements");
      }   
    }
    //if movement message was corrupted resend;
    if(commandType == 'X'){
      //Serial1.println(importantMessageBuffer);
    }

}

void setup()
{
  Serial.begin(9600);
  Serial1.begin(9600);

  bool usbPower = usbPowerPresent();
  uint16_t batteryLevel = readBatteryMillivolts();

  if(!usbPower){
    printConsoleVariable(("Battery Level " + String(batteryLevel) + "mv"));
  }
  
  lineSensors.initThreeSensors();

  proxSensors.initThreeSensors();

  uint16_t levels[] = { 4, 15, 32, 55, 85, 95, 120 };
  proxSensors.setBrightnessLevels(levels, sizeof(levels)/2);
  buttonB.waitForButton();
  turnSensorSetup();
  
  startMillis = millis(); 
  lastPersonCheckMillis = millis();

}


void loop()
{
  retrieveSerial();

  if(incomingMessage){
    handleIncomingMessage();
  }

  lineSensors.read(lineSensorValues);
  currentMillis = millis();

  if(currentMillis - startMillis >= 150){
    proxSensors.read();
  }

  if (currentMillis - startMillis >= period)
  {
    startMillis = currentMillis;
    printLineSensors(lineSensorValues[0], lineSensorValues[1], lineSensorValues[2]);
    printProximity();
    readEncoders(true);
  }

  switch(driveMode){
    case 1:
     
      runModeOne();
      break;
    case 2:
      
      runModeTwo();
      break;
    case 3:
      
      runModeThree();
      break;
    default:
      delay(1000);
      break;

  }

  incomingMessage = false;
}
