import numpy as np
import matplotlib.pyplot as plt
import time
from .DriveControlInterop import DriveController

# TODO make all this usefully packaged 
def plan_turn(theta, w, a, direction, dt = .1, r = 14):
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

    if direction=="right":
        r_control = -control
        l_control = control
    elif direction=="left":
        r_control = control
        l_control = -control
    return l_control, r_control, t

# TODO ahh dammit v vs w. fix it.
def plan_straight(dist, v, a, direction, dt=.1):
    t_ramp = v/a
    dist_min = v**2/a
    dist_rect = dist - dist_min
    if dist_rect > 0:
        t_rect = dist_rect/v
        t_total = 2*t_ramp + t_rect
        
        ramp_up = a * np.linspace(0, t_ramp, int(t_ramp/dt + 1))
        ramp_down = v - (a * np.linspace(0, t_ramp, int(t_ramp/dt + 1)))
        rect = np.full((int((t_rect/dt)+1)), v)
        
        control = np.concatenate([ramp_up[:-1], rect[:-1], ramp_down])
        t = []
        curr_t = 0
        for _ in control:
            t.append(curr_t)
            curr_t +=dt
    else:
        t_ramp = np.sqrt(dist/a)
        peak_v = a*t_ramp
        ramp_up = a * np.linspace(0, t_ramp, int(t_ramp/dt + 1))
        ramp_down =  peak_v - (a * np.linspace(0, t_ramp, int(t_ramp/dt + 1)))

        control = np.concatenate([ramp_up[:-1], ramp_down])
        t = []
        curr_t = 0
        for _ in control:
            t.append(curr_t)
            curr_t +=dt

    if direction == "fwd":
        l_control = control
        r_control = control

    if direction == "bwd":
        l_control = -control
        r_control = -control
    return l_control, r_control, t

def run_plan(controller, l_control, r_control, dt=.1):
    for l,r in zip(l_control, r_control):
        #print(controller.read_speeds(0),controller.read_speeds(1))
        controller.send_speed(l, r)
        time.sleep(dt)


if __name__ == "__main__":
    controller = DriveController("/dev/ttyACM0")
    control, t = plan_turn(np.pi/8, np.pi/2, 2)

    for i in range(8):
        run_plan(controller, control)
        time.sleep(1)
