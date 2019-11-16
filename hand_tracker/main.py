#!/usr/bin/env python

import os, sys, inspect
src_dir = os.path.dirname(inspect.getfile(inspect.currentframe()))
lib_dir = os.path.abspath(os.path.join(src_dir, './lib'))
sys.path.insert(0, lib_dir)

from motion_capturer_leap_motion import MotionCapturerLeapMotion
from motion_capturer_mouse import MotionCapturerMouse
from transmitter import Transmitter

def main():

    if sys.argv[1] == 'leap_motion':
        motion_capturer = MotionCapturerLeapMotion()
    elif sys.argv[1] == 'mouse':
        motion_capturer = MotionCapturerMouse()
    elif sys.argv[1] == 'mix':
        motion_capturer = MotionCapturerLeapAndMouse()

    transmitter = Transmitter(sys.argv[2])

    while True:
        try:
            (pitch, roll, yaw, throttle) = motion_capturer.get_axis_values()
            transmitter.send_axis_values(pitch, roll, yaw, throttle)
        except ValueError:
            print('Ignoring frame')


if __name__ == '__main__':
    main()