#pragma once

#include <Wire.h>
#include <Zumo32U4.h>

int startingDistance = 0;

unsigned long lastPersonCheckMillis;
unsigned long currentTime;

int proximityPeriod = 300;

bool modeTwoTakeOver = false;

void proximityPersonCheck(bool leftSide){

    if(proxSensors.countsLeftWithLeftLeds() >= 4 ){
        buzzer.playFromProgramSpace(PSTR("!<g4"));
        printConsoleVariable("LPD");
    }
    if(!leftSide){
        if(proxSensors.countsRightWithRightLeds() >= 4 ){
            buzzer.playFromProgramSpace(PSTR("!<a4"));
            printConsoleVariable("RPD");
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
  ModeTwoWait,
};

State state = FindLeft;
State previous_state = Starting;

void updateState(State changeTo){
    if(state != ModeTwoWait){
        previous_state = state;
    }
    state = changeTo;
/*
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
        case 13: printConsoleVariable("Waiting for console"); break;
        default: printConsoleVariable("Error State");
    }*/
}


bool moveDetection () {
    if(lineSensorValues[1] > QTR_THRESHOLD_MIDDLE){
        ledGreen(1);
        ledRed(1);
        ledYellow(1);

        if(!modeTwoTakeOver){
            
            if(state == FindLeft){
                // Turn Left
                reverse(0.2);
                updateState(RightCornering);
            }else if(state == FollowingLeft){
                //Turn Right
                updateState(RightCornering);
            }else if(state == ForwardFindLeft || state == CheckForward){
                reverse(0.2);
                updateState(LeftCornering);
            }else{
                reverse(0.2);
                updateState(RightCornering);
            }
        }else{
            reverse(0.2);
            drive(0,0);
            updateState(ModeTwoWait);
        }
        return true;
    } else if (lineSensorValues[0] > QTR_THRESHOLD_TRACK_LEFT){
        ledGreen(0);
        ledRed(0);
        ledYellow(1);
        if(state == CheckForward){
            drive(0,0);
            updateState(CorridorReverse);
            return true;
        }
        if (lineSensorValues[0] > QTR_THRESHOLD_LEFT) {
            // if over line too far correct path.
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
    if(state != ModeTwoWait){
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
            }else if(state == RightCornering){
                if(!modeTwoTakeOver){
                    turn('R', 2, false);
                    
                    if(previous_state == FollowingLeft){
                        lostLeft = 0;
                        updateState(FindLeft);
                    }else if(state == CheckForward){
                        turn('I', 1, false);
                    }else if(state == ForwardFindLeft){
                        turn('I', 1, false);
                    }else{
                        updateState(FindLeft);
                    }
                }else{
                    drive(0,0);
                    updateState(ModeTwoWait);
                }

            }else if(state == LeftCornering){
                if(!modeTwoTakeOver){ 
                    turn('L', 1.9, false);
                    
                    if(previous_state == CorridorReverse){
                        updateState(FindLeft);
                    }else{
                        updateState(ForwardFindLeft);
                        startingDistance = encoders.getCountsRight();
                    }
                }else{
                    drive(0,0);
                    updateState(ModeTwoWait);
                }
            }else if(state == CorrectLeft){
                turn('O', 1, false);
            }else if(state == CorrectRight){
                turn('I', 1, false);
            }else if(state == CorridorReverse){
                reverse(0.2);
                if(!modeTwoTakeOver){ 
                    updateState(LeftCornering);
                }else{
                    drive(0,0);
                    updateState(ModeTwoWait);
                }
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
                    proximityPersonCheck(false);
                }
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
                    if(!modeTwoTakeOver){
                        updateState(CheckForward);
                        startingDistance = encoders.getCountsRight();
                    }else{
                        drive(0,0);
                        updateState(ModeTwoWait);
                    }
                }//else leave to find left
                
            }else if(state == LeftCornering){
                turn('L', 1.9, false);
                if(previous_state == CorridorReverse){
                    updateState(FindLeft);
                    startingDistance = encoders.getCountsRight();
                }else{
                        updateState(ForwardFindLeft);
                        startingDistance = encoders.getCountsRight();
                }
            
            }else if(state == LosingLeftLine){
                turn('I', 1,false);
            }else if(state == CorrectLeft){
                updateState(FindLeft);
            }else if(state == CorrectRight){
                turn('I', 1, false);
                updateState(FindLeft);
                // refind left or continue dependant on previous states
            }else if(state == CheckForward){
                
                int cpr = (float)750 * (float) 1.5;
                drive(FORWARD_SPEED , FORWARD_SPEED);

                int16_t countsRight = encoders.getCountsRight();

                bool errorRight = encoders.checkErrorRight();
                int16_t startingCount = countsRight;

                if((int32_t)countsRight > (startingDistance + cpr)){
                    // did distance without hitting wall
                    drive(0,0);
                    proximityPersonCheck(true);
                    updateState(ForwardFindLeft);
                    startingDistance = encoders.getCountsRight();
                }
                
            }else if(state == ForwardFindLeft){

                int cpr = (float)750 * (float) 1.5;
                drive(FORWARD_SPEED , FORWARD_SPEED);

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
}

void runModeTwo(){
    modeTwoTakeOver = true;

    if(state == ModeTwoWait){
     if(incomingMessage){
      
      switch(commandType){
        case 'L':
            turn('L', 2, false);
            previous_state = LeftCornering;
            updateState(FindLeft);
        break;
        case 'R':
            turn('R', 2, false);
            previous_state = RightCornering;
            updateState(FindLeft);
        break;
        case 'F':
            startingDistance = encoders.getCountsRight();
            updateState(CheckForward);
        break;
      }

     }
    }else{

        control();
    }
}
