import Leap
import math

class MotionCapturer:

    def __init__(self):
        self.controller = Leap.Controller()


    def get_axis_values(self):
        frame = self.controller.frame()
        if len(frame.hands) == 0:
            print("No hand detected")
            return 0, 0, 0, 0
        elif len(frame.hands) > 1:
            print("More than one hand detected")
            raise ValueError

        hand = frame.hands[0]
        pitch = 1 - self.normalize(hand.direction.pitch)
        roll = 1 - self.normalize(hand.palm_normal.roll)
        yaw = self.normalize(hand.direction.yaw)
        print(pitch, roll, yaw)

        return 0, 0, 0, 0


    def normalize(self, radians):
        return (radians + math.pi) / (2 * math.pi)