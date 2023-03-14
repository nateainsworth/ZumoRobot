#pragma once

#include <Wire.h>
#include <Zumo32U4.h>



void runModeOne(){
    if(incomingMessage){
        manualMove(commandType, false);
    }
    
}