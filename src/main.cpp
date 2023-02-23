/*
#include <Wire.h>
#include <Zumo32U4.h>

Zumo32U4Motors motors;
Zumo32U4Buzzer buzzer;
Zumo32U4ButtonA buttonA;
Zumo32U4ButtonB buttonB;
Zumo32U4LineSensors lineSensors;
char action;


int speed = 100;


void calibrateSensors()
{
  display.clear();

  // Wait 1 second and then begin automatic sensor calibration
  // by rotating in place to sweep the sensors over the line
  delay(1000);
  for(uint16_t i = 0; i < 120; i++)
  {
    if (i > 30 && i <= 90)
    {
      motors.setSpeeds(-200, 200);
    }
    else
    {
      motors.setSpeeds(200, -200);
    }

    lineSensors.calibrate();
  }
  motors.setSpeeds(0, 0);
}

void setup(){
  Serial1.begin(9600);
  Serial.begin(9600);
  

  buttonB.waitForButton();
  calibrateSensors();
  buttonB.waitForButton();

  buzzer.play("L16 cdegreg4");
  while(buzzer.isPlaying());
  
}

void loop(){

  if (Serial1.available() > 0){
    action = Serial1.read();
    //Serial1.print("I received: ");
    //Serial1.println(action);

    Serial1.write(1);

    switch (action){
      case 'a':
        ledYellow(1);
        motors.setLeftSpeed(speed);
        motors.setRightSpeed(speed*-1);

        break;
      case 's':
        ledYellow(1);
        motors.setLeftSpeed(speed*-1);
        motors.setRightSpeed(speed*-1);
  
        break;
      case 'd':
        ledRed(1);
        motors.setRightSpeed(speed);
        motors.setLeftSpeed(speed*-1);

        break;
      case 'w':
        ledRed(1);
        motors.setLeftSpeed(speed);
        motors.setRightSpeed(speed);

        break;
      default:
        motors.setLeftSpeed(0);
        motors.setRightSpeed(0);
        ledYellow(0);
        ledRed(0);
    }



  }

}
*/

/* This example uses the line sensors on the Zumo 32U4 to follow
a black line on a white background, using a PID-based algorithm.
It works decently on courses with smooth, 6" radius curves and
has been tested with Zumos using 75:1 HP motors.  Modifications
might be required for it to work well on different courses or
with different motors.

This demo requires a Zumo 32U4 Front Sensor Array to be
connected, and jumpers on the front sensor array must be
installed in order to connect pin 4 to DN4 and pin 20 to DN2. */
/*
#include <Wire.h>
#include <Zumo32U4.h>

// This is the maximum speed the motors will be allowed to turn.
// A maxSpeed of 400 lets the motors go at top speed.  Decrease
// this value to impose a speed limit.
const uint16_t maxSpeed = 200;

Zumo32U4Buzzer buzzer;
Zumo32U4LineSensors lineSensors;
Zumo32U4Motors motors;
Zumo32U4ButtonB buttonB;

// Change next line to this if you are using the older Zumo 32U4
// with a black and green LCD display:
// Zumo32U4LCD display;
Zumo32U4OLED display;

int16_t lastError = 0;

#define NUM_SENSORS 5
unsigned int lineSensorValues[NUM_SENSORS];

// Sets up special characters for the display so that we can show
// bar graphs.
void loadCustomCharacters()
{
  static const char levels[] PROGMEM = {
    0, 0, 0, 0, 0, 0, 0, 63, 63, 63, 63, 63, 63, 63
  };
  display.loadCustomCharacter(levels + 0, 0);  // 1 bar
  display.loadCustomCharacter(levels + 1, 1);  // 2 bars
  display.loadCustomCharacter(levels + 2, 2);  // 3 bars
  display.loadCustomCharacter(levels + 3, 3);  // 4 bars
  display.loadCustomCharacter(levels + 4, 4);  // 5 bars
  display.loadCustomCharacter(levels + 5, 5);  // 6 bars
  display.loadCustomCharacter(levels + 6, 6);  // 7 bars
}

void printBar(uint8_t height)
{
  if (height > 8) { height = 8; }
  const char barChars[] = {' ', 0, 1, 2, 3, 4, 5, 6, (char)255};
  display.print(barChars[height]);
}

void calibrateSensors()
{
  display.clear();

  // Wait 1 second and then begin automatic sensor calibration
  // by rotating in place to sweep the sensors over the line
  delay(1000);
  for(uint16_t i = 0; i < 120; i++)
  {
    if (i > 30 && i <= 90)
    {
      motors.setSpeeds(-200, 200);
    }
    else
    {
      motors.setSpeeds(200, -200);
    }

    lineSensors.calibrate();
  }
  motors.setSpeeds(0, 0);
}

// Shows a bar graph of sensor readings on the display.
// Returns after the user presses A.
void showReadings()
{
  display.clear();

  while(!buttonB.getSingleDebouncedPress())
  {
    lineSensors.readCalibrated(lineSensorValues);

    display.gotoXY(0, 0);
    for (uint8_t i = 0; i < NUM_SENSORS; i++)
    {
      uint8_t barHeight = map(lineSensorValues[i], 0, 1000, 0, 8);
      printBar(barHeight);
    }
  }
}

void setup()
{
  // Uncomment if necessary to correct motor directions:
  //motors.flipLeftMotor(true);
  //motors.flipRightMotor(true);

  lineSensors.initFiveSensors();

  loadCustomCharacters();

  // Play a little welcome song
  buzzer.play(">g32>>c32");

  // Wait for button A to be pressed and released.
  display.clear();
  display.print(F("Press A"));
  display.gotoXY(0, 1);
  display.print(F("to calib"));
  buttonB.waitForButton();

  calibrateSensors();

  showReadings();

  // Play music and wait for it to finish before we start driving.
  display.clear();
  display.print(F("Go!"));
  buzzer.play("L16 cdegreg4");
  while(buzzer.isPlaying());
}

void loop()
{
  // Get the position of the line.  Note that we *must* provide
  // the "lineSensorValues" argument to readLine() here, even
  // though we are not interested in the individual sensor
  // readings.
  int16_t position = lineSensors.readLine(lineSensorValues);

  // Our "error" is how far we are away from the center of the
  // line, which corresponds to position 2000.
  int16_t error = position - 2000;

  // Get motor speed difference using proportional and derivative
  // PID terms (the integral term is generally not very useful
  // for line following).  Here we are using a proportional
  // constant of 1/4 and a derivative constant of 6, which should
  // work decently for many Zumo motor choices.  You probably
  // want to use trial and error to tune these constants for your
  // particular Zumo and line course.
  int16_t speedDifference = error / 4 + 6 * (error - lastError);

  lastError = error;

  // Get individual motor speeds.  The sign of speedDifference
  // determines if the robot turns left or right.
  int16_t leftSpeed = (int16_t)maxSpeed + speedDifference;
  int16_t rightSpeed = (int16_t)maxSpeed - speedDifference;

  // Constrain our motor speeds to be between 0 and maxSpeed.
  // One motor will always be turning at maxSpeed, and the other
  // will be at maxSpeed-|speedDifference| if that is positive,
  // else it will be stationary.  For some applications, you
  // might want to allow the motor speed to go negative so that
  // it can spin in reverse.
  leftSpeed = constrain(leftSpeed, 0, (int16_t)maxSpeed);
  rightSpeed = constrain(rightSpeed, 0, (int16_t)maxSpeed);

  motors.setSpeeds(leftSpeed, rightSpeed);
}*/

/** This example uses the Zumo's line sensors to detect the white
border around a sumo ring.  When the border is detected, it
backs up and turns. */


#include <Wire.h>
#include <Zumo32U4.h>

// This might need to be tuned for different lighting conditions,
// surfaces, etc.
#define QTR_THRESHOLD_TRACK_LEFT    210  // microseconds
#define QTR_THRESHOLD_TRACK_RIGHT   300  // microseconds

#define QTR_THRESHOLD_LEFT    300  // microseconds
#define QTR_THRESHOLD_MIDDLE  130  // microseconds
#define QTR_THRESHOLD_RIGHT   400  // microseconds

// These might need to be tuned for different motor types.
#define REVERSE_SPEED     200  // 0 is stopped, 400 is full speed
#define TURN_SPEED        200
#define FORWARD_SPEED     200
#define REVERSE_DURATION  200  // ms
#define TURN_DURATION     300  // ms

// Motor speed when turning.  400 is the max speed.
const uint16_t turnSpeed = 200;
// For angles measured by the gyro, our convention is that a
// value of (1 << 29) represents 45 degrees.  This means that a
// uint32_t can represent any angle between 0 and 360.
//const int32_t gyroAngle45 = 0x20000000;

// Motor speed when turning during line sensor calibration.
const uint16_t calibrationSpeed = 200;

// Change next line to this if you are using the older Zumo 32U4
// with a black and green LCD display:
// Zumo32U4LCD display;
Zumo32U4OLED display;

Zumo32U4ButtonB buttonB;
Zumo32U4Buzzer buzzer;
Zumo32U4Motors motors;
Zumo32U4LineSensors lineSensors;
Zumo32U4IMU imu;
char action;
boolean crashed = false;
int speed = 100;
boolean manualTakeOver = false;
boolean left_track = false;
boolean false_track = false;


#define NUM_SENSORS 3
unsigned int lineSensorValues[NUM_SENSORS];

#include "Turnsensor.h"

void waitForButtonAndCountDown()
{
  ledYellow(1);
  display.clear();
  display.print(F("Press B"));

  buttonB.waitForButton();

  ledYellow(0);
  display.clear();

  // Play audible countdown.
  for (int i = 0; i < 3; i++){
    delay(1000);
    buzzer.playNote(NOTE_G(3), 200, 15);
  }

  delay(1000);
  buzzer.playNote(NOTE_G(4), 500, 15);
  delay(1000);
}

/*void calibrateSensors()
{
  display.clear();

  // Wait 1 second and then begin automatic sensor calibration
  // by rotating in place to sweep the sensors over the line
  delay(1000);
  for(uint16_t i = 0; i < 120; i++){
    if (i > 30 && i <= 90){
      motors.setSpeeds(-200, 200);
    }else{
      motors.setSpeeds(200, -200);
    }
    lineSensors.calibrate();
  }
  motors.setSpeeds(0, 0);
}*/

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
    lineSensors.calibrate();
    turnSensorUpdate();
  }

  // Turn to the right 90 degrees.
  motors.setSpeeds(calibrationSpeed, -calibrationSpeed);
  while((int32_t)turnAngle > -turnAngle45 * 2)
  {
    lineSensors.calibrate();
    turnSensorUpdate();
  }

  // Turn back to center using the gyro.
  motors.setSpeeds(-calibrationSpeed, calibrationSpeed);
  while((int32_t)turnAngle < 0)
  {
    lineSensors.calibrate();
    turnSensorUpdate();
  }

  // Stop the motors.
  motors.setSpeeds(0, 0);
}

void move(char direction){
  switch (direction){
      case 'a':
        ledYellow(1);
        motors.setLeftSpeed(speed);
        motors.setRightSpeed(speed*-1);

        break;
      case 's':
        ledYellow(1);
        motors.setLeftSpeed(speed*-1);
        motors.setRightSpeed(speed*-1);
  
        break;
      case 'd':
        ledRed(1);
        motors.setRightSpeed(speed);
        motors.setLeftSpeed(speed*-1);

        break;
      case 'w':
        ledRed(1);
        motors.setLeftSpeed(speed);
        motors.setRightSpeed(speed);

        break;
      default:
        motors.setLeftSpeed(0);
        motors.setRightSpeed(0);
        ledYellow(0);
        ledRed(0);
    }
}



void turn(char dir, int angle)
{

  turnSensorReset();

  uint8_t sensorIndex;

  switch(dir)
  {
  case 'B':
    // Turn left 125 degrees using the gyro.
    motors.setSpeeds(-turnSpeed, turnSpeed);
    while((int32_t)turnAngle < turnAngle45 * 3)
    {
      turnSensorUpdate();
    }
    sensorIndex = 1;
    break;

  case 'L':
    // Turn left 45 degrees using the gyro.
    //Serial1.println((int32_t)turnAngle);
    Serial1.println("Turn Left");
    motors.setSpeeds(-turnSpeed, turnSpeed);
    while((int32_t)turnAngle < turnAngle1 * angle)//((int32_t)turnAngle < angle)//
    {
      //Serial1.println(turnAngle);
      turnSensorUpdate();
    }
    sensorIndex = 1;
    break;

  case 'R':
    Serial1.println("Turn Right");

    motors.setSpeeds(turnSpeed, -turnSpeed);
    
    while((int32_t)turnAngle > -turnAngle1 * angle)
    {
      turnSensorUpdate();
    }
    sensorIndex = 3;
    break;
  case 'F':
    // Turn right 45 degrees using the gyro.
    motors.setSpeeds(turnSpeed, -turnSpeed);
    while((int32_t)turnAngle > -turnAngle45)
    {
      turnSensorUpdate();
    }
    sensorIndex = 1;
    break;

  default:
    // This should not happen.
    return;
  }
  motors.setSpeeds(0,0);

  // Turn the rest of the way using the line sensors.
  //while(1)
  //{
   /* readSensors();
    if (aboveLine(sensorIndex))
    {
      // We found the line again, so the turn is done.
      break;
    }
    */
  //}
}

void setup()
{
  Serial1.begin(9600);
  // Uncomment if necessary to correct motor directions:
  //motors.flipLeftMotor(true);
  //motors.flipRightMotor(true);

  lineSensors.initThreeSensors();

  //buttonB.waitForButton();
  //lineSensorSetup();
  //calibrateSensors();
    // Calibrate the gyro and show readings from it until the user
  // presses button A.
  buttonB.waitForButton();
  turnSensorSetup();
  
  lineSensorSetup();
  buttonB.waitForButton();
  //waitForButtonAndCountDown();
}

void loop()
{
  


  if(!manualTakeOver){
    lineSensors.read(lineSensorValues);
    if(!crashed){
      Serial1.println(" ");
      Serial1.print(String(lineSensorValues[0]) + " ");
      Serial1.print(String(lineSensorValues[1]) + " " ); 
      Serial1.print(String(lineSensorValues[2]) + " " );
      
    }

    //delay(1000);

    if (lineSensorValues[1] > QTR_THRESHOLD_MIDDLE) {// CHECK CENTER 
      
      ledGreen(1);
      ledRed(1);
      ledYellow(1);
      if(!crashed){
      Serial1.println("Crashed Middle");}
      crashed = true;
      motors.setSpeeds(0, 0);
      turn('F', 1);
    } else if (lineSensorValues[0] > QTR_THRESHOLD_TRACK_LEFT) {// CHECK LEFT
      ledGreen(1);

      // if too far over line turn off line
      if (lineSensorValues[0] > QTR_THRESHOLD_LEFT) {// CHECK LEFT
        ledGreen(0);
        ledRed(0);
        ledYellow(1);
        
        
        if(!crashed){Serial1.println("Crashed Left");}
        crashed = true;
        turn('R', 1);
      }else{
        // follow line 

        crashed = false;
        left_track = true;
        motors.setSpeeds(FORWARD_SPEED, FORWARD_SPEED);
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
      Serial1.println("Crashed Right");}
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
      motors.setSpeeds(FORWARD_SPEED, FORWARD_SPEED);
      }
    }
    //todo add else crashed back in
  //}else{
    //TODO PUT CONTROL MOVEMENT INSIDE IF STOPPING IT FROM BEING CONTROLLED WHEN AUTOMATED 
  //}
  }else{
    if (Serial1.available() > 0){
      action = Serial1.read();
      Serial1.print("I received: ");
      Serial1.println(action);


      switch (action){
        case 'a':
          ledYellow(1);
          motors.setLeftSpeed(speed);
          motors.setRightSpeed(speed*-1);

          break;
        case 's':
          ledYellow(1);
          motors.setLeftSpeed(REVERSE_SPEED*-1);
          motors.setRightSpeed(REVERSE_SPEED*-1);
          delay(200);
          break;
        case 'd':
          ledRed(1);
          motors.setRightSpeed(speed);
          motors.setLeftSpeed(speed*-1);

          break;
        case 'w':
          ledRed(1);
          motors.setLeftSpeed(speed);
          motors.setRightSpeed(speed);
          break;
        default:
          motors.setLeftSpeed(0);
          motors.setRightSpeed(0);
          ledYellow(0);
          ledRed(0);
      }
    
    }
  }

}


