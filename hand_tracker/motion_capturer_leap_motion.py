import Leap
import math
from motion_capturer import MotionCapturer

class MotionCapturerLeapMotion(MotionCapturer):

    def __init__(self):
        self.controller = Leap.Controller()


    def get_axis_values(self):
        frame = self.controller.frame()
        if len(frame.hands) == 0:
            print("No hand detected")
            return 0.5, 0.5, 0.5, 0
        elif len(frame.hands) > 1:
            print("More than one hand detected")
            raise ValueError

        hand = frame.hands[0]
        pitch = 1 - self.normalize(hand.direction.pitch)
        roll = 1 - self.normalize(hand.palm_normal.roll)
        yaw = (self.normalize(hand.direction.yaw) - 0.5) * 0.1 + 0.5
        throttle = min(1, hand.palm_position[1] / 300)
        print(pitch, roll, yaw, throttle)
        return pitch, roll, yaw, throttle


    def normalize(self, radians):
        return (radians + math.pi) / (2 * math.pi)
