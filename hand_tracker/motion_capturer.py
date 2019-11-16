from abc import abstractmethod

class MotionCapturer():

    @abstractmethod
    def get_axis_values(self):
        pass
        