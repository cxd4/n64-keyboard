#ifndef _BUTTONS_H_
#define _BUTTONS_H_

#define MASK_NO_BUTTONS         0x00000000

#define MASK_C_RIGHT            (1u <<  0)
#define MASK_C_LEFT             (1u <<  1)
#define MASK_C_DOWN             (1u <<  2)
#define MASK_C_UP               (1u <<  3)
#define MASK_R_TRIG             (1u <<  4)
#define MASK_L_TRIG             (1u <<  5)
#define MASK_reserved0          (1u <<  6)
#define MASK_reserved1          (1u <<  7)
#define MASK_R_JPAD             (1u <<  8)
#define MASK_L_JPAD             (1u <<  9)
#define MASK_D_JPAD             (1u << 10)
#define MASK_U_JPAD             (1u << 11)
#define MASK_START_BUTTON       (1u << 12)
#define MASK_Z_TRIG             (1u << 13)
#define MASK_B_BUTTON           (1u << 14)
#define MASK_A_BUTTON           (1u << 15)

/*
 * real N64 system behavior (documented in the product hazards manual):
 * Pressing START+L+R resets current stick state as the new neutral position.
 */
#define CONTROL_STICK_EXCEPTION (MASK_START_BUTTON | MASK_L_TRIG | MASK_R_TRIG)

#define MASK_STICK_RIGHT        (0xF0F0u | MASK_R_JPAD | MASK_C_RIGHT)
#define MASK_STICK_LEFT         (0xF0F0u | MASK_L_JPAD | MASK_C_LEFT)
#define MASK_STICK_DOWN         (0xF0F0u | MASK_D_JPAD | MASK_C_DOWN)
#define MASK_STICK_UP           (0xF0F0u | MASK_U_JPAD | MASK_C_UP)

/*
 * We need this to keep track of which control stick directions are currently
 * being used by the keyboard, as it is possible to press the control stick
 * in both opposite directions at the same time when emulated via keyboard.
 */
typedef struct {
    int up;
    int down;
    int right;
    int left;
} control_stick_activity;

/*
 * The four directional arrow keys on the keyboard have no character
 * representation, at least under ASCII, so we will use the ASCII characters:
 *     up   :  SOH (start of heading)
 *     left :  STX (start of text)
 *     right:  ETX (end of text)
 *     down :  EOT (end of transmission)
 *
 * Little else can be done since the other ASCII code points are used up.
 * The only other vacant group of four code points seems to be ASCII+28 (FS).
 */
#if 0
#define DIRECTIONAL_BASE        '\28'
#else
#define DIRECTIONAL_BASE        '\1'
#endif

#define KEYBOARD_UP             (DIRECTIONAL_BASE + 0)
#define KEYBOARD_LEFT           (DIRECTIONAL_BASE + 1)
#define KEYBOARD_RIGHT          (DIRECTIONAL_BASE + 2)
#define KEYBOARD_DOWN           (DIRECTIONAL_BASE + 3)

#include <stddef.h>
#include "my_types.h"

/*
 * assuming an ASCII implementation, should be 128 * 2 bytes per u16
 * Probably this is more maintainable and readable if allocated using malloc.
 */
extern pu16 press_masks;

extern void map_keys(void);
extern NOINLINE size_t filter_OS_key_code(size_t signal);

#endif
