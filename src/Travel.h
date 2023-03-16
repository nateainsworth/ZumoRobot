#pragma once

#include <Wire.h>
#include <Zumo32U4.h>

uint16_t REVERSE_SPEED   =  50;
uint16_t TURN_SPEED      =  50;
uint16_t FORWARD_SPEED   =  80;

int movedelay = 70; // used for button press movement in mode one

void drive(int leftMotor,int rightMotor){
  if(motor_on){
    motors.setSpeeds(leftMotor, rightMotor);
  }
}



void manualMove(char command, bool partAutomated){
    switch (command){
        case 'D': // turns left
          ledYellow(1);
          motors.setLeftSpeed(FORWARD_SPEED);
          motors.setRightSpeed(FORWARD_SPEED*-1);

          if(!partAutomated){delay(movedelay);}
          break;
        case 'S': //revers
          ledYellow(1);
          motors.setLeftSpeed(FORWARD_SPEED*-1);
          motors.setRightSpeed(FORWARD_SPEED*-1);

          if(!partAutomated){delay(movedelay);}
          break;
        case 'A': //turns right
          ledRed(1);
          motors.setRightSpeed(FORWARD_SPEED);
          motors.setLeftSpeed(FORWARD_SPEED*-1);

          if(!partAutomated){delay(movedelay);}
          break;
        case 'W': //forward
          ledRed(1);
          motors.setLeftSpeed(FORWARD_SPEED);
          motors.setRightSpeed(FORWARD_SPEED);

          if(!partAutomated){delay(movedelay);}
          break;
        default:
          motors.setLeftSpeed(0);
          motors.setRightSpeed(0);
          ledYellow(0);
          ledRed(0);
    }

    if(!partAutomated){
      motors.setLeftSpeed(0);
      motors.setRightSpeed(0);
      ledYellow(0);
      ledRed(0);
    }
}



void turn(char dir, float angle,bool thresholds)
{

  turnSensorReset();
  uint8_t sensorIndex;
  switch(dir)
  {
  // left edge
  case 'I':
    // Turn left on the spot by specified angle
    drive(-TURN_SPEED , TURN_SPEED );
    if(motor_on){
      while((int32_t)turnAngle < turnAngle1 * angle)
      {
        turnSensorUpdate();
      }
    }
    sensorIndex = 1;
    break;
  // turn right on the spot by specified angle
  case 'O':
    drive(TURN_SPEED , -TURN_SPEED );
    if(motor_on){
      while((int32_t)turnAngle > -turnAngle1 * angle)
      {
        turnSensorUpdate();
      }
    }
    sensorIndex = 3;
    break;
    case 'l':
    // Turn left by specified angle
    drive(0, TURN_SPEED );
    if(motor_on){
      while((int32_t)turnAngle < turnAngle1 * angle)
      {
        turnSensorUpdate();
      }
    }
    sensorIndex = 1;
    break;
  case 'r':
    // turns right by specified angle
    drive(TURN_SPEED ,0);
    if(motor_on){
      while((int32_t)turnAngle > -turnAngle1 * angle)
      {
        turnSensorUpdate();
      }
    }
    sensorIndex = 3;
    break;
  case 'R':
    // Turns right 45 degrees
    drive(TURN_SPEED , -TURN_SPEED );
    if(motor_on){
      while((int32_t)turnAngle > -turnAngle45 * angle )
      {
        turnSensorUpdate();
      }
      //int previousDistance = encoders.getCountsAndResetRight();
      //printMovementUpdate("s",previousDistance,"R",90);
    }
    
    sensorIndex = 1;
    break;
  case 'L':
    // Turns left 45 degrees 
    drive(-TURN_SPEED , TURN_SPEED );
    if(motor_on){
      while((int32_t)turnAngle < turnAngle45 * angle)
      {
        turnSensorUpdate();
      }
      //int previousDistance = encoders.getCountsAndResetRight();
      //printMovementUpdate("s",previousDistance,"L",90);
    }
    sensorIndex = 1;
    break;

  default:
    return;
  }
  drive(0,0);

}

void reverse(float distance){

  int cpr = (float)750 * (float)distance;
  drive(-REVERSE_SPEED , -REVERSE_SPEED);

  int16_t countsRight = encoders.getCountsRight();
  bool errorRight = encoders.checkErrorRight();
  int16_t startingCount = countsRight;
  if(motor_on){
    while((int32_t)countsRight > (startingCount - cpr) && errorRight!=true)//&& errorLeft != true
    {
      countsRight = encoders.getCountsRight();
      errorRight = encoders.checkErrorRight();
    }
    
  }
  drive(0,0);
}
