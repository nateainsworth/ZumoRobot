/** This example uses the Zumo's line sensors to detect the white
border around a sumo ring.  When the border is detected, it
backs up and turns. */


#include <Wire.h>
#include <Zumo32U4.h>

// This might need to be tuned for different lighting conditions,
// surfaces, etc.
int QTR_THRESHOLD_TRACK_LEFT  =  210; 
int QTR_BOUND_TRACK_LEFT = 260;
int QTR_THRESHOLD_TRACK_RIGHT =  300; 

int QTR_THRESHOLD_LEFT   = 550; // was 400
int QTR_THRESHOLD_MIDDLE = 140; 
int QTR_THRESHOLD_RIGHT  = 400; 


// Motor speed when turning during line sensor calibration.
const uint16_t calibrationSpeed = 200;


Zumo32U4ButtonB buttonB;
Zumo32U4Buzzer buzzer;
Zumo32U4Motors motors;
Zumo32U4LineSensors lineSensors;
Zumo32U4ProximitySensors proxSensors;
Zumo32U4Encoders encoders;
Zumo32U4IMU imu;

const char encoderErrorLeft[] PROGMEM = "!<c2";
const char encoderErrorRight[] PROGMEM = "!<e2";
int errorPrinted = 0;


char action;
bool crashed = false;
int speed = 80;
bool manualTakeOver = false;
bool left_track = false;
bool false_track = false;
bool motor_on = true;

bool proxLeftActive;
bool proxFrontActive;
bool proxRightActive;

bool maneuver_crash = false;


#define NUM_SENSORS 3
unsigned int lineSensorValues[NUM_SENSORS];


unsigned long startMillis;  //some global variables available anywhere in the program
unsigned long currentMillis;
const unsigned long period = 150; 
int sensorTurn = 1;



enum Mode{
  ModeOne,
  ModeTwo,
  ModeThree,
};

char importantMessageBuffer[32]; 
bool importantReceived = true;
int movementCount = 0;

#include "MessageHandler.h"
#include "Turnsensor.h"
#include "Travel.h"

#include "ModeOne.h"
#include "ModeTwo.h"
//#include "ModeThree.h"



// Calibrates the line sensors by turning left and right, then
// shows a bar graph of calibrated sensor readings on the display.
// Returns after the user presses A.
void lineSensorSetup()
{
  //display.clear();
  //display.print(F("Line cal"));

  // Delay so the robot does not move while the user is still
  // touching the button.
  delay(1000);

  // We use the gyro to turn so that we don't turn more than
  // necessary, and so that if there are issues with the gyro
  // then you will know before actually starting the robot.

  //turnSensorReset();
  
  // Turn to the left 90 degrees.
  motors.setSpeeds(-calibrationSpeed, calibrationSpeed);
  while((int32_t)turnAngle < turnAngle45 * 2)
  {
    printGyro((((int32_t)turnAngle >> 16) * 360) >> 16);
    lineSensors.calibrate();
    turnSensorUpdate();
  }

  // Turn to the right 90 degrees.
  motors.setSpeeds(calibrationSpeed, -calibrationSpeed);
  while((int32_t)turnAngle > -turnAngle45 * 2)
  {
    printGyro((((int32_t)turnAngle >> 16) * 360) >> 16);
    lineSensors.calibrate();
    turnSensorUpdate();
  }

  // Turn back to center using the gyro.
  motors.setSpeeds(-calibrationSpeed, calibrationSpeed);
  while((int32_t)turnAngle < 0)
  {
    printGyro((((int32_t)turnAngle >> 16) * 360) >> 16);
    lineSensors.calibrate();
    turnSensorUpdate();
  }

  // Stop the motors.
  motors.setSpeeds(0, 0);
}


void readEncoders(bool printReadings){

    int16_t countsLeft = encoders.getCountsLeft();
    int16_t countsRight = encoders.getCountsRight();

    bool errorLeft = encoders.checkErrorLeft();
    bool errorRight = encoders.checkErrorRight();
/*
    if(errorLeft || errorRight){
      if(errorPrinted == 0){
        if (errorLeft)
        { 
          errorPrinted = 10;
          printConsoleVariable("Left Encoder Error");
        }

        if (errorRight)
        {
          errorPrinted = 10;
          printConsoleVariable("Right Encoder Error");
        }
      }else{
        errorPrinted--;
      }
    }*/
    if(printReadings){
      printEncoders(countsLeft, countsRight, errorLeft, errorRight);
    }
}

// todo remove test bool
bool test = false;

void setup()
{
  Serial.begin(9600);
  Serial1.begin(9600);

  bool usbPower = usbPowerPresent();
  uint16_t batteryLevel = readBatteryMillivolts();

  if(!usbPower){
   // printConsoleVariable(("Battery Level " + String(batteryLevel) + "mv"));
  }
  
  lineSensors.initThreeSensors();

  proxSensors.initThreeSensors();

  uint16_t levels[] = { 4, 15, 32, 55, 85, 95, 120 };
  proxSensors.setBrightnessLevels(levels, sizeof(levels)/2);

  //proxSensors.setPeriod(420);
  //proxSensors.setPulseOnTimeUs(421);
  //proxSensors.setPulseOffTimeUs(578);
  //uint16_t levels[] = { 4, 15, 32, 55, 85, 120 };
  //proxSensors.setBrightnessLevels(levels, sizeof(levels)/2);
  

//TODO remove test
  //if(!test){
  buttonB.waitForButton();

  turnSensorSetup();
  
  //lineSensorSetup();
  
  
  //buttonB.waitForButton();
  startMillis = millis(); 
  lastDetectionMillis  = millis();
  lastPersonCheckMillis = millis();
//}else{
 // buttonB.waitForButton();
//}
}


void loop()
{


  retrieveSerial();

  if(incomingMessage){
    Serial.println("type: ");
    Serial.println(commandType);
    Serial.println("Incoming: ");
    Serial.println(incomingChars);

    if(commandType == 'M'){

      switch (incomingChars[0]){
          case '1':
            manualTakeOver = true;
            break;
          case '2':
            break;
          case '3':
            manualTakeOver = false;
            break;
          case '4':
            motor_on = !motor_on;
            Serial.print("motor changed");
            break;
          default:
            manualTakeOver = true;
      }
    }

    if(commandType == 'U'){
       FORWARD_SPEED = (int)incomingChars;
    }
    
    if(commandType == 'I'){
     /*int sliders [2];
      parseSliders(sliders, 2);
      QTR_THRESHOLD_TRACK_LEFT = sliders[0];
      QTR_THRESHOLD_TRACK_RIGHT = sliders[1];*/
      updateLowSliders();
    }

    if(commandType == 'O'){
      /*int sliders [3];
      parseSliders(sliders, 3);
      QTR_THRESHOLD_LEFT  = sliders[0];
      QTR_THRESHOLD_MIDDLE = sliders[1];
      QTR_THRESHOLD_RIGHT = sliders[2];
      */
     updateMaxSliders();
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
      Serial1.println(importantMessageBuffer);
    }

  }

  lineSensors.read(lineSensorValues);
  
  
  currentMillis = millis();

  if(currentMillis - startMillis >= 150){
    proxSensors.read();
  }


  if (currentMillis - startMillis >= period)
  {
    startMillis = currentMillis;
    //printConsoleVariable("sending values");

    //int16_t countsLeft = encoders.getCountsLeft();
    //int16_t countsRight = encoders.getCountsRight();

    //bool errorLeft = encoders.checkErrorLeft();
    //bool errorRight = encoders.checkErrorRight();

    //printAllSensors(0, 0, 0, 0, lineSensorValues[0], lineSensorValues[1], lineSensorValues[2]);

    //printProximity();

 printLineSensors(lineSensorValues[0], lineSensorValues[1], lineSensorValues[2]);
 printProximity();
 readEncoders(true);
 /*
    switch(sensorTurn){
      case 1:
        printLineSensors(lineSensorValues[0], lineSensorValues[1], lineSensorValues[2]);
        sensorTurn = 2;
      break;
      case 2:
        printProximity();
        sensorTurn = 3;
      break;
      case 3: 
        readEncoders(true);
        sensorTurn = 1;
      break;
      }
*/
  }



  //while(noSensors have been detected run mode)

  if(!manualTakeOver){
    if(test)
    {
      //buttonB.waitForButton();
      //proximityPersonCheck();


      //forward(0.8);
 
      //buttonB.waitForButton();
      //turn('L', 2);
      //test = false;
    }else{
      //runModeThree();
      runModeTwo();

          
    }   


  }else{

    if(incomingMessage){

      manualMove(commandType, false);

  }

  incomingMessage = false;

  }
}
