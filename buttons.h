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

#include "contr.h"

/*
 * assuming an ASCII implementation, should be 128 * 2 bytes per u16
 * Probably this is more maintainable and readable if allocated using malloc.
 */
extern pu16 press_masks;

#endif