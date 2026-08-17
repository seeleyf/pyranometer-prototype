Pyranometer Self-Cleaning Prototype
A hardware/software prototype developed during an internship to automate the cleaning of pyranometer domes at electrical substations. Pyranometers measure solar irradiance, and dome contamination directly impacts measurement accuracy. Manual cleaning by technicians is currently required — this prototype aims to automate that process.
How it works
A 3D-printed arm positions a water pump and blower fan over the dome. A button press triggers the full cycle: the arm rotates into position, pulses water onto the dome, soaks, runs the fan to dry the surface, then returns home. LEDs indicate system status throughout.
Files
pyranometer_basic.ino — core cycle, no error handling
pyranometer_error.ino — same cycle with pin-disconnect detection and Serial Monitor error reporting
Hardware
Arduino Uno, MG996R servo, 2-channel relay, LM2596 buck converter, 12V diaphragm pump, 5015 blower fan, 4× status LEDs, momentary push button.
Status: In progress — mechanical and firmware improvements ongoing.
