#pragma once

#include <Wire.h>
#include <Zumo32U4.h>

void automation(){
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
            proximityPersonCheck(false);
        }else if(state == RightCornering){
            turn('R', 2, false);
            printConsoleVariable("right corner");

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

        }else if(state == LeftCornering){
            printConsoleVariable("left corner");
            turn('L', 1.9, false);
            if(previous_state == CorridorReverse){
              updateState(FindLeft);
              startingDistance = encoders.getCountsRight();
            }else{
              updateState(ForwardFindLeft);
              startingDistance = encoders.getCountsRight();
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
            updateState(LeftCornering);
            printConsoleVariable("Entering corridor");
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
           
        }else if(state == Scanning){
            drive(0,0);
            proximityPersonCheck(false);
        }else if(state == FindLeft){
            turn('l', 1,false);
            //Still didn't find left
            if(previous_state == FollowingLeft){
              lostLeft++;
            }
            // couldn't find left more than 4 times must be corner
            if(lostLeft  >= 4 ){
                turn('O', lostLeft, false);
                lostLeft = 0;
                // check forward until distance reached
                updateState(CheckForward);
                startingDistance = encoders.getCountsRight();
            }//else leave to find left
            
        }else if(state == LeftCornering){
            printConsoleVariable("left corner");
            turn('L', 1.9, false);
             if(previous_state == CorridorReverse){
                updateState(FindLeft);
            }else{
              updateState(ForwardFindLeft);
              startingDistance = encoders.getCountsRight();
            }
        
        }else if(state == LosingLeftLine){
            turn('I', 1,false);
        }else if(state == CorrectLeft){
          //TODO test thisonly occured on home circut
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
        }else if(state == CheckForward){
            int cpr = (float)750 * (float) 1.5;
            drive(FORWARD_SPEED, FORWARD_SPEED);

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
                printConsoleVariable("past room on left");
            }
        }
    }
}

void runModeThree(){
    automation();
}
