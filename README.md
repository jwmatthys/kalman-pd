kalman-pd
=========

Simple control rate Kalman filter for Pure Data

The full Kalman algorithm is a complex, powerful, and flexible estimator
suitable for a wide variety of purposes. But a simplified version is adequate
for most purposes, and reduces computational intensity.

Analysis mode is toggled with [analyze 1( and [analyze 0(, which performs
statistical analysis on the incoming data to determine mean and standard
deviation. These are then used as initial values for the Kalman algorithm,
generally producing more accurate results in fewer iterations.

Based on http://bilgin.esme.org/BitsBytes/KalmanFilterforDummies.aspx
