from enum import Enum
from queue import Queue

import os
import glob
import numpy as np
import time
import json

class Direction(Enum):
    FORWARD = 1
    BACKWARD = 2
    LEFT = 3
    RIGHT = 4
    STOP = 5

def run(cvQueue: Queue):
    cvObject = OpenCVController()

#the loop below takes a command and checks what can be invoked.
    while True:
        if not cvQueue.empty():  # If there's something in the queue...
            commandFromQueue = cvQueue.get()
            cvQueue.task_done()
            
            if commandFromQueue == "terminate":
                print("Terminate OpenCV")
                return
               
            if commandFromQueue == "personFollow":
                cvObject.person_following(cvQueue)
                
            # you may add other commands and other functions
            




class OpenCVController:
    def __init__(self):
        #put your initializer here
        #like open the camera/caliberate the camera 
        pass

#this is an example function you can replace with

    def person_following(self, cvQueue: Queue):
    
        #the tracking function setup part should be here
        print("start following")
                    if not objectSeenOnce:
                    else:  # Object has been seen before
                        if leftOrRightLastSent is not None:
                            if leftOrRightLastSent == Direction.RIGHT:
                                self.send_serial_command(Direction.RIGHT, b'r');
                                commandString = "SEARCHING: GO RIGHT"
                            elif leftOrRightLastSent == Direction.LEFT:
                                self.send_serial_command(Direction.LEFT, b'l');
                                commandString = "SEARCHING: GO LEFT"
                        else:  # variable hasn't been set yet (seems unlikely), but default to left
                            self.send_serial_command(Direction.LEFT, b'l');
        
        #the loop below should be the main loop for tracking
        while True:
        
            print("now following ")
            
            #this part is to detect whether we have new orders 
            #if so we need to break out of this loop
            #this is essential! make sure to include it!
            if not cvQueue.empty():
                print("new order detected")
                break
            
            
        print("done following")
        return

