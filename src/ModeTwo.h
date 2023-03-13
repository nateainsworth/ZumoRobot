#pragma once

#include <Wire.h>
#include <Zumo32U4.h>

int startingDistance = 0;

unsigned long lastPersonCheckMillis;
unsigned long currentTime;

unsigned long lastDetectionMillis;
unsigned long leftDetectMillis;
unsigned long rightDetectMillis;

int proximityPeriod = 300;

void proximityPersonCheck(){
    delay(500);
    if(proxSensors.countsFrontWithLeftLeds() >= 4 ){
        leftDetectMillis = millis();
        if(leftDetectMillis - lastDetectionMillis >= 150){
            buzzer.playFromProgramSpace(PSTR("!<g4"));
            lastDetectionMillis = leftDetectMillis;
            printConsoleVariable("PD");
        }

    }
    if(proxSensors.countsFrontWithRightLeds() >= 4 ){
        rightDetectMillis = millis();
        // save previous detection 
        if(rightDetectMillis - lastDetectionMillis >= 150){
            buzzer.playFromProgramSpace(PSTR("!<a4"));
            lastDetectionMillis = rightDetectMillis;
            printConsoleVariable("PD");
        }
        
      
    }

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
  ForwardFindLeft,
  CorridorReverse,
};

State state = FindLeft;
State previous_state = Starting;

void updateState(State changeTo){
    previous_state = state;
    state = changeTo;
    //printConsoleVariable("New State");

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
        case 12: printConsoleVariable("CorridorReverse"); break;
        default: printConsoleVariable("Error State");
    }
}


bool moveDetection () {
    if(lineSensorValues[1] > QTR_THRESHOLD_MIDDLE){
        ledGreen(1);
        ledRed(1);
        ledYellow(1);
        if(state == FindLeft){
            // Turn Left
            reverse(0.2);
            updateState(RightCornering);
        }else if(state == FollowingLeft){
            //Turn Right
            updateState(RightCornering);
        }else if(state == ForwardFindLeft){
            reverse(0.2);
            updateState(LeftCornering);
        }else{
            printConsoleVariable("ERR1");
            reverse(0.2);
            updateState(RightCornering);
            
        }
        return true;
    } else if (lineSensorValues[0] > QTR_THRESHOLD_TRACK_LEFT){
        ledGreen(0);
        ledRed(0);
        ledYellow(1);
        if(state == CheckForward){
            buzzer.playNote(A5,10,5);
            
            drive(0,0);
            delay(5000);
            updateState(CorridorReverse);
            return true;
            
        }
        if (lineSensorValues[0] > QTR_THRESHOLD_LEFT) {
            /// if over line too far correct path.
            updateState(CorrectLeft);
        }else{
            // not over line too far keep following.
            updateState(FollowingLeft);
        }
        return true;

    } else if (lineSensorValues[2] > QTR_THRESHOLD_RIGHT) {
        ledGreen(0);
        ledRed(1);
        ledYellow(0);
        updateState(CorrectRight);
        return true;
    }else{
        ledGreen(0);
        ledRed(0);
        ledYellow(0);
        return false;
    }

}

int lostLeft = 0;


void control(){
    bool detected = moveDetection();
    if(detected){
        if(state == FollowingLeft){
            drive(FORWARD_SPEED, FORWARD_SPEED);
            lostLeft = 0;
            currentTime = millis();

            if(currentTime - lastPersonCheckMillis >= proximityPeriod)
            {
                lastPersonCheckMillis = currentTime;
            }
        }else if(state == Scanning){
            drive(0,0);
            proximityPersonCheck();
        }else if(state == RightCornering){
            turn('R', 2, false);
            // TODO setup new direction to go in
            if(previous_state == FollowingLeft){
                lostLeft = 0;
                updateState(FindLeft);
            }else if(state == CheckForward){
                turn('I', 1, false);
            }else if(state == ForwardFindLeft){
                turn('I', 1, false);
            }else{
                printConsoleVariable("ERR2");
                updateState(FindLeft);
            }

        }else if(state == LeftCornering){
            turn('L', 1.9, false);
            // TODO setup new direction to go in
            if(previous_state){
                updateState(ForwardFindLeft);
                startingDistance = encoders.getCountsRight();
            }
        }else if(state == CorrectLeft){
            turn('O', 1, false);
        }else if(state == CorrectRight){
            turn('I', 1, false);
        }else if(state == CheckForward){
            // hit middle wall should have already turned
        }else if(state == CorridorReverse){
            reverse(0.2);
            updateState(LeftCornering);
        }
    //No detection of a line 
    }else{
        if(state == FollowingLeft){
            // refind left
            updateState(FindLeft);
            currentTime = millis();

            if(currentTime - lastPersonCheckMillis >= proximityPeriod)
            {
                lastPersonCheckMillis = currentTime;
                proximityPersonCheck();
            }
           //TODO consider removing scanning
        }else if(state == Scanning){
            drive(0,0);
            proximityPersonCheck();
        }else if(state == FindLeft){
            
            turn('l', 1,false);
            //Still didn't find left
            if(previous_state == FollowingLeft){
            lostLeft++;
            }
            if(lostLeft  >= 4 ){
                turn('O', lostLeft, false);
                lostLeft = 0;
                // check forward until distance reached
                updateState(CheckForward);
                startingDistance = encoders.getCountsRight();
            }//else leave to find left
            
        }else if(state == RightCornering){
        
        }else if(state == LeftCornering){
            turn('L', 1.9, false);
            //if(previous_state){
                updateState(ForwardFindLeft);
                startingDistance = encoders.getCountsRight();
            //}
        
        }else if(state == LosingLeftLine){
            turn('I', 1,false);
        }else if(state == CorrectLeft){
            
        }else if(state == CorrectRight){
            turn('I', 1, false);
            updateState(FindLeft);
            // refind left or continue dependant on previous states
        }else if(state == CheckForward){
            int cpr = (float)750 * (float) 1.5;
            drive(maneuver_speed , maneuver_speed);

            int16_t countsRight = encoders.getCountsRight();

            bool errorRight = encoders.checkErrorRight();
            int16_t startingCount = countsRight;

            if((int32_t)countsRight > (startingDistance + cpr)){
                // did distance without hitting wall
                drive(0,0);
                proximityPersonCheck();
                updateState(ForwardFindLeft);
                startingDistance = encoders.getCountsRight();
            }
        }else if(state == ForwardFindLeft){

            int cpr = (float)750 * (float) 1.5;
            drive(maneuver_speed , maneuver_speed);

            int16_t countsRight = encoders.getCountsRight();

            bool errorRight = encoders.checkErrorRight();
            int16_t startingCount = countsRight;

            if((int32_t)countsRight > (startingDistance + cpr)){
                // did distance still no hit
                updateState(FindLeft);
                drive(0,0);
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
