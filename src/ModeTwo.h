#pragma once

#include <Wire.h>
#include <Zumo32U4.h>

int startingDistance = 0;

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
      //while(!maneuver_crash){
        //reverse(0.2);
       // turn('R', 1, true);
       // forward(1.8);
       // drive(0,0);
       // break;
      }
    
      /*
    proxSensors.countsLeftWithRightLeds(),
    proxSensors.countsFrontWithLeftLeds(),
    proxSensors.countsFrontWithRightLeds(),
    proxSensors.countsRightWithLeftLeds(),
    proxSensors.countsRightWithRightLeds()*/
}



enum State
{
  Starting,
  FollowingLeft,
  Scanning,
  FindLeft,
  RightCornering,
  LeftCornering,
  LosingLeftLine,
  CorrectLeft,
  CorrectRight,
  CheckForward,
  ForwardToFind,
  ForwardFindLeft
};

State state = FindLeft;
State previous_state = Starting;

void updateState(State changeTo){
    previous_state = state;
    state = changeTo;
    printConsoleVariable("New State");

    switch(changeTo){
        case 0: printConsoleVariable("Starting"); break;
        case 1: printConsoleVariable("FollowingLeft"); break;
        case 2: printConsoleVariable("Scanning"); break;
        case 3: printConsoleVariable("FindLeft"); break;
        case 4: printConsoleVariable("RightCornering"); break;
        case 5: printConsoleVariable("LeftCornering"); break;
        case 6: printConsoleVariable("LosingLeftLine"); break;
        case 7: printConsoleVariable("CorrectLeft"); break;
        case 8: printConsoleVariable("CorrectRight"); break;
        case 9: printConsoleVariable("CheckForward"); break;
        case 10: printConsoleVariable("ForwardToFind"); break;
        case 11: printConsoleVariable("ForwardFindLeft"); break;
        default: printConsoleVariable("Error State");
    }
}


bool moveDetection () {
    if(lineSensorValues[1] > QTR_THRESHOLD_MIDDLE){
        if(state == FindLeft){
            // Turn Left
            reverse(0.2);
            updateState(RightCornering);
        }else if(state == FollowingLeft){
            //Turn Right
            updateState(RightCornering);
        }else{
            printConsoleVariable("ERR1");
        }
        return true;
    } else if (lineSensorValues[0] > QTR_THRESHOLD_TRACK_LEFT){

        
        if (lineSensorValues[0] > QTR_THRESHOLD_LEFT) {
            /// if over line too far correct path.
            updateState(CorrectLeft);
        }else{
            // not over line too far keep following.
            updateState(FollowingLeft);
        }
        return true;

    } else if (lineSensorValues[2] > QTR_THRESHOLD_RIGHT) {

        updateState(CorrectRight);
        return true;
    }else{
        return false;
    }

}

int lostLeft = 0;


void control(){
    bool detected = moveDetection();
    if(detected){
        if(state == FollowingLeft){
            drive(FORWARD_SPEED, FORWARD_SPEED);
        }else if(state == Scanning){
            proximityPersonCheck();
        }else if(state == RightCornering){
            turn('R', 1.9, true);
            // TODO setup new direction to go in

        }else if(state == LeftCornering){
            turn('L', 1.9, true);
            // TODO setup new direction to go in
        }else if(state == CorrectLeft){
            turn('O', 1, false);
        }else if(state == CorrectRight){
            turn('I', 1, false);
        }else if(state == CheckForward){
            // hit middle wall should have already turned
        }
    }else{
        if(state == FollowingLeft){
           // refind left
           updateState(FindLeft);
           lostLeft++;
        }else if(state == Scanning){
             proximityPersonCheck();
        }else if(state == FindLeft){
            turn('l', 1,false);
            //Still didn't find left
            lostLeft++;
            if(lostLeft  >= 2 ){
                // check forward until distance reached
                updateState(CheckForward);
                startingDistance = encoders.getCountsRight();
            }//else leave to find left
            
        }else if(state == RightCornering){
        
        }else if(state == LeftCornering){
        
        }else if(state == LosingLeftLine){
            turn('I', 1,false);
        }else if(state == CorrectLeft){
            
        }else if(state == CorrectRight){
            // refind left or continue dependant on previous states
        }else if(state == CheckForward){
            int cpr = (float)750 * (float) 1;
            drive(maneuver_speed , maneuver_speed);

            int16_t countsRight = encoders.getCountsRight();

            bool errorRight = encoders.checkErrorRight();
            int16_t startingCount = countsRight;

            if((int32_t)countsRight < (startingDistance + cpr)){
                // did distance without hitting wall
                proximityPersonCheck();
                updateState(ForwardFindLeft);
            }
        }else if(state == ForwardFindLeft){
            int cpr = (float)750 * (float) 1;
            drive(maneuver_speed , maneuver_speed);

            int16_t countsRight = encoders.getCountsRight();

            bool errorRight = encoders.checkErrorRight();
            int16_t startingCount = countsRight;

            if((int32_t)countsRight < (startingDistance + cpr)){
                // did distance still no hit
                updateState(FindLeft);
            }
        }
    }
}

void runModeTwo(){
    //manualMove(commandType, true);
    control();
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
