#pragma once

#include <Wire.h>
#include <Zumo32U4.h>



void runModeOne(){
    if(incomingMessage){
        manualMove(commandType, false);
        currentTime = millis();

        if(currentTime - lastPersonCheckMillis >= 3000)
        {
            lastPersonCheckMillis = currentTime;
            proximityPersonCheck(false);
        }
           
    }
    
}