import multiprocessing as mp
import OpenCVcontrol

cvQueue = mp.JoinableQueue()
allQueues = []
allQueues.append(cvQueue)

if __name__ == '__main__':
    cvTh = mp.Process(target=OpenCVcontrol.run, args=(cvQueue,))

    allThreads = []
    allThreads.append(cvTh)
    cvQueue.put('personFollow')

    for thread in allThreads:
        thread.start()

    for thread in allThreads:
        thread.join()



    print("exiting")

