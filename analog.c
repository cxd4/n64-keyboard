#include <math.h>
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

    x1 = *(x);
    y1 = *(y);

    arc = atan2(y1 / radius, x1 / radius) + arc_interval;

    x2 = cos(arc);
    y2 = sin(arc);

    *(x) = (signed char)(x2 * radius);
    *(y) = (signed char)(y2 * radius);
    return;
}
