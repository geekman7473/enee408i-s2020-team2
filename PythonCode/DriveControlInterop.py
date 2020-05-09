import serial
import struct

COMMAND_END = "!"
LEFT_MOTOR = 0
RIGHT_MOTOR = 1
class DriveController:

    def __init__(self, comm_port):
        self.serial = serial.Serial(comm_port)
        

    def send_speed(self, right_speed, left_speed):
        message = ["SPEED", right_speed, left_speed]

        str_message = " ".join([str(c) for c in message]) + COMMAND_END

        self.serial.write(str_message.encode("UTF-8"))

    def read_counts(self, motor):
        message = ["GET_COUNT", motor]
        str_message = " ".join([str(c) for c in message]) + COMMAND_END
        #print(str_message)
        self.serial.write(str_message.encode("UTF-8"))

        b_resp = self.serial.read(4)
        return int.from_bytes(b_resp, byteorder='little', signed = True)

    def read_speeds(self, motor):
        message = ["GET_SPEED", motor]
        str_message = " ".join([str(c) for c in message]) + COMMAND_END
        self.serial.write(str_message.encode("UTF-8"))

        b_resp = self.serial.read(4)
        return struct.unpack("<f", b_resp)
        
    
if __name__ == "__main__":
    import time
    test = DriveController("/dev/ttyACM0")
    while True:
        time.sleep(1)
        #print(test.read_counts(RIGHT_MOTOR))
        #print(test.read_counts(LEFT_MOTOR))
        print(test.read_speeds(RIGHT_MOTOR)[0], test.read_speeds(LEFT_MOTOR)[0])
        #print(test.read_counts(LEFT_MOTOR), test.read_counts(RIGHT_MOTOR))
