from motion_capturer import MotionCapturer
from motion_capturer_leap_motion import MotionCapturerLeapMotion
from motion_capturer_mouse import MotionCapturerMouse

class MotionCapturerLeapAndMouse(MotionCapturer):

    def __init__(self):
        self.leap_motion = MotionCapturerLeapMotion
        self.mouse = MotionCapturerMouse


    def get_axis_values(self):
        pitch, roll, _, throttle = self.leap_motion.get_axis_values()
        _, yaw, _, _ = self.mouse.get_axis_values()
        return pitch, roll, yaw, throttle
