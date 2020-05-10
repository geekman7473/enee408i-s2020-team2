import serial
import struct

COMMAND_END = "!"
LEFT_MOTOR = 0
RIGHT_MOTOR = 1
class DriveController:

    def __init__(self, comm_port, rate=57600):
        self.serial = serial.Serial(comm_port, baudrate=rate, timeout=1)
        self.serial.write("GET_SPEED 1!".encode("ascii"))
        b_resp = self.serial.read(4)
        
        

    def send_speed(self, right_speed, left_speed):
        message = ["SPEED", "{:.2f}".format(right_speed), "{:.2f}".format(left_speed)]

        str_message = " ".join([str(c) for c in message]) + COMMAND_END
        self.serial.write(str_message.encode("ascii"))
        self.serial.flush()
        #b_resp = self.serial.readline()
        #print(b_resp)
        #print(str.decode(b_resp,"UTF-8"))
        
        
    def read_counts(self, motor):
        message = ["GET_COUNT", motor]
        str_message = " ".join([str(c) for c in message]) + COMMAND_END
        print(str_message)
        self.serial.write(str_message.encode("ascii"))

        b_resp = self.serial.read(4)
        return int.from_bytes(b_resp, byteorder='little', signed = True)

    def read_speeds(self, motor):
        message = ["GET_SPEED", motor]
        str_message = " ".join([str(c) for c in message]) + COMMAND_END
        self.serial.write(str_message.encode("ascii"))

        b_resp = self.serial.read(4)
        return struct.unpack("<f", b_resp)[0]
        
    
if __name__ == "__main__":
    import time
    test = DriveController("/dev/ttyACM0")
    #print(test.read_counts(RIGHT_MOTOR))
    #print(test.read_counts(LEFT_MOTOR))
    
    #time.sleep(10)
    #print(test.read_speeds(RIGHT_MOTOR)[0], test.read_speeds(LEFT_MOTOR)[0])
    #print(test.read_counts(LEFT_MOTOR), test.read_counts(RIGHT_MOTOR))
    #test.send_speed(0,0)
    t = 0
    test.send_speed(5.0,-5.0)
    while(t <100):
        #test.send_speed(1,-1)
        print(test.read_speeds(LEFT_MOTOR),test.read_speeds(RIGHT_MOTOR))
        time.sleep(.1)
        t+=1
    test.send_speed(0,0)
