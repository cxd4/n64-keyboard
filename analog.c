#include <math.h>
#include <signal.h>
#include "analog.h"

#include "contr.h"
extern BUTTONS controllers[];

long arcs_per_second = 10;
static double pi(void)
{
    static double pi_final;
    double old_approximation, base;

    if (pi_final > 3.14 && pi_final < 3.15)
        return (pi_final);
    base = 0;
    pi_final = 2;
    do {
        old_approximation = pi_final;
        base = sqrt(base + 2);
        pi_final = (pi_final * 2) / base;
    } while (pi_final != old_approximation);
    return (pi_final);
}

signed char clamp_stick(signed long magnitude)
{
    if (magnitude < -128)
        return -128;
    if (magnitude > +127)
        return +127;
    return ((signed char)magnitude);
}

int stick_range(void)
{
    static const u8 magnitudes[] = {
         80, /* hardware limitation:  Strongest real N64 stick presses do 80. */
        128, /* software limitation (Hold Shift.):  stick_mask & 0xFF >= -128 */
         64, /* a little slow (Hold Ctrl.) */
         32, /* very slow (Hold Ctrl+Shift.) */
    };
    const int shift_amount = (ENDIAN_M ? 6 + 8 : 6 + 0);

    return (int)magnitudes[(controllers[0].Value >> shift_amount) & 3];
}

void stick_rotate(signed char * x, signed char * y, float degrees)
{
    double arc;
    float x1, y1;
    double x2, y2;

    const int radius = stick_range();
    const double arc_interval = pi() * (degrees / 180);
    const double ninety_degrees = pi() / (degrees < 0 ? -2 : +2); /* CW : CCW */

    x1 = *(x);
    y1 = *(y);

#define PREFER_ARCCOS_OVER_ARCSIN
#ifdef PREFER_ARCCOS_OVER_ARCSIN
    arc = acos(x1 / radius) + arc_interval;
#else
    arc = asin(y1 / radius) + arc_interval;
#endif
    switch (quadrant(x1, y1)) {
    case 1:
        break;
    case 2: /* arcsin() returns angles in QI and QIV, discarding -x. */
        arc += ninety_degrees; /* Rotate from QI to QII, preserving +y. */
        break;
    case 3: /* arcsin() returns angles in QI and QIV, discarding -x. */
#ifdef PREFER_ARCCOS_OVER_ARCSIN
        arc += ninety_degrees; /* Rotate from QII to QIII, preserving -x. */
#else
        arc -= ninety_degrees; /* Rotate from QIV to QIII, preserving -y. */
#endif
        break;
    case 4: /* arccos() returns angles in QI and QII, discarding -y. */
#ifdef PREFER_ARCCOS_OVER_ARCSIN
        arc -= ninety_degrees; /* Rotate from QI to QIV, preserving +x. */
#endif
        break;
    }

    x2 = cos(arc);
    y2 = sin(arc);

    *(x) = (signed char)(x2 * radius);
    *(y) = (signed char)(y2 * radius);
    return;
}

int quadrant(signed long x, signed long y)
{
    if (x == 0 && y == 0)
        return 0; /* no angle, no slope...no quadrant */

    if (x > 0 && y >= 0)
        return 1; /* Quadrant I:  0 to 89.999... degrees */
    if (x <= 0 && y > 0)
        return 2; /* Quadrant II:  90 to 179.999... degrees */
    if (y <= 0 && x < 0)
        return 3; /* Quadrant III:  180 to 269.999... degrees */
    if (y < 0 && x >= 0)
        return 4; /* Quadrant IV:  270 to 359.999... or -0.001 to -90 degrees */

    raise(SIGFPE);
    return -1;
}
