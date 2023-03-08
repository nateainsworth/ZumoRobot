#pragma once

#include <Wire.h>
#include <Zumo32U4.h>



/*
enum State
{
  StateFollowingLeft,
  StateScanning,
  StateNoLine,
  StateDeadEnd,
};

ChangeState(newState){
  state = (State)newState;
  justChangedState = true;
  stateStartTime = millis();
  ledRed(0);
  ledYellow(0);
  ledGreen(0);
}*/

bool detectedPerson = false;

void proximityPersonCheck(){
  delay(500);
    if(proxSensors.countsFrontWithLeftLeds() >= 4 ){
      detectedPerson = true;
    }
    if(proxSensors.countsFrontWithRightLeds() >= 4 ){
      
      detectedPerson = true;
    }

    if(detectedPerson){
      printConsoleVariable("PD");
      while(!maneuver_crash){
        //reverse(0.2);
        turn('R', 1, true);
        forward(1.8);
        drive(0,0);
        break;
      }
    }
      /*
    proxSensors.countsLeftWithRightLeds(),
    proxSensors.countsFrontWithLeftLeds(),
    proxSensors.countsFrontWithRightLeds(),
    proxSensors.countsRightWithLeftLeds(),
    proxSensors.countsRightWithRightLeds()*/
}

int lostTrack = 0;
bool roomOrCorridor = false;
int startingDistance = 0;

    

void runModeThree(){


if (lineSensorValues[1] > QTR_THRESHOLD_MIDDLE) {// CHECK CENTER 
     // while(!maneuver_crash){
        ledGreen(1);
        ledRed(1);
        ledYellow(1);
        
        printConsoleVariable("CrM");
        left_track = false;
        crashed = true;
        drive(0, 0);
        int16_t countsRight = encoders.getCountsAndResetRight();
        int16_t countLeft = encoders.getCountsAndResetLeft();
        printMovementUpdate("s", countsRight,"R", (int)(45 * 2));
        if( roomOrCorridor == true){
          printConsoleVariable("corridor");
          reverse(0.2);
          // find left line again

        }else{
          delay(50);
          reverse(0.1);

          turn('R', 1.9, true);
        }
        //break;
     // }
    } else if (lineSensorValues[0] > QTR_THRESHOLD_TRACK_LEFT) {// CHECK LEFT
      ledGreen(1);

      
      // if too far over line turn off line
      if (lineSensorValues[0] > QTR_THRESHOLD_LEFT) {// CHECK LEFT
   
        ledGreen(0);
        ledRed(0);
        ledYellow(1);
        
        
        if(!crashed){
          printConsoleVariable("CrL");
        }
        crashed = true;
        turn('O', 1, false);
      }else if(lineSensorValues[0] < QTR_BOUND_TRACK_LEFT){
        turn('l', 1,false);


      }else{
        // follow line 

        crashed = false;
        left_track = true;
        drive(FORWARD_SPEED, FORWARD_SPEED);
      }

      
    } else if (lineSensorValues[2] > QTR_THRESHOLD_RIGHT) {// CHECK RIGHT

      ledGreen(0);
      ledRed(1);
      ledYellow(0);
      printConsoleVariable("CrR");

      crashed = true;
      turn('I', 1, false);


    } else {
        //while(!maneuver_crash){

          //if(left_track == true){
            // if left wall is lost turn left to try and find it
        
          //}else{
          if(lostTrack >= 2 || !roomOrCorridor){
            
            printConsoleVariable("EOT"); // end of track
            //forward(0.8);
            //turn('o', 2,false);

            int16_t countsRight = encoders.getCountsAndResetRight();
            int16_t countLeft = encoders.getCountsAndResetLeft();
            printMovementUpdate("s", countsRight,"l", (int)(45 * 2));
            delay(50);
            left_track = false;
            //lostTrack = 0;
            roomOrCorridor = true;
            startingDistance = encoders.getCountsRight();
            /*
            proximityPersonCheck();
            if(!detectedPerson){
              printConsoleVariable("GO"); //continue
              turn('L', 1.9,false);
              lostTrack = 0;
              delay(300);
              left_track = false;
              
              
              maneuver_crash = false;
              crashed = false;

              roomOrCorridor = true;
              proximityPersonCheck();

              forward(0.2);
            }*/
            printConsoleVariable("SK");
          }else{
            if(left_track){
              // refinding left by slight movement
              printConsoleVariable("LL"); // look left
              turn('I', 1,false);
              crashed = false;
              ledGreen(0);
              ledRed(0);
              ledYellow(0);
              lostTrack++;
            }else{
              if(roomOrCorridor == true){
                int cpr = (float)750 * (float) 1;
                drive(maneuver_speed , maneuver_speed);

                int16_t countsRight = encoders.getCountsRight();

                bool errorRight = encoders.checkErrorRight();
                int16_t startingCount = countsRight;

                if((int32_t)countsRight < (startingDistance + cpr)){
                  // middle of room
                  drive(0,0);
                  printConsoleVariable("rom");
                  proximityPersonCheck();

                  //TODO Deal with DETECTION

                }
              }else{
                // find left track without right motor to do a larger circle
                printConsoleVariable("lnr");
                turn('l', 1,false);
                crashed = false;
                ledGreen(0);
                ledRed(0);
                ledYellow(0);
              }

            }

           
          }
          //break;
        //}
      // Otherwise, go straight.
      //drive(FORWARD_SPEED, FORWARD_SPEED);
      //}
    }
    //todo add else crashed back in
  //}else{
    //TODO PUT CONTROL MOVEMENT INSIDE IF STOPPING IT FROM BEING CONTROLLED WHEN AUTOMATED 
  //}
    
}

void updateLowSliders(){
      int sliders [2];
      parseSliders(sliders, 2);
      QTR_THRESHOLD_TRACK_LEFT = sliders[0];
      QTR_THRESHOLD_TRACK_RIGHT = sliders[1];
}

void updateMaxSliders(){
      int sliders [3];
      parseSliders(sliders, 3);
      QTR_THRESHOLD_LEFT  = sliders[0];
      QTR_THRESHOLD_MIDDLE = sliders[1];
      QTR_THRESHOLD_RIGHT = sliders[2];
      Serial.println("Updated left: ");
      Serial.print(QTR_THRESHOLD_LEFT);
}
