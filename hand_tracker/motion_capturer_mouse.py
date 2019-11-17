from motion_capturer import MotionCapturer

import logitech_controller.logidevmon as logidevmon
import json
from threading import Thread

def registerEvent(event, capturer):
    capturer.processEvents(event)
    return True

def run_loop(capturer):
    logidevmon.read_events(registerEvent, capturer)

class MotionCapturerMouse(MotionCapturer):
    def __init__(self):
        self.currentThrottle = 0
        self.currentX = 50
        self.currentY = 50
        self.currentJaw = 50
        self.mouseId = 0
        self.captureThread = None
        self.noShutdown = True
        self.captureMouse()

    def captureMouse(self):
        logidevmon.list_devices()
        for device in logidevmon.LOGITECH_DEVICES:
            print (f"{device['unitId']} {device['type']} : {device['name']}")

            if (device["type"] == "mouse"):
                self.mouseId = device['unitId']

        logidevmon.set_thumbWheel_config(self.mouseId,False,False,False)
        logidevmon.set_wheel_config(self.mouseId,False,False,False)
        logidevmon.set_thumbWheel_config(self.mouseId, False, False, False)

        if (len(logidevmon.LOGITECH_DEVICES)>0 and self.mouseId != 0):
            logidevmon.set_wheel_config(self.mouseId,True,True,True)
            logidevmon.set_thumbWheel_config(self.mouseId, True, True, True)
            logidevmon.set_immediate_pointer_redirection_config(self.mouseId, True, True, True)

            self.captureThread = Thread(target=run_loop, args=(self, ))
            self.captureThread.start()

    def get_axis_values(self):
        res = (float(self.currentY) / 100, float(self.currentX) / 100, float(self.currentJaw) / 100, float(self.currentThrottle) / 100)

        # Slow down rotations
        distance_to_middle = 50 - self.currentY
        step = float(distance_to_middle) / 50
        self.currentY = self.currentY + step
        distance_to_middle = 50 - self.currentX
        step = float(distance_to_middle) / 50
        self.currentX = self.currentX + step

        distance_to_middle = 50 - self.currentJaw
        step = float(distance_to_middle) / 50
        self.currentJaw = self.currentJaw + step

        return res

    def shutdown(self):
        logidevmon.set_immediate_pointer_redirection_config(self.mouseId, False, False, False)
        logidevmon.set_wheel_config(self.mouseId,False,False,False)
        logidevmon.set_thumbWheel_config(self.mouseId, False, False, False)
        self.noShutdown = False

    def normalize(self, n):
        if n > 100:
            n = 100
        elif n < 0:
            n = 0
        return n

    def processWheel(self, event):
        delta = int(event["value"]["delta"])
        if delta < 0:
            delta = delta * 0.4
        else:
            delta = delta * 0.6
        self.currentThrottle = self.normalize(self.currentThrottle + delta)

    def interpretMouse(self, value, mult):
        if(value == 0):
            return 0
        elif(value > 1000):
            return 3 * mult
        else:
            return -3 * mult

    def processXYRaw(self, event):
        self.currentX = self.normalize(self.currentX + self.interpretMouse(event["value"]["dx"], -1))
        self.currentY = self.normalize(self.currentY + self.interpretMouse(event["value"]["dy"], 1))

    def processJaw(self, event):
        delta = int(event["value"]["rotation"])
        delta = delta * 15
        self.currentJaw = self.normalize(self.currentJaw + delta)

    def processEvents(self, message):
        res = json.loads(message)

        if (res["path"] == "wheel"):
            self.processWheel(res)
        elif(res["path"] == "thumbWheel"):
            self.processJaw(res)
        elif (res["path"] == "divertedRawXY"):
            self.processXYRaw(res)

        return self.noShutdown