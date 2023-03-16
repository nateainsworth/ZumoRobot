#pragma once

#include <Wire.h>
#include <Zumo32U4.h>

// records loss of left line
int lostLeft = 0;

// encoder record
int startingDistance = 0;

// proximity checking variables
unsigned long lastPersonCheckMillis;
unsigned long currentTime;
int proximityPeriod = 3000;

// state machine setup
enum State
{
  Starting,
  FollowingLeft,
  FindLeft,
  RightCornering,
  LeftCornering,
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


// checks proximity and lets out a buzz and sends a message upon detection.
void proximityPersonCheck(bool leftSide){

    if(leftSide){
        if(proxSensors.countsLeftWithLeftLeds() >= 4 ){
            buzzer.playFromProgramSpace(PSTR("!<g4"));
            printConsoleVariable("LPD");
        }
    }else {
        if(proxSensors.countsRightWithRightLeds() >= 4 ){
            buzzer.playFromProgramSpace(PSTR("!<a4"));
            printConsoleVariable("RPD");
        }
    }

}


void updateState(State changeTo){
    if(state != ModeTwoWait){
        previous_state = state;
    }
    state = changeTo;

    //used for debugging state changes

    switch(changeTo){
        case 0: printConsoleVariable("Starting"); break;
        case 1: printConsoleVariable("FollowingLeft"); break;
        case 2: printConsoleVariable("FindLeft"); break;
        case 3: printConsoleVariable("RightCornering"); break;
        case 4: printConsoleVariable("LeftCornering"); break;
        case 5: printConsoleVariable("CorrectLeft"); break;
        case 6: printConsoleVariable("CorrectRight"); break;
        case 7: printConsoleVariable("CheckForward"); break;
        case 8: printConsoleVariable("ForwardToFind"); break;
        case 9: printConsoleVariable("ForwardFindLeft"); break;
        case 10: printConsoleVariable("CorridorReverse"); break;
        case 11: printConsoleVariable("Waiting for console"); break;
        default: printConsoleVariable("Error State");
    }
}

// checks for line detection and updates the state of movement
bool moveDetection () {
    if(lineSensorValues[1] > QTR_THRESHOLD_MIDDLE){ // checks front sensor against the middle threshold
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
    } else if (lineSensorValues[0] > QTR_THRESHOLD_TRACK_LEFT){ // checks left sensor against the min left threshold
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

    } else if (lineSensorValues[2] > QTR_THRESHOLD_RIGHT) { // checks right sensor against the right threshold
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


// checks for detection of lines before then handling the current state of the robot dependant on if it is on a line or not on a line.
void semiControl(){

    //if(state != ModeTwoWait){
        bool detected = moveDetection();
        if(detected){
            if(state == FollowingLeft){ // line detected and still following line
                drive(FORWARD_SPEED, FORWARD_SPEED);
                lostLeft = 0;
                currentTime = millis();

                if(currentTime - lastPersonCheckMillis >= proximityPeriod) // checks for detection of people in rooms on the right on a countdown
                {
                    lastPersonCheckMillis = currentTime;
                    proximityPersonCheck(false);
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
                        startingDistance = encoders.getCountsRight();
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
                if(state == ForwardFindLeft){
                    updateState(previous_state);
                }else if(state == CheckForward){
                    updateState(previous_state);
                }
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
            // no longer detected left but is still in a following left state
            if(state == FollowingLeft){
                updateState(FindLeft);
                currentTime = millis();

                if(currentTime - lastPersonCheckMillis >= proximityPeriod)
                {
                    lastPersonCheckMillis = currentTime;
                    proximityPersonCheck(false);
                }
            // still hasn't detected left.
            }else if(state == FindLeft){
                
                //TODO detect how far the gyro has moved if going around in circles try something else.
                turn('l', 1,false);
                //Still didn't find left but was following left so update lost left
                if(previous_state == FollowingLeft){
                lostLeft++;
                }
                // if lost left more than 4 times it has reached a left corner
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
                }//else keep looping until it finds left
                
            }else if(state == LeftCornering){ // set starting distance for forward movements after turning left using encoders
                turn('L', 1.9, false);
                if(previous_state == CorridorReverse){
                    updateState(FindLeft);
                    startingDistance = encoders.getCountsRight();
                }else{
                        updateState(ForwardFindLeft);
                        startingDistance = encoders.getCountsRight();
                }
            
            }else if(state == CorrectLeft){
                updateState(FindLeft);
            }else if(state == CorrectRight){
                turn('I', 1, false);
                if(state == ForwardFindLeft){
                    updateState(previous_state);
                }else if(state == CheckForward){
                    updateState(previous_state);
                }else{
                    updateState(FindLeft);
                }
                // refind left or continue dependant on previous states
            }else if(state == CheckForward){ // use encoders to move forward to the start or end of a corridor or to the middle of a room.
                
                int cpr = (float)750 * (float) 1.5;
                drive(FORWARD_SPEED , FORWARD_SPEED);

                int16_t countsRight = encoders.getCountsRight();

                bool errorRight = encoders.checkErrorRight();
                int16_t startingCount = countsRight;

                if((int32_t)countsRight > (startingDistance + cpr)){
                    // did distance without hitting wall check for person in the room
                    drive(0,0);
                    proximityPersonCheck(true);
                    updateState(ForwardFindLeft);
                    startingDistance = encoders.getCountsRight();
                }
                
            }else if(state == ForwardFindLeft){ // use encoders to move forward past a room and find the left wall

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
    //}
}

void runModeTwo(){
    modeTwoTakeOver = true;
    // switch's to manual override when state has stopped movement.
    if(state == ModeTwoWait){
     if(incomingMessage){
      
      switch(commandType){
        case 'L':
            printConsoleVariable("left turn activated ");
            turn('L', 2, false);
            previous_state = LeftCornering;
            updateState(FindLeft);
        break;
        case 'R':
            printConsoleVariable("right turn activated ");
            turn('R', 2, false);
            previous_state = RightCornering;
            updateState(FindLeft);
        break;
        case 'F':
            printConsoleVariable("forward activated ");
            startingDistance = encoders.getCountsRight();
            updateState(CheckForward);
        break;
      }

     }
    }else{

        semiControl();
    }
}
