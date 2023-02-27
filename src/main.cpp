/** This example uses the Zumo's line sensors to detect the white
border around a sumo ring.  When the border is detected, it
backs up and turns. */


#include <Wire.h>
#include <Zumo32U4.h>


// This might need to be tuned for different lighting conditions,
// surfaces, etc.
int QTR_THRESHOLD_TRACK_LEFT  =  210;  // microseconds
int QTR_THRESHOLD_TRACK_RIGHT =  300; // microseconds

int QTR_THRESHOLD_LEFT   = 400; // microseconds
int QTR_THRESHOLD_MIDDLE = 100; // microseconds
int QTR_THRESHOLD_RIGHT  = 400; // microseconds

// These might need to be tuned for different motor types.
#define REVERSE_SPEED     100  // 0 is stopped, 400 is full speed
#define TURN_SPEED        100
int FORWARD_SPEED      =  100;
#define REVERSE_DURATION  200  // ms
#define TURN_DURATION     300  // ms

// Motor speed when turning.  400 is the max speed.
const uint16_t turnSpeed = 100;

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
boolean crashed = false;
int speed = 100;
boolean manualTakeOver = true;
boolean left_track = false;
boolean false_track = false;
boolean motor_on = true;

bool proxLeftActive;
bool proxFrontActive;
bool proxRightActive;


#define NUM_SENSORS 3
unsigned int lineSensorValues[NUM_SENSORS];


unsigned long startMillis;  //some global variables available anywhere in the program
unsigned long currentMillis;
const unsigned long period = 500; 

#include "Turnsensor.h"

void printGyro(int angle){
  static char buffer[10];
  sprintf(buffer, "<G:%d>",
    angle
  );
  Serial1.print(buffer);

}

void printConsoleVariable(String variable){
  static char buffer[80];
  sprintf(buffer, "<E:%s>",
    variable.c_str()
  );
  Serial1.println(buffer);

}

void printLineSensors(int line1, int line2, int line3){
  static char buffer[80]; 
  sprintf(buffer, "<L:%d,%d,%d>",
    line1,
    line2,
    line3
  );
  Serial1.println(buffer);

}

void printEncoders(int countsLeft, int countsRight, bool errorLeft, bool errorRight){
  static char buffer[80]; 
  sprintf(buffer, "<R:%d,%d,%d,%d>",
    countsLeft,
    countsRight,
    errorLeft,
    errorRight
  );
  Serial1.println(buffer);
  Serial.println(buffer);
}


void drive(int leftMotor,int rightMotor){
  if(motor_on){
    motors.setSpeeds(leftMotor, rightMotor);
  }
}


// Modified a version of reading different incoming information from serial
// Robin2. (2014, December 1). Serial input basics. Arduino Forum. Retrieved February 26, 2023, from https://forum.arduino.cc/t/serial-input-basics/278284/73 
const byte numChars = 32;
char incomingChars[numChars]; // an array to store the received data
boolean incomingMessage = false;
char commandType;//[32] = {0};
//String commandType;

void retrieveSerial() {
  static boolean recvInProgress = false;
  static boolean commandReceived = false;

    static byte ndx = 0;
    char rc;
    

    while (Serial1.available() > 0 && incomingMessage == false) {
        rc = Serial1.read();

        if (recvInProgress == true) {
          // if doesn't = the end  of message incoming ---
            if (rc != '>') {
               /*if(commandReceived){
                // only add message after : don't include :
                if(rc != ':' ){*/
                if(!commandReceived){
                    commandType = rc;
                    commandReceived = true;

                }else{
                  if(rc != ':' ){
                    incomingChars[ndx] = rc;
                    ndx++;
                    if (ndx >= numChars) {
                        ndx = numChars - 1;
                    }
                  }
                }
            }
            else {
              
                incomingChars[ndx] = '\0'; // terminate the string
                recvInProgress = false;
                ndx = 0;
                incomingMessage = true;
                commandReceived = false;
                
            }
        }
        
        else if (rc == '<') {
            recvInProgress = true;
        }
    }
}  

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




void turn(char dir, int angle)
{

  turnSensorReset();

  uint8_t sensorIndex;

  switch(dir)
  {
  case 'B':
    // Turn left 125 degrees using the gyro.
    drive(-turnSpeed, turnSpeed);
    while((int32_t)turnAngle < turnAngle45 * 3)
    {
      turnSensorUpdate();
    }
    sensorIndex = 1;
    break;

  case 'L':
    // Turn left 45 degrees using the gyro.
    //Serial1.println((int32_t)turnAngle);
    //Serial1.println("<E:Turn Left>");
    drive(-turnSpeed, turnSpeed);
    while((int32_t)turnAngle < turnAngle1 * angle)//((int32_t)turnAngle < angle)//
    {
      //printGyro((((int32_t)turnAngle >> 16) * 360) >> 16);
      //Serial1.println(turnAngle);
      turnSensorUpdate();
    }
    sensorIndex = 1;
    break;

  case 'R':
    //Serial1.println("<E:Turn Right>");

    drive(turnSpeed, -turnSpeed);
    
    while((int32_t)turnAngle > -turnAngle1 * angle)
    {
      //printGyro((((int32_t)turnAngle >> 16) * 360) >> 16);
      turnSensorUpdate();
    }
    sensorIndex = 3;
    break;
  case 'F':
    // Turn right 45 degrees using the gyro.
    drive(turnSpeed, -turnSpeed);
    while((int32_t)turnAngle > -turnAngle45)
    {
      //printGyro((((int32_t)turnAngle >> 16) * 360) >> 16);
      turnSensorUpdate();
    }
    sensorIndex = 1;
    break;

  default:
    // This should not happen.
    return;
  }
  drive(0,0);

  // Turn the rest of the way using the line sensors.
  //while(1)
  //{
   // readSensors();
    //if (aboveLine(sensorIndex))
    //{
    //  // We found the line again, so the turn is done.
    //  break;
    //}
    
  //}
}

// Prints a line with all the sensor readings to the serial
// monitor.
void printReadingsToSerial()
{
  static char buffer[80];
  sprintf(buffer, "<P:%d,%d,%d,%d,%d,%d>",
    proxSensors.countsLeftWithLeftLeds(),
    proxSensors.countsLeftWithRightLeds(),
    proxSensors.countsFrontWithLeftLeds(),
    proxSensors.countsFrontWithRightLeds(),
    proxSensors.countsRightWithLeftLeds(),
    proxSensors.countsRightWithRightLeds()
  );
  Serial1.println(buffer);
  Serial.println(buffer);
}

char tempChars[32]; 

void parseSliders ( int *sliders, int quantity){

  char* ptr = strtok(incomingChars, ",");
  sliders[0] = atol(ptr);
  ptr = strtok(NULL, ",");
  sliders[1] = atol(ptr);

  if(quantity = 3){
    ptr = strtok(NULL, ",");
    sliders[2] = atol(ptr);
  }

}

void setup()
{
  Serial.begin(9600);
  Serial1.begin(9600);


  bool usbPower = usbPowerPresent();

  uint16_t batteryLevel = readBatteryMillivolts();

  if(!usbPower){
    printConsoleVariable(("Battery Level " + String(batteryLevel) + " mv"));
  }

  // Uncomment if necessary to correct motor directions:
  //motors.flipLeftMotor(true);
  //motors.flipRightMotor(true);
  
  lineSensors.initThreeSensors();

  proxSensors.initThreeSensors();

  //proxSensors.setPeriod(420);
  //proxSensors.setPulseOnTimeUs(421);
  //proxSensors.setPulseOffTimeUs(578);
  //uint16_t levels[] = { 4, 15, 32, 55, 85, 120 };
  //proxSensors.setBrightnessLevels(levels, sizeof(levels)/2);
  
  //buttonB.waitForButton();
  //lineSensorSetup();
  //calibrateSensors();
    // Calibrate the gyro and show readings from it until the user
  // presses button A.

//TODO UNCOMMENT
  /*
  buttonB.waitForButton();
  turnSensorSetup();
  
  lineSensorSetup();
  
*/
  
buttonB.waitForButton();
startMillis = millis(); 
}




void loop()
{


  retrieveSerial();
  //parseCommand();

  if(incomingMessage){
    Serial.println("type: ");
    Serial.println(commandType);
    Serial.println("Incoming: ");
    Serial.println(incomingChars);

    //Serial1.println("<E:received>");

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

      //Serial1.println("<E:command W received>");
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

  }

  /*static uint16_t lastSampleTime = 0;
  // checks when last updated and holds off for less frequent refreshing
  if ((uint16_t)(millis() - lastSampleTime) >= 100)
  {
    lastSampleTime = millis();
    proxSensors.read();
    printReadingsToSerial();
  }
*/



  lineSensors.read(lineSensorValues);

  //printLineSensors(lineSensorValues[0], lineSensorValues[1],lineSensorValues[2]); 

  currentMillis = millis();  //get the current "time" (actually the number of milliseconds since the program started)
  if (currentMillis - startMillis >= period)  //test whether the period has elapsed
  {

    startMillis = currentMillis;  //IMPORTANT to save the start time of the current LED state.


    int16_t countsLeft = encoders.getCountsLeft();
    int16_t countsRight = encoders.getCountsRight();

    bool errorLeft = encoders.checkErrorLeft();
    bool errorRight = encoders.checkErrorRight();;

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
    }

    printEncoders(countsLeft, countsRight, errorLeft, errorRight);

  }



  if(!manualTakeOver){


    //delay(1000);

    if (lineSensorValues[1] > QTR_THRESHOLD_MIDDLE) {// CHECK CENTER 
      
      ledGreen(1);
      ledRed(1);
      ledYellow(1);
      if(!crashed){
        printConsoleVariable("Crashed Middle");
      }
      crashed = true;
      drive(0, 0);
      turn('F', 1);
    } else if (lineSensorValues[0] > QTR_THRESHOLD_TRACK_LEFT) {// CHECK LEFT
      ledGreen(1);

      // if too far over line turn off line
      if (lineSensorValues[0] > QTR_THRESHOLD_LEFT) {// CHECK LEFT
        ledGreen(0);
        ledRed(0);
        ledYellow(1);
        
        
        if(!crashed){
          printConsoleVariable("Crashed Left");
        }
        crashed = true;
        turn('R', 1);
      }else{
        // follow line 

        crashed = false;
        left_track = true;
        drive(FORWARD_SPEED, FORWARD_SPEED);
      }

      // If leftmost sensor detects line, reverse and turn to the
      // right.
      //motors.setSpeeds(-REVERSE_SPEED, -REVERSE_SPEED);
      //delay(REVERSE_DURATION);
      //motors.setSpeeds(TURN_SPEED, -TURN_SPEED);
      //delay(TURN_DURATION);
      //motors.setSpeeds(FORWARD_SPEED, FORWARD_SPEED);
    } else if (lineSensorValues[2] > QTR_THRESHOLD_RIGHT) {// CHECK RIGHT
      ledGreen(0);
      ledRed(1);
      ledYellow(0);

      if(!crashed){
        printConsoleVariable("Crashed Right");
      //Serial1.println("<E:Crashed Right>");
      }
      crashed = true;
      turn('L', 1);

      // If rightmost sensor detects line, reverse and turn to the
      // left.
      //motors.setSpeeds(-REVERSE_SPEED, -REVERSE_SPEED);
      //delay(REVERSE_DURATION);
      //motors.setSpeeds(-TURN_SPEED, TURN_SPEED);
      //delay(TURN_DURATION);
      //motors.setSpeeds(FORWARD_SPEED, FORWARD_SPEED);
    } else {

      if(left_track == true){
        // if left wall is lost turn left to try and find it
        turn('L', 1);
      }else{

      crashed = false;
      ledGreen(0);
      ledRed(0);
      ledYellow(0);
      // Otherwise, go straight.
      drive(FORWARD_SPEED, FORWARD_SPEED);
      }
    }
    //todo add else crashed back in
  //}else{
    //TODO PUT CONTROL MOVEMENT INSIDE IF STOPPING IT FROM BEING CONTROLLED WHEN AUTOMATED 
  //}
  }else{

    if(incomingMessage){
      //Serial.println("Type: ");
      ///Serial.println(commandType);
      //Serial.println(incomingChars);


      switch (commandType){
        case 'A':
          ledYellow(1);
          motors.setLeftSpeed(speed);
          motors.setRightSpeed(speed*-1);

          delay(70);
          break;
        case 'S':
          ledYellow(1);
          motors.setLeftSpeed(speed*-1);
          motors.setRightSpeed(speed*-1);

          delay(70);
          break;
        case 'D':
          ledRed(1);
          motors.setRightSpeed(speed);
          motors.setLeftSpeed(speed*-1);

          delay(70);
          break;
        case 'W':
          ledRed(1);
          motors.setLeftSpeed(speed);
          motors.setRightSpeed(speed);

          delay(70);
          break;
        default:
          motors.setLeftSpeed(0);
          motors.setRightSpeed(0);
          ledYellow(0);
          ledRed(0);
      }
    }
    motors.setLeftSpeed(0);
    motors.setRightSpeed(0);
    ledYellow(0);
    ledRed(0);

  }

  
  incomingMessage = false;

}




/*
#include <Wire.h>
#include <Zumo32U4.h>


// Modified a version of reading different incoming information from serial
// Robin2. (2014, December 1). Serial input basics. Arduino Forum. Retrieved February 26, 2023, from https://forum.arduino.cc/t/serial-input-basics/278284/73 
const byte numChars = 32;
char incomingChars[numChars]; // an array to store the received data
boolean incomingMessage = false;
char commandType;

void retrieveSerial() {
    static boolean recvInProgress = false;
    boolean commandReceived = false;

    static byte ndx = 0;
    char rc;
    commandType = ' ';

    while (Serial1.available() > 0 && incomingMessage == false) {
        rc = Serial1.read();

        if (recvInProgress == true) {
          // if doesn't = the end  of message incoming ---
            if (rc != '>') {
               if(commandReceived){
                // only add message after : don't include :
                if(rc != ':' ){
                  incomingChars[ndx] = rc;
                  ndx++;
                  if (ndx >= numChars) {
                      ndx = numChars - 1;
                  }
                }
              }else if(rc != '<'){
                  commandType = rc;
                  commandReceived = true;
              }
                
            }
            else {
              
                incomingChars[ndx] = '\0'; // terminate the string
                recvInProgress = false;
                ndx = 0;
                incomingMessage = true;
                commandReceived = false;
                
            }
        }
        
        else if (rc == '<') {
            recvInProgress = true;
        }
    }
}

void setup()
{
  Serial.begin(9600);
  Serial1.begin(9600);

  
}

void loop(){
  retrieveSerial();


  if(incomingMessage){
    Serial.println("Type: ");
    Serial.println(commandType);
    Serial.println(incomingChars);
    incomingMessage = false;
    Serial1.println("<recieved>");

  if(commandType == 'h'){
    Serial1.println("<command h recieved>");
  }

    if(commandType == 'd'){
    Serial1.println("<command d recieved>");
  }
  }else{
      Serial1.println("<nothing to report>");
      
  }

}


*/