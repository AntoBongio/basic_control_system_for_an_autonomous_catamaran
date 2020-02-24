// This is a guard condition so that contents of this file are not included
// more than once.  
#ifndef PWM_HEADER_TEMPLATE_H
#define	PWM_HEADER_TEMPLATE_H

#include <xc.h> // include processor files - each processor file is guarded.

#define SOURCE_OSCILLATOR_FREQUENCY  7372800

void init_PWM(float fpwm, int duty[]);

int get_pwm_prescaler(double fpwm);

void update_pwm(int duty[]);

int round(double x);

#endif	/* PWM_HEADER_TEMPLATE_H */
