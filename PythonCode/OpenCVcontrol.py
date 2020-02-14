from queue import Queue
import numpy as np
import cv2

def run(cvQueue: Queue):
    cvObject = OpenCVController()

    while True:
        if not cvQueue.empty():  # If there's something in the queue...
            commandFromQueue = cvQueue.get()
            cvQueue.task_done()
            if commandFromQueue == "personFollow":
                cvObject.person_following(cvQueue)
                return


class OpenCVController:
    cap = cv2.VideoCapture(0)

    def __init__(self):
        cap = cv2.VideoCapture(0)
        # Check whether user selected camera is opened successfully.
        if not (self.cap.isOpened()):
            print("cant open")
        else:
            print("Opened")
            cap.set(cv2.CAP_PROP_FRAME_HEIGHT, 480)
            cap.set(cv2.CAP_PROP_FRAME_WIDTH, 480)

    def person_following(self, cvQueue: Queue):
        print("start following")
        while (True):
            # Capture frame-by-frame for reference
            ret, frame = self.cap.read()
            cv2.imshow("preview", frame)
            #apply filter using & on a pixel by pixel format
            hsv = cv2.cvtColor(frame, cv2.COLOR_BGR2HSV)
            l_b = np.array([66, 40, 50])
            u_b = np.array([168, 255, 245])
            #so far we are just setting the color bound to green/blue stuff
            #now we use the filter
            msk = cv2.inRange(hsv, l_b, u_b)#set the filter
            res = cv2.bitwise_and(frame, frame, mask=msk)#apply the filter
            # Display the resulting frame
            cv2.imshow("res", res)

            # Waits for a user input to quit the application
            if cv2.waitKey(1) & 0xFF == ord("q"):
                break
        return
