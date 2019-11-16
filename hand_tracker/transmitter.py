import serial


class Transmitter():

    def __init__(self, channel, baudrate=115200):
        self.serial = serial.Serial()
        self.serial.port = channel
        self.serial.baudrate = 115200
        self.serial.open()


    def send_axis_values(self, pitch, roll, yaw, throttle)
        pass