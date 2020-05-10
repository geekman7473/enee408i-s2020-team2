import numpy as np
import matplotlib.pyplot as plt
import time
from .DriveControlInterop import DriveController

def plan_turn(theta, w, a, dt = .1, r = 14):
    t_ramp = w/a
    theta_min = w**2/a
    theta_rect = theta - theta_min
    if theta_rect > 0:
        t_rect = theta_rect/w
        t_total = 2*t_ramp + t_rect
        
        ramp_up = a * np.linspace(0, t_ramp, int(t_ramp/dt + 1))
        ramp_down = w - (a * np.linspace(0, t_ramp, int(t_ramp/dt + 1)))
        rect = np.full((int((t_rect/dt)+1)), w)
        
        control_w = np.concatenate([ramp_up, rect, ramp_down])
        t = []
        curr_t = 0
        for _ in control_w:
            t.append(curr_t)
            curr_t +=dt

        control = control_w * r
    else:
        t_ramp = np.sqrt(theta/a)
        peak_w = a*t_ramp
        ramp_up = a * np.linspace(0, t_ramp, int(t_ramp/dt + 1))
        ramp_down =  peak_w - (a * np.linspace(0, t_ramp, int(t_ramp/dt + 1)))

        control_w = np.concatenate([ramp_up, ramp_down])
        t = []
        curr_t = 0
        for _ in control_w:
            t.append(curr_t)
            curr_t +=dt

        control = control_w * r
        
    return control, t

def run_plan(controller,control, direction = "right", dt=.1):
    for v in control:
        print(controller.read_speeds(0),controller.read_speeds(1))
        if direction == "right":
            controller.send_speed(v, -v)
        elif direction == "left":
            controller.send_speed(-v, v)
        time.sleep(dt)


controller = DriveController("/dev/ttyACM0")
control, t = plan_turn(np.pi/8, np.pi/2, 2)

for i in range(8):
    run_plan(controller, control)
    time.sleep(1)
