import Leap

class MotionCapturer:

    def __init__(self):
        self.controller = Leap.Controller()


    def get_axis_values(self):
        frame = self.controller.frame()
        if frame.hands
