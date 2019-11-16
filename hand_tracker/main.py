import os, sys, inspect
src_dir = os.path.dirname(inspect.getfile(inspect.currentframe()))
lib_dir = os.path.abspath(os.path.join(src_dir, './lib'))
sys.path.insert(0, lib_dir)

from motion_capturer import MotionCapturer

def main():

    motion_capturer = MotionCapturer()
    transmitter = Transmitter(sys.argv[1])

    while True:
        try:
            (pitch, roll, yaw, throttle) = motion_capturer.get_axis_values()
            transmitter.send_axis_values(pitch, roll, yaw, throttle)
        except ValueError:
            print('Ignoring frame')



