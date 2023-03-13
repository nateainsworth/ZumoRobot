# Zumo Seach and Rescue Robot
The Project is made up of two sides one being the Graphical user interface and the other being the programmed Zumo Robot. The GUI and robot are both configured to talk to the robot using two Xbee devices to open a serial communication stream. The Zumo project can be uploded onto a Zumo bot but may need configuring through the GUI or directly by updating it's global variables. The Zumo features three different modes, for searching and rescuing. The GUI offers a method of controlling, changing drive modes and setting up the Zumo robot sensors and speed. The GUI features a thread pool to handle incoming messages efficiently from the serial communication channel.


## **Table of Contents**
- [**Setup and Runing the Application**](#setup-and-running-the-application)
    - [**Setup the Zumo**](#setting-up-the-zumo)
    - [**Running the Zumo**](#running-the-zumo)
    - [**Setup the Graphical User Interface**](#setting-up-the-gui)
    - [**Running the Graphical User Interface**](#running-the-gui)
- [**Different Driving Modes**](#different-driving-modes)
    - [**Mode One**](#mode-one)
    - [**Mode Two**](#mode-two)
    - [**Mode Three**](#mode-three)
- [**Manual GUI Commands**](#manual-gui-commands)
- [**References**](#references)


# Setup and Running the Application

## Setting up the Zumo
1. Setting up the Zumo, `open` the Zumo application into Visual Studio Code or your desired arduino IDE.
2. `Connect` your Zumo to the PC via USB.
3. Press the `upload` button provided it has worked you will be presented by a console message saying success. 


## Running the Zumo
1. Turn the Zumo on.
2. `Wait for the orange light to go off` indicating that the Zumo has calibrated it's Gyro. You will also notice values changing on the GUI.
3. Using the GUI `select the desired mode` you wish to run the Zumo on.
4. The Zumo will then activate and operate in the chosen mode.
5. If required the `reset` button on the Zumo can be used to revert it back to the setup.

<br />

## Setting up the GUI
1. Load the GUI application in `Processing 4`.
2. The GUI will need to be configured to talk over the correct serial port. `plug in` the communicator Xbee. 
3. Using a `console log` print the ports list to display all possible ports to connect to.
4. `Run` the application to see the console update.
5. Work out the post `index` within the list count the index of the port you need to connect to starting from 0.
6. Update the ID `String portName = Serial.list()[NEW ID HERE];`.
7. Save the project select `File` form the menu and click `Export`.
8. The project will now run without processing connecting to your Zumo.

## Running The GUI
1. Run the `.app` file created by the setup step.
2. Use the features provided by the GUI to communicate and receive information on the Zumo bot. These are as follows.
- Manual commands written in the following format <W>
- Mode Selection
- Manual direction controls you can also use `arrow keys` while on the gui to control the Zumo.
- Speed sliders for increasing or decreasing the speed of the Zumo.
- A button for disabling the motor on the zumo, this can be handy when setting up line sensor thresholds.
- Sliders for setting the `Minimum` thresholds of the left and right sensors. to allow it to find a line.
- Sliders for setting the `Maximum` thresholds of the left, middle and right sensors. To stop the Zumo from driving too far over the line.
- Sensor readings which are updated frequently. This displays gyro readings, line sensor readings, proximity readings and encoder distances.
- Lastly the console which can be paused however be aware pausing the console will back up incoming serial communication.

<br />

# Different Driving Modes
Upon turning on the Zumo it will configure it's Gyro and wait for a mode to be selected in the GUI.

## Mode One
Full control of the Zumo using the GUI buttons, the Zumo will makes use of proximity sensors on timers to detect find objects representing humans to be rescued. It will make a buzz and send a message to the GUI upon detection.

## Mode Two
Partial control the Zumo, the Zumo will drive autonomously within black lines using line sensors, until it finds rooms, corridors or dead ends, at which point it will change to manual mode for intervention with the GUI to send it off on its next direction. During it's journey it will pick up objects representing humans to be rescued,it will make a buzz and send a message to the GUI upon detection.

## Mode Three
Full autonomous mode drives the robot using a set of conditions to work out it's positioning and current state of travel. It makes use previous states and current states to work out it's next move. It follows the left line (wall) until it finds a room. it drives to the middle of the room and scans for objects representing humans to be rescued. It will make a buzz and send a message to the GUI upon detection, it then continues on. If the Zumo loses a left line it drives forward to work out if it has reached a room or a corridor. if it comes across a line on it's left before it has reached half way across a room then it knows it has reached a corridor.

# Manual GUI Commands
Sending commands to the Zumo using the manual command field.
    - Turn left `<A>`
    - Turn Right `<D>`
    - Go Forward `<W>`
    - Go Backward `<S>`
    - Load Mode One `<M:1>`
    - Load Mode Two `<M:2>`
    - Load Mode Three `<M:3>`
    - Set left and right min threshold `<I:500,600>` 500 = left and 600 = right
    - Set left middle and right max threshold `<O:500,600,700>` 500 = left, 600 = right, 700 = front / middle
    - Set Speed `<U:200>` 200 being the new speed
    - Disable Motor `<M:4>`


# References
Pololu. (2021, November 12). Zumo-32U4-Arduino-Library example SumoProximitySensors. GitHub. Retrieved March 13, 2023, from https://github.com/pololu/zumo-32u4-arduino-library/blob/a5945ce13496f92f9311d4622e43ff08d42cf57c/examples/SumoProximitySensors 

Pololu. (2021, November 16). Zumo-32U4-Arduino-library examples mazesolver . GitHub. Retrieved March 13, 2023, from https://github.com/pololu/zumo-32u4-arduino-library/tree/a5945ce13496f92f9311d4622e43ff08d42cf57c/examples/MazeSolver 

Pololu. (2022, September 7). Zumo-32U4-Arduino-Library examples encoders. GitHub. Retrieved March 13, 2023, from https://github.com/pololu/zumo-32u4-arduino-library/tree/f4dfe054e23176ba445748b4b91f463701e7eb76/examples/Encoders 

Robin2 . (2014, December). Serial input basics. Arduino Forum. Retrieved March 13, 2023, from https://forum.arduino.cc/t/serial-input-basics/278284/40 

UKHeliBob. (2017, October 2). Using millis() for timing. A beginners guide. Arduino Forum. Retrieved March 13, 2023, from https://forum.arduino.cc/t/using-millis-for-timing-a-beginners-guide/483573 

Israr, T. (2020, March 17). Measure distance using encoders - zumo 32U4. Pololu Forum. Retrieved March 13, 2023, from https://forum.pololu.com/t/measure-distance-using-encoders-zumo-32u4/19076 