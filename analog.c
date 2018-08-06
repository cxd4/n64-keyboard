#include <math.h>
#include "analog.h"

#include "contr.h"
extern BUTTONS controllers[];

long arcs_per_second = 10;

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

void stick_rotate(signed char * x, signed char * y, double degrees)
{
    double arc;
    double x1, y1;
    double x2, y2;
    const int radius = stick_range();
    const double pi = 3.141592653589793;
    const double arc_interval = pi * (degrees / 180.);

    x1 = *(x);
    y1 = *(y);

    arc = asin(y1 / radius) + arc_interval;
    if (x1 <= 0 && y1 > 0)
        arc += pi / 2; /* Translate from Q I to Q II. */
    if (y1 <= 0 && x1 < 0)
        arc -= pi / 2; /* Translate from Q IV to Q III. */

    x2 = cos(arc);
    y2 = sin(arc);

    *(x) = (signed char)(x2 * radius);
    *(y) = (signed char)(y2 * radius);
    return;
}
