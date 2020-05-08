import multiprocessing as mp
import time
import threading as th
import queue as qu
from flask import Flask, render_template
from flask_ask import Ask, statement, question

import OpenCVcontrol
#import other files if you want

#you may add more queues to handle other stuff
cvQueue = mp.JoinableQueue()


allQueues = []
allQueues.append(cvQueue)


app = Flask(__name__)
ask = Ask(app, '/')


@ask.launch
def launched():
	return question("hello. what would you like me to do?").reprompt(
		"if you don't need me, please put me to sleep.")

@ask.intent('AMAZON.FallbackIntent')
def default():
	return question("Sorry, I don't understand that command. What would you like me to do?").reprompt(
		"What would you like me to do now?")

#the intent has to have the same name as what you have on alexa.
#add other intents as needed
@ask.intent('terminateRobot')
def terminate():
    for queue in allQueues:
        queue.put("terminate")
    return statement('processes are terminated')

@ask.intent('trackMe')
def trackMe():
    cvQueue.put("personFollow")
    cvQueue.join()
    return statement('I\'m following you now. Please do not run')

if __name__ == '__main__':
    alexaTh = th.Thread(target= app.run)
    alexaTh.daemon = True
    cvTh = mp.Process(target=OpenCVcontrol.run, args=(cvQueue,))
    #you may have more threads for other tasks, just append them all toghther
    
    allThreads = []
    allThreads.append(cvTh)

    alexaTh.start()

    for thread in allThreads:
        thread.start()

    for thread in allThreads:
        thread.join()



    print("exiting")

