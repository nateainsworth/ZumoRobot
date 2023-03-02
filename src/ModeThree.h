#pragma once

#include <Wire.h>
#include <Zumo32U4.h>



/*
enum State
{
  StatePausing,
  StateWaiting,
  StateScanning,
  StateDriving,
  StateBacking,
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
    if(proxSensors.countsFrontWithLeftLeds() >= 4 ){
      detectedPerson = true;
    }
    if(proxSensors.countsFrontWithRightLeds() >= 4 ){
      
      detectedPerson = true;
    }

    if(detectedPerson){
      printConsoleVariable("Person Detected");
      while(!maneuver_crash){
        turn('R', 1.8);
        forward(1.5);
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

void runModeThree(){

    if(maneuver_crash){
      printConsoleVariable("Crashed During Maneuver");

      delay(5000);

    }else if (lineSensorValues[1] > QTR_THRESHOLD_MIDDLE) {// CHECK CENTER 
     // while(!maneuver_crash){
        ledGreen(1);
        ledRed(1);
        ledYellow(1);

        
        printConsoleVariable("Crashed Middle");
        left_track = false;
        crashed = true;
        drive(0, 0);
        int16_t countsRight = encoders.getCountsAndResetRight();
        int16_t countLeft = encoders.getCountsAndResetLeft();
        printMovementUpdate("s", countsRight,"R", (int)(45 * 2));
        delay(50);
        reverse(0.1);

        turn('R', 1.9);
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
          printConsoleVariable("Crashed Left");
        }
        crashed = true;
        turn('O', 1);
      }else if(lineSensorValues[0] < QTR_BOUND_TRACK_LEFT){
        turn('l', 1);


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

  
       printConsoleVariable("Crashed Right");
       
      
      crashed = true;
      turn('I', 1);


    } else {
        //while(!maneuver_crash){

          //if(left_track == true){
            // if left wall is lost turn left to try and find it
        
          //}else{
          if(lostTrack >= 2){
            
            printConsoleVariable("End Of Track");
            forward(0.8);
            int16_t countsRight = encoders.getCountsAndResetRight();
            int16_t countLeft = encoders.getCountsAndResetLeft();
            printMovementUpdate("s", countsRight,"l", (int)(45 * 2));
            delay(50);
            turn('L', 1.9);
            lostTrack = 0;
            delay(100);
            left_track = false;
            forward(0.2);
            proximityPersonCheck();
          }else{
            if(left_track){
              turn('I', 1);
              crashed = false;
              ledGreen(0);
              ledRed(0);
              ledYellow(0);
              lostTrack++;
            }else{
              turn('l', 1);
              crashed = false;
              ledGreen(0);
              ledRed(0);
              ledYellow(0);

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
