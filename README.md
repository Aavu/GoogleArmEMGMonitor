# ECE 6122 Final project
## Google Arm EMG Monitor
### Introduction
This app is designed to work with the Google Prosthetic Arm built in the Robotic Musicianship Lab. The arm aids prosthetic drummers (forearm) to be able to play the drums with grip control!.

This app helps in easy data collection for model training and also aids monitoring the raw sensor data.

### Architecture and Data Flow
The client facing app is built using the QT framework. The plotting uses the 3rd-party library - QCustomPlot. It is a light weight header only library that is easy to use. The client app runs on the user's own computer. The server runs on the Raspberry pi inside the arm. It collects data from the EMG sensors using CANbus protocol and streams data over TCP along with the keypress strokes recorded from a connected keyboard to the Client app. The client app displays the received data on screen. The user can just click a button to record data along with ground truth (the key press) for each gestures.

### Some notable design choices
* The client TCP is implemented by inheriting the QTCPSocket class from QT framework while the server TCP is implemented from scratch. 
* The client app is written for Mac OSX and the server is written for the Linux (Raspbian buster).
* All the data buffer uses a custom implementation of Ring buffer / Circular Queue
* Inspired from Digital Audio Workstations, the recording uses a temp file to record data and then transfer to an actual txt file only when the user decides to save the session.
* The client can ssh onto the device and start the server automatically without any extra steps from the users!
* Each component works on its own thread.

## Installation
* To build the client app, you need qmake. The easiest way is to download [QT Creater](https://www.qt.io/product/development-tools) and compile it within the IDE. All the necessary sources are in the project. No extra steps needed.
* To build the server app, you need cmake. In the device's raspberry pi, Use the following commands after cloning the repo. If using seperate zip files, please skip the first 2 steps.
    * ```cd GoogleArmEMGMonitor```
    * ```git checkout server```
    * ```mkdir build && cd build```
    * ```cmake .. && make```
    * No need to manually start the server, the  client app will automatically start it.
    * Before connecting to server, make sure the server path is set correctly in preferences.

## Check list
* Custom classes used
* Sockets
* Threads
