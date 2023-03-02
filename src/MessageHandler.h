#pragma once

#include <Wire.h>
#include <Zumo32U4.h>


// --------------------- Retrive serial messages --------------------- 


// Modified a version of reading different incoming information from serial
// Robin2. (2014, December 1). Serial input basics. Arduino Forum. Retrieved February 26, 2023, from https://forum.arduino.cc/t/serial-input-basics/278284/73 
const byte numChars = 32;
char incomingChars[numChars]; // an array to store the received data
boolean incomingMessage = false;
char commandType;

void retrieveSerial() {
  static boolean recvInProgress = false;
  static boolean commandReceived = false;

    static byte ndx = 0;
    char rc;
    

    while (Serial1.available() > 0 && incomingMessage == false) {
        rc = Serial1.read();

        if (recvInProgress == true) {
          // if doesn't = the end  of message incoming ---
            if (rc != '>') {

                if(!commandReceived){
                    commandType = rc;
                    commandReceived = true;

                }else{
                  if(rc != ':' ){
                    incomingChars[ndx] = rc;
                    ndx++;
                    if (ndx >= numChars) {
                        ndx = numChars - 1;
                    }
                  }
                }
            }
            else {
              
                incomingChars[ndx] = '\0';
                recvInProgress = false;
                ndx = 0;
                incomingMessage = true;
                commandReceived = false;
                
            }
        }
        
        else if (rc == '<') {
            recvInProgress = true;
        }
    }
}  


// ---------------------  Print Messages --------------------- 

void printGyro(int angle){
  static char buffer[10];
  sprintf(buffer, "<G:%d>",
    angle
  );
  Serial1.print(buffer);

}

void printConsoleVariable(String variable){
  static char buffer[80];
  sprintf(buffer, "<E:%s>",
    variable.c_str()
  );
  Serial1.println(buffer);

}

void printLineSensors(int line1, int line2, int line3){
  static char buffer[80]; 
  sprintf(buffer, "<L:%d,%d,%d>",
    line1,
    line2,
    line3
  );
  Serial1.println(buffer);
  Serial.println("line sensor:");
  Serial.println(buffer);

}

void printEncoders(int countsLeft, int countsRight, bool errorLeft, bool errorRight){
  static char buffer[80]; 
  sprintf(buffer, "<R:%d,%d,%d,%d>",
    countsLeft,
    countsRight,
    errorLeft,
    errorRight
  );
  Serial1.println(buffer);
  Serial.println(buffer);
}


void printProximity()
{
  static char buffer[80];
  sprintf(buffer, "<P:%d,%d,%d,%d,%d,%d>",
    proxSensors.countsLeftWithLeftLeds(),
    proxSensors.countsLeftWithRightLeds(),
    proxSensors.countsFrontWithLeftLeds(),
    proxSensors.countsFrontWithRightLeds(),
    proxSensors.countsRightWithLeftLeds(),
    proxSensors.countsRightWithRightLeds()
  );
  Serial1.println(buffer);
  Serial.println(buffer);
}

void printAllSensors(int countsLeft, int countsRight, bool errorLeft, bool errorRight, int line1, int line2, int line3)
{
  static char buffer[80];
  sprintf(buffer, "<A:%d,%d,%d,%d,%d,%d|%d,%d,%d,%d|%d,%d,%d>",
    proxSensors.countsLeftWithLeftLeds(),
    proxSensors.countsLeftWithRightLeds(),
    proxSensors.countsFrontWithLeftLeds(),
    proxSensors.countsFrontWithRightLeds(),
    proxSensors.countsRightWithLeftLeds(),
    proxSensors.countsRightWithRightLeds(),
    countsLeft,
    countsRight,
    errorLeft,
    errorRight,
    line1,
    line2,
    line3
  );
  Serial1.println(buffer);
  Serial.println(buffer);
}

void printMovementUpdate(String command, int distance,String commandTwo, int distanceTwo){
  if(importantReceived){
    static char buffer[32]; 
    sprintf(buffer, "<D:%s,%d,%s,%d>",
      command.c_str(),
      distance,
      commandTwo.c_str(),
      distanceTwo
    );
    importantReceived = false;
    movementCount = movementCount + 2;
    Serial1.println(buffer);
    Serial.println(buffer);
    
  }else{
    Serial1.println(importantMessageBuffer);
    while(!importantReceived){
      retrieveSerial();
      if(incomingMessage){
        if(commandType == 'Q'){
          if((int)incomingMessage == movementCount){
            importantReceived = true;
          }else{
            Serial.println("error with movements");
          }
          
        }else if(commandType == 'X'){//if movement message was corrupted resend;
          //TODO USE VALUE TO CHECK CURRENT MOVEMENT COUNT BEFORE RESENDING TO AVOID DUPLICATES
          Serial1.println(importantMessageBuffer);
        }else{
          Serial.println('Command' + String(commandType) + ": issue incoming during while loop");
        }
      }
    }
  }
}



// --------------------- Parse Serial Messages --------------------- 

char tempChars[32]; 

void parseSliders ( int *sliders, int quantity){

  char* ptr = strtok(incomingChars, ",");
  sliders[0] = atol(ptr);
  ptr = strtok(NULL, ",");
  sliders[1] = atol(ptr);

  if(quantity = 3){
    ptr = strtok(NULL, ",");
    sliders[2] = atol(ptr);
  }

}
