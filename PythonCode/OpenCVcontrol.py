from enum import Enum
from queue import Queue

import os
import glob
import imutils
import numpy as np
import cv2
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

    while True:
        if not cvQueue.empty():  # If there's something in the queue...
            commandFromQueue = cvQueue.get()
            cvQueue.task_done()
            if commandFromQueue == "personFollow":
                cvObject.person_following(cvQueue)
            elif commandFromQueue =="calibrate":
                cvObject.calibrateP1(cvQueue)
                cvObject.calibrateP2(cvQueue)

            return



class OpenCVController:

    def __init__(self):
        self.cap = cv2.VideoCapture(1)
        with open('cameraParams.json', 'r') as f:
            data = json.load(f)
        self.cameraMatrix = np.array(data['cameraMatrix'], dtype=np.float32)
        self.distCoeffs = (np.array(data['distCoeffs'], dtype=np.float32))

        # Check whether user selected camera is opened successfully.
        if not (self.cap.isOpened()):
            print("cant open")
        else:
            print("Opened")
            self.cap.set(cv2.CAP_PROP_FRAME_HEIGHT, 480)
            self.cap.set(cv2.CAP_PROP_FRAME_WIDTH, 480)

    def person_following(self, cvQueue: Queue):
        print("start following")
        # Below are variables to set what we consider center and in-range
        radiusInRangeLowerBound, radiusInRangeUpperBound = 80, 120
        centerRightBound, centerLeftBound = 400, 200
        radiusTooCloseLowerLimit = 250

        while (True):
            # Capture frame-by-frame for reference
            ret,frameDistorted = self.cap.read()
            cv2.imshow("preview", frameDistorted)
            frame=cv2.fisheye.undistortImage(frameDistorted,self.cameraMatrix,self.distCoeffs);
            #apply filter using & on a pixel by pixel format
            blurred = cv2.GaussianBlur(frame, (5, 5), 0)
            hsv = cv2.cvtColor(blurred, cv2.COLOR_BGR2HSV)
            l_b = np.array([66, 40, 50])
            u_b = np.array([168, 255, 245])
            #so far we are just setting the color bound to green/blue stuff
            #now we use the filter
            msk = cv2.inRange(hsv, l_b,u_b)
            msk = cv2.erode(msk, None, iterations=2)  # TODO: these were 3 or 5 before (more small blob removal)
            msk = cv2.dilate(msk, None, iterations=2)
            res = cv2.bitwise_and(frame, frame, mask=msk)#apply the filter
            cv2.imshow("preview", frameDistorted)
            cnts = cv2.findContours(msk.copy(), cv2.RETR_EXTERNAL, cv2.CHAIN_APPROX_SIMPLE)
            cnts = imutils.grab_contours(cnts)

            objectSeenOnce = False  # Object has never been seen before
            leftOrRightLastSent = None  # Keep track of whether we sent left or right last

            # Only proceed if at least one contour was found
            # If nothing is found, then look around OR send the STOP command to halt movement (depends on situation)
            if len(cnts) == 0:
                # If we haven't seen the object before, then we'll stay halted until we see one. If we HAVE seen the
                # object before, then we'll move in the direction (left or right) that we did most recently
                if not objectSeenOnce:
                    self.send_serial_command(Direction.STOP, b'h')
                    commandString = "STOP"
                else:  # Object has been seen before
                    if leftOrRightLastSent is not None:
                        if leftOrRightLastSent == Direction.RIGHT:
                            self.send_serial_command(Direction.RIGHT, b'r')
                            commandString = "SEARCHING: GO RIGHT"
                        elif leftOrRightLastSent == Direction.LEFT:
                            self.send_serial_command(Direction.LEFT, b'l')
                            commandString = "SEARCHING: GO LEFT"
                    else:  # variable hasn't been set yet (seems unlikely), but default to left
                        self.send_serial_command(Direction.LEFT, b'l')
                        commandString = "DEFAULT SEARCHING: GO LEFT"

            elif len(cnts) > 0:  # Else if we are seeing some object...

                # Find the largest contour in the mask and use it to compute the minimum enclosing circle and centroid
                c = max(cnts, key=cv2.contourArea)
                ((x, y), radius) = cv2.minEnclosingCircle(c)
                M = cv2.moments(c)
                center = (int(M["m10"] / M["m00"]), int(M["m01"] / M["m00"]))

                filteredPtsRadius = [radius]

                # Only consider it to a valid object if it's big enough - else it could be some other random thing
                if filteredPtsRadius[0] <= 25:
                    # TODO this is the same code as the block above - I should extract these out to a function
                    # If we haven't seen the object before, then we'll stay halted until we see one
                    # If we HAVE seen the object before, then we'll move in the direction (left or right) that we did
                    # most recently
                    if not objectSeenOnce:
                        self.send_serial_command(Direction.STOP, b'h');
                        commandString = "STOP";
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
                            commandString = "DEFAULT SEARCHING: GO LEFT"

                else:  # This object isn't super small ... we should proceed with the tracking
                    if not objectSeenOnce:
                        objectSeenOnce = True

                    #  draw the circle on the frame TODO consider removing this eventually - could speed things up (barely)
                    cv2.circle(frame, (int(x), int(y)), int(filteredPtsRadius[0]), (0, 255, 255), 2)
                    cv2.circle(frame, center, 5, (0, 0, 255), -1)
                    filteredPtsX = [center[0]]
                    filteredPtsY = [center[1]]


                    if filteredPtsRadius[0] > radiusTooCloseLowerLimit:
                        commandString = "MOVE BACKWARD - TOO CLOSE TO TURN"
                        self.send_serial_command(Direction.BACKWARD, b'b')
                    elif filteredPtsX[0] > centerRightBound:
                        commandString = "GO RIGHT"
                        self.send_serial_command(Direction.RIGHT, b'r')
                        if leftOrRightLastSent != Direction.RIGHT:
                            leftOrRightLastSent = Direction.RIGHT
                    elif filteredPtsX[0] < centerLeftBound:
                        commandString = "GO LEFT"
                        self.send_serial_command(Direction.LEFT, b'l')
                        if leftOrRightLastSent != Direction.LEFT:
                            leftOrRightLastSent = Direction.LEFT
                    elif filteredPtsRadius[0] < radiusInRangeLowerBound:
                        commandString = "MOVE FORWARD"
                        self.send_serial_command(Direction.FORWARD, b'f')
                    elif filteredPtsRadius[0] > radiusInRangeUpperBound:
                        commandString = "MOVE BACKWARD"
                        self.send_serial_command(Direction.BACKWARD, b'b')
                    elif radiusInRangeLowerBound < filteredPtsRadius[0] < radiusInRangeUpperBound:
                        commandString = "STOP MOVING - IN RANGE"
                        self.send_serial_command(Direction.STOP, b'h')


            # Display the resulting frame
            cv2.imshow("res", frame)

            # Waits for a user input to quit the application
            if cv2.waitKey(1) & 0xFF == ord("q"):
                break
        return

    def calibrateP1(self, cvQueue: Queue):
        i=0
        print("calibrating\n")
        while (True):
            # Capture frame-by-frame for reference
            ret,frameDistorted = self.cap.read()
            cv2.imshow("raw",frameDistorted)
            k=cv2.waitKey(1)
            if k & 0xFF == ord("s"):
                t1 = time.clock()
                t2 = time.clock()
                while(cv2.waitKey(1)& 0xFF != ord("p")):
                    ret, frameDistorted = self.cap.read()
                    if time.clock()-t1>0.500:
                        t1=time.clock()
                        cv2.imwrite(filename='saved_img' + str(i) + '.jpg', img=frameDistorted)
                        i = i + 1
                        cv2.imshow("res", frameDistorted)
                        t2=time.clock()
                    if time.clock() - t2 > 0.500:
                        cv2.destroyWindow("res")
                    if(i>40):
                        break
                cv2.destroyWindow("raw2")
            elif k & 0xFF == ord("q"):
                cv2.destroyAllWindows()
                break

        return

    def calibrateP2(self, cvQueue: Queue):
        CHECKERBOARD = (7, 9)
        subpix_criteria = (cv2.TERM_CRITERIA_EPS + cv2.TERM_CRITERIA_MAX_ITER, 30, 0.1)
        calibration_flags = cv2.fisheye.CALIB_RECOMPUTE_EXTRINSIC + cv2.fisheye.CALIB_CHECK_COND + cv2.fisheye.CALIB_FIX_SKEW
        objp = np.zeros((1, CHECKERBOARD[0] * CHECKERBOARD[1], 3), np.float32)
        objp[0, :, :2] = np.mgrid[0:CHECKERBOARD[0], 0:CHECKERBOARD[1]].T.reshape(-1, 2)
        _img_shape = None
        objpoints = []  # 3d point in real world space
        imgpoints = []  # 2d points in image plane.
        images = glob.glob('*.jpg')
        for fname in images:
            img = cv2.imread(fname)
            if _img_shape == None:
                _img_shape = img.shape[:2]
            else:
                assert _img_shape == img.shape[:2], "All images must share the same size."
            gray = cv2.cvtColor(img, cv2.COLOR_BGR2GRAY)
            # Find the chess board corners
            ret, corners = cv2.findChessboardCorners(gray, CHECKERBOARD,
                                                     cv2.CALIB_CB_ADAPTIVE_THRESH + cv2.CALIB_CB_FAST_CHECK + cv2.CALIB_CB_NORMALIZE_IMAGE)
            # If found, add object points, image points (after refining them)
            if ret == True:
                objpoints.append(objp)
                cv2.cornerSubPix(gray, corners, (3, 3), (-1, -1), subpix_criteria)
                imgpoints.append(corners)
        N_OK = len(objpoints)
        K = np.zeros((3, 3))
        D = np.zeros((4, 1))
        rvecs = [np.zeros((1, 1, 3), dtype=np.float64) for i in range(N_OK)]
        tvecs = [np.zeros((1, 1, 3), dtype=np.float64) for i in range(N_OK)]
        rms, _, _, _, _ = \
            cv2.fisheye.calibrate(
                objpoints,
                imgpoints,
                gray.shape[::-1],
                K,
                D,
                rvecs,
                tvecs,
                calibration_flags,
                (cv2.TERM_CRITERIA_EPS + cv2.TERM_CRITERIA_MAX_ITER, 30, 1e-6)
            )
        print("Found " + str(N_OK) + " valid images for calibration")
        print("DIM=" + str(_img_shape[::-1]))
        print("K=np.array(" + str(K.tolist()) + ")")
        print("D=np.array(" + str(D.tolist()) + ")")

    def send_serial_command(self,d, param):
        if d==Direction.STOP:
            print("Stop\n")
        elif d== Direction.BACKWARD:
            print("Back\n")
        elif d == Direction.FORWARD:
            print("Back\n")
        elif d== Direction.LEFT:
            print("Left\n")
        elif d == Direction.RIGHT:
            print("Right\n")
        else:
            pass
