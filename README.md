# dyno
Arduino project to refit Go Systems 100HP Dynanometer

The dyno has an Interface Systems 250Kg load cell which measures the torque. We are using an HX711 24-bit A/D load cell amplifier to read the values from this sensor.

The dyno HAD a 60-tooth metal gear which was being used as the RPM sensor in conjunction with a magnetic reluctance sensor. This sensor had failed and replacements were prohibitively expensive, so this has been switched to using a 4-magnet rotor-disc and hall sensor.

The replacement electronics is based around an Arduino Nano with onboard CH340C USB->Serial converter, allowing for connection using a standard USB-C cable. All input/output from the system is via the USB serial port.

## Principles of operation
The system code runs in a continuous loop:

1.) The system counts the number of pulses from the RPM sensor for 1/2 second.
2.) Take 10 samples from the load cell and average them to get a more accurate torque reading.
3.) Print the timestamped results via the serial port in a comma-separated format for easy importing to excel.
4.) Check for any serial-input from the user (to support TARE and CALIBRATE functions for the load cell), and process accordingly.

### Reading the RPM
A 4-magnet rotor disc is attached to the back of the dynanometer and is read using a hall effect sensor. The RPM function makes use of the two internal counter-timers within the Ardunio. One times is used to meaure a set period, and the second is used a a pulse-counter to sum the number of times the hall sensor has been triggered. We are measuing RPM in 1/2 second intervals, which gives us a minimum resolution of 30RPM (i.e. if we see 2 magnets passing the hall sensor in 1/2 second sample window indicates 60RPM)

## Usage

The Dynanometer outputs serial data at 115200bps (8N1) whenever the RPM or Torque is non-zero.

```
0:02:27, RPM: 0, lb-ft: 0.0, HP: 0.00
0:02:28, RPM: 30, lb-ft: 20.78, HP: 0.12
0:02:29, RPM: 90, lb-ft: 14.81, HP: 0.25
0:02:29, RPM: 180, lb-ft: 17.35, HP: 0.59
```

Enabling serial logging in your terminal program will allow this data to be captured and later imported as CSV values into a spreadhseet.

The serial input is checked every cycle for input from the user. Currently there are two supported inputs.

'z' - Zero/Tare the load cell. 
'c' - Calibrate the load cell with a 49.5lb calibration weight.

Calibration settings are stored in EEPROM and will persist across power-cycles.

