#################################################################
#
# srv02_control.py
#
# Adapted from:
# https://github.com/quanser/quanser_sdk_examples/blob/main/python/hardware/qube_servo2_usb_control_example/qube_servo2_usb_control_example.py
#
# State-feedback control with derivative filter (Quanser SRV02 + Q2-USB)
#
#################################################################
import quanser.hardware
from quanser.hardware import HIL, HILError, Clock
import math
import signal
import sys
import array as arr
import numpy as np
import time
import csv

# --- System constants (from datasheet) ---
# All comments are in English by request.
m_p = 0.127        # Mass of pendulum [kg]
L_p = 0.337        # Total length of pendulum [m]
l_p = 0.156        # Distance from pivot to center of mass [m]
J_p = 0.0012       # Pendulum moment of inertia about center of mass [kg*m^2]
m_r = 0.257        # Mass of rotary arm with two thumb screws [kg]
r = 0.216          # Rotary arm length from pivot to tip [m]
l_r = 0.0619       # Rotary arm length from pivot to center of mass [m]
K_enc = 4096       # Pendulum encoder resolution [counts/rev]
g = 9.81           # Gravitational acceleration [m/s^2]
Er = 0.42          # Potential energy at the downward position [J]

# --- Drivetrain and motor constants (from datasheet) ---
R_m  = 2.6         # Motor armature resistance [Ohm]         
K_g  = 70.0        # Gear ratio [unitless]                    
eta_g = 0.9        # Gear efficiency [unitless]              
eta_m = 0.69       # Motor efficiency [unitless]             
k_t  = 0.00768     # Motor torque constant [N*m/A]            
k_m  = 0.00768     # Back-EMF constant [V*s/rad]              
L_r = r            # Alias to match formula notation L_r

mu = 4             # Slider gain
u_max = 9          # Maximum acceleration [m/s^2]

stop = False
log_data = []      # Data logging: (timestamp_us, theta, alpha, Vm)

def signal_handler(signum, frame):
    global stop
    stop = True

# Derivative filter (Quanser)
def ddt_filter(u, state, A, Ts):
    y = 1/(A*Ts+2)*(2*A*u - 2*A*state[0] - state[1]*(A*Ts - 2))
    state[0] = u
    state[1] = y
    return y, state

# Register a Ctrl+C handler
signal.signal(signal.SIGINT, signal_handler)

board_type = "q2_usb"
board_identifier = "0"

try:
    card = HIL()
    card.open(board_type, board_identifier)

    try:
        samples           = HIL.INFINITE
        frequency         = 400
        period            = 1 / frequency
        samples_in_buffer = math.ceil(0.1 * frequency)

        analog_channels   = arr.array("i", [0])
        encoder_channels  = arr.array("i", [0, 1])  # motor (theta), pendulum (alpha)

        num_analog_channels  = len(analog_channels)
        num_encoder_channels = len(encoder_channels)

        voltages = arr.array("d", [0.0] * num_analog_channels)
        counts   = arr.array("i", [0] * num_encoder_channels)

        print("Running SRV02 state-feedback control at %g Hz" % frequency)
        print("Press CTRL-C to stop.\n")

        # Make sure the motor voltage is zero
        card.write_analog(analog_channels, num_analog_channels, voltages)
        
        # Reset the encoder counts to zero (optional)
        card.set_encoder_counts(encoder_channels, num_encoder_channels, arr.array("i", [0,0]))

        # Create encoder task
        task = card.task_create_encoder_reader(samples_in_buffer, encoder_channels, num_encoder_channels)
        card.task_start(task, Clock.HARDWARE_CLOCK_0, frequency, samples)

        # --- Controller setup ---
        K = np.array([-0.3121149, 12.201727, -1.2822833, 1.65234272])

        # Derivative filter states: [u_k-1, y_k-1]
        state_theta_dot = np.array([0.0, 0.0], dtype=np.float64)
        state_alpha_dot = np.array([0.0, 0.0], dtype=np.float64)

        # Reference command
        command_deg = 0

        samples_read = card.task_read_encoder(task, 1, counts)
        
        while samples_read > 0 and not stop:

            # --- Read sensors ---
            theta = counts[0] * 2 * np.pi / K_enc       # [rad]
            alpha_f = counts[1] * 2 * np.pi / K_enc     # Full raw pendulum angle [rad]
            alpha = (alpha_f % (2*np.pi)) - np.pi       # Inverted pendulum angle [rad]

            # --- Compute derivatives with filter (A=50 rad/s) ---
            theta_dot, state_theta_dot = ddt_filter(theta, state_theta_dot, 50, period)
            alpha_dot, state_alpha_dot = ddt_filter(alpha, state_alpha_dot, 50, period)

            # --- State vector ---
            states = command_deg * np.array([np.pi/180, 0, 0, 0]) - np.array([theta, alpha, theta_dot, alpha_dot])

            # --- Swing-up / Balance control ---
            safety_limit = 20 * math.pi / 180
            if abs(alpha) > safety_limit:
                # Swing-up controller: energy-based control
                E = (0.5 * J_p * (alpha_dot ** 2)) + (0.5 * m_p * g * L_p * (1 - np.cos(alpha_f)))
                u = np.clip(mu * (E - Er) * np.sign(alpha_dot * np.cos(alpha_f)), -u_max, u_max)
                tau = m_r * L_r * u
                Vm = (tau * R_m) / (eta_g * K_g * eta_m * k_t) + (K_g * k_m * theta_dot)
                Vm = -Vm
            else:
                # Balance controller: LQR state feedback
                Vm = -1.0 * np.dot(K, states)

            # --- Log data (timestamp in µs, theta, alpha, Vm) ---
            timestamp_us = time.time_ns() // 1000
            log_data.append((timestamp_us, theta, alpha, Vm))
            
            print("theta: %7.4f rad   alpha: %7.4f rad   Voltage: %7.4f" % (theta, alpha, Vm))

            # --- Send control to motor (clipped) ---
            card.write_analog(analog_channels, num_analog_channels, np.array([np.clip(Vm, -5, 5)], dtype=np.float64))

            # Next sample
            samples_read = card.task_read_encoder(task, 1, counts)

        # Stop tasks
        card.task_stop(task)
        card.task_delete(task)

        # Motor off
        voltages[0] = 0.0
        card.write_analog(analog_channels, num_analog_channels, voltages)

    except HILError as ex:
        print("Unable to run task. %s" % ex.get_error_message())

    finally:
        card.close()

except HILError as ex:
    print("Unable to open board. %s" % ex.get_error_message())

# --- Save to CSV ---
with open("encoder_log.csv", "w", newline="") as f:
    writer = csv.writer(f)
    writer.writerow(["timestamp_us", "theta_rad", "alpha_rad", "voltage"])
    writer.writerows(log_data)

print("Program finished. Data saved to encoder_log.csv")
exit(0)
