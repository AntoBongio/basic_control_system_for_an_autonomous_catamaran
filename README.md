# Basic control system for an autonomous catamaran

> Goal: Implementing a basic control system for an autonomous catamaran.

A microcontroller board is connected to two outboard motors. Each outboard motor is composed by a DC motor and a propeller installed 
at the end of its shaft. Together, the two outboard motors allow the catamaran to move and rotate in the water. The microcontroller receives 
desired reference valuesfor the rotation speed of the motors from a control PC, in terms of motor RPMs (rounds per minute). These 
reference signals are sent through a serial interface. The microcontroller sends a feedback message back to the control PC to report a few 
status information.
