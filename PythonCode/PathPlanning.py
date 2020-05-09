import numpy as np
import matplotlib.pyplot as plt
import time

def plan(theta, w, a, dt = .05, r = 14):
    t_ramp = w/a
    theta_min = w**2/a
    theta_rect = theta - theta_min
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
    return control, t

def run_plan(controller,control, direction = "right", dt=.05):
    for v in control:
        if direction == "right":
            controller.send_speed(v, -v)
        else if direction == "left":
            controller.send_speed(-v, v)
        time.sleep()
    
control, t = plan(-np.pi/4, np.pi/16, np.pi/16)
plt.plot(t, control)
plt.show()
    
