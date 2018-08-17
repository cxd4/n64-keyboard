#ifndef _ANALOG_H_
#define _ANALOG_H_

/*
 * for automatic, rapid control stick revolutions
 *
 * A precision of 4 means swap the stick between 0, 90, 180, and 270 degrees.
 * A precision of 8 means 0, 45, 90, 135, 180, 225, 270, then 315 degrees.
 *
 * A precision greater than 8 should never be necessary.
 * If that was necessary, using keyboard keys to rotate the control stick
 * would have been undetected by the game for such a precision requirement.
 *
 * Negative values should, in theory, yield the same results of spins done on
 * the absolute value of the negative precision clockwise instead of counter-
 * clockwise, but this has not been tested to work as well as CCW spins.
 *
 * Results from the Mecha Fly-Guy stick test in Mario Party:
 *     f(60) =  10 spins / 10 seconds (barely enough to draw Paddle Battle)
 *     f(24) =  25 spins / 10 seconds (almost enough to win Power Pedal)
 *     f(12) =  50 spins / 10 seconds
 *     f(10) =  60 spins / 10 seconds
 *     f(9)  =  66 spins / 10 seconds
 *     f(8)  =  75 spins / 10 seconds (inconsistent with Zelda instant spin)
 *     f(7)  =  43 spins / 10 seconds
 *     f(6)  = 100 spins / 10 seconds (too imprecise to work with Pedal Power)
 */
extern long arcs_per_second;

#define MAX_CONTROLLERS                 (8 / 2)

extern signed char clamp_stick(signed long magnitude);
extern int stick_range(void);
extern void stick_rotate(signed char * x, signed char * y, float degrees);

#endif
