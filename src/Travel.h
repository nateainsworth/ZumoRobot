#pragma once

#include <Wire.h>
#include <Zumo32U4.h>

// These might need to be tuned for different motor types.
uint16_t REVERSE_SPEED   =  50;  // 0 is stopped, 400 is full speed
uint16_t TURN_SPEED      =  50;
uint16_t FORWARD_SPEED   =  80;
//uint16_t maneuver_speed = 100;
//uint16_t REVERSE_DURATION =  200;  // ms
//uint16_t TURN_DURATION   = 300; // ms

// Motor speed when turning.  400 is the max speed.
//const uint16_t TURN_SPEED  = 100;

void drive(int leftMotor,int rightMotor){
  if(motor_on){
    motors.setSpeeds(leftMotor, rightMotor);
  }
}
/*
int crash_alert = 0;
void thresholdCheck(bool ignoreLeft, bool ignoreRight){//bool ignoreLeft, bool ignoreRight
 lineSensors.read(lineSensorValues);
  if ((lineSensorValues[0] > 1700 && !ignoreLeft) || lineSensorValues[1] > 960 || (lineSensorValues[2] > 1700 && ! ignoreRight)) {//lineSensorValues[0] > 960 ||  || lineSensorValues[2] > 900
    crash_alert++;

    //drive(0, 0);
  }else{
    crash_alert = 0;
  }

  if(crash_alert == 3){
    drive(0,0);
    printConsoleVariable("CRA");
    printLineSensors(lineSensorValues[0], lineSensorValues[1], lineSensorValues[2]);
    maneuver_crash = true;
    
  }

}*/

int movedelay = 70;

void manualMove(char command, bool partAutomated){
    switch (command){
        case 'D':
          ledYellow(1);
          motors.setLeftSpeed(FORWARD_SPEED);
          motors.setRightSpeed(FORWARD_SPEED*-1);

          if(!partAutomated){delay(movedelay);}
          break;
        case 'S':
          ledYellow(1);
          motors.setLeftSpeed(FORWARD_SPEED*-1);
          motors.setRightSpeed(FORWARD_SPEED*-1);

          if(!partAutomated){delay(movedelay);}
          break;
        case 'A':
          ledRed(1);
          motors.setRightSpeed(FORWARD_SPEED);
          motors.setLeftSpeed(FORWARD_SPEED*-1);

          if(!partAutomated){delay(movedelay);}
          break;
        case 'W':
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
  case 'B':
    // Turn left 125 degrees using the gyro.
    drive(-TURN_SPEED , TURN_SPEED );
    if(motor_on){
      while((int32_t)turnAngle < turnAngle45 * 3)
      {
        turnSensorUpdate();
      }
    }
    sensorIndex = 1;
    break;
  // left edge
  case 'I':
    // Turn left 45 degrees using the gyro.
    //Serial1.println((int32_t)turnAngle);
    //Serial1.println("<E:Turn Left>");
    drive(-TURN_SPEED , TURN_SPEED );
    if(motor_on){
      while((int32_t)turnAngle < turnAngle1 * angle)//((int32_t)turnAngle < angle)//
      {
        //printGyro((((int32_t)turnAngle >> 16) * 360) >> 16);
        //Serial1.println(turnAngle);
        turnSensorUpdate();
      }
    }
    sensorIndex = 1;
    break;
  // right edge
  case 'O':
    //Serial1.println("<E:Turn Right>");

    drive(TURN_SPEED , -TURN_SPEED );
    if(motor_on){
      while((int32_t)turnAngle > -turnAngle1 * angle)
      {
        //printGyro((((int32_t)turnAngle >> 16) * 360) >> 16);
        turnSensorUpdate();
      }
    }
    sensorIndex = 3;
    break;
    case 'l':
    // Turn left 45 degrees using the gyro.
    //Serial1.println((int32_t)turnAngle);
    //Serial1.println("<E:Turn Left>");
    drive(0, TURN_SPEED );
    if(motor_on){
      while((int32_t)turnAngle < turnAngle1 * angle)//((int32_t)turnAngle < angle)//
      {
        //printGyro((((int32_t)turnAngle >> 16) * 360) >> 16);
        //Serial1.println(turnAngle);
        turnSensorUpdate();
      }
    }
    sensorIndex = 1;
    break;
  // right edge
  case 'r':
    //Serial1.println("<E:Turn Right>");

    drive(TURN_SPEED ,0);
    if(motor_on){
      while((int32_t)turnAngle > -turnAngle1 * angle)
      {
        //printGyro((((int32_t)turnAngle >> 16) * 360) >> 16);
        turnSensorUpdate();
      }
    }
    sensorIndex = 3;
    break;
  case 'R':
    // Turn right 45 degrees using the gyro.
    //int16_t oldCountsRight = encoders.getCountsAndResetRight();
    //int16_t oldCountsLeft = encoders.getCountsAndResetLeft();
    //int16_t countsRight = encoders.getCountsRight();
    //printMovementUpdate("s", countsRight,"r", (int)(45 * angle));

    drive(TURN_SPEED , -TURN_SPEED );
    if(motor_on){
      while((int32_t)turnAngle > -turnAngle45 * angle )
      {
       /* if( thresholds){
          thresholdCheck(true, false);
        }*/
        //printGyro((((int32_t)turnAngle >> 16) * 360) >> 16);
        turnSensorUpdate();
      }
    }
    
    sensorIndex = 1;
    break;
  case 'L':
    //int16_t oldCountsRightr = encoders.getCountsAndResetRight();
    //int16_t oldCountsLeftr = encoders.getCountsAndResetLeft();


    // Turn left 45 degrees using the gyro.
    drive(-TURN_SPEED , TURN_SPEED );
    if(motor_on){
      while((int32_t)turnAngle < turnAngle45 * angle)
      {
       /* if( thresholds){
          thresholdCheck(true, false);
        }*/
        //printGyro((((int32_t)turnAngle >> 16) * 360) >> 16);
        turnSensorUpdate();
      }
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

void reverse(float distance){

  int cpr = (float)750 * (float)distance;
  drive(-REVERSE_SPEED , -REVERSE_SPEED);

  //int16_t countsLeft = encoders.getCountsLeft();
  int16_t countsRight = encoders.getCountsRight();

  //bool errorLeft = encoders.checkErrorLeft();
  bool errorRight = encoders.checkErrorRight();
  int16_t startingCount = countsRight;
  //printConsoleVariable("Reversing");
  if(motor_on){
    while((int32_t)countsRight > (startingCount - cpr) && errorRight!=true)//&& errorLeft != true
    {
      //thresholdCheck();
      //countsLeft = encoders.getCountsLeft();
      countsRight = encoders.getCountsRight();
      
      //errorLeft = encoders.checkErrorLeft();
      errorRight = encoders.checkErrorRight();
    }
    
  }
  drive(0,0);
  //printConsoleVariable("Finished Reversing");
}

/*
void forward(float distance){

  int cpr = (float)750 * (float)distance;
  drive(maneuver_speed , maneuver_speed);

  //int16_t countsLeft = encoders.getCountsLeft();
  int16_t countsRight = encoders.getCountsRight();

  //bool errorLeft = encoders.checkErrorLeft();
  bool errorRight = encoders.checkErrorRight();
  int16_t startingCount = countsRight;
  //printConsoleVariable("Forward:" + startingCount);
  if(motor_on){
    while((int32_t)countsRight < (startingCount + cpr) && errorRight!=true )//&& errorLeft != true 
    {
      //thresholdCheck(true,true);
      //countsLeft = encoders.getCountsLeft();
      countsRight = encoders.getCountsRight();
      
      //errorLeft = encoders.checkErrorLeft();
      errorRight = encoders.checkErrorRight();
    }
    
  }
  drive(0,0);
  //printConsoleVariable("Finished: " + String(countsRight) + " : " + String(cpr * distance));
}*/

