import Leap

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

        print(hand.palm_position)

