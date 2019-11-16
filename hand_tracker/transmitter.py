import serial,struct


# Name convention
# rudder = pitch
# throttle = roll
# elevator = throttle
# aileron = yaw
class Transmitter():

    def __init__(self, channel, baudrate=115200):

        try:
            self.serial = serial.Serial()
            self.serial.port = channel
            self.serial.baudrate = 115200
            self.serial.open()
        except Exception:
            print("Serial not connected")


    def scale(self, value):
       return int(1000 + (value * 1000))


    def send_axis_values(self, pitch, roll, yaw, throttle):
        data_payload = struct.pack('>4H', self.scale(roll), self.scale(pitch), self.scale(throttle), self.scale(yaw))
        try:
            self.serial.write(data_payload)
        except Exception:
            print("Serial not connected")
