#include <assert.h>
#include <malloc.h>

#include <ctype.h>
#include <string.h>
#include "buttons.h"

pu16 press_masks;

void map_keys(void)
{
    const size_t possible_characters = 128; /* assumes ASCII keyboard input */

    press_masks = (pu16)malloc(possible_characters * sizeof(u16));
    assert(press_masks != NULL);
    memset(press_masks, MASK_NO_BUTTONS, possible_characters * sizeof(u16));

    press_masks['\'']= MASK_C_RIGHT;
    press_masks['L'] = MASK_C_LEFT;
    press_masks[';'] = MASK_C_DOWN;
    press_masks['P'] = MASK_C_UP;
    press_masks['I'] = MASK_R_TRIG;
    press_masks['U'] = MASK_L_TRIG;

    press_masks[KEYBOARD_SHIFT]   = MASK_reserved0;
    press_masks[KEYBOARD_CONTROL] = MASK_reserved1;

    press_masks[KEYBOARD_RIGHT] = MASK_R_JPAD;
    press_masks[KEYBOARD_LEFT ] = MASK_L_JPAD;
    press_masks[KEYBOARD_DOWN ] = MASK_D_JPAD;
    press_masks[KEYBOARD_UP   ] = MASK_U_JPAD;

    press_masks['E'] = MASK_START_BUTTON;
    press_masks['O'] = MASK_Z_TRIG;
    press_masks['K'] = MASK_B_BUTTON;
    press_masks['J'] = MASK_A_BUTTON;

    press_masks['W'] = MASK_STICK_UP;
    press_masks['A'] = MASK_STICK_LEFT;
    press_masks['S'] = MASK_STICK_DOWN;
    press_masks['D'] = MASK_STICK_RIGHT;
    return;
}

NOINLINE size_t filter_OS_key_code(size_t signal)
{
#if defined(_WIN32) || defined(_WIN64)
    switch (signal) {
    case 0x25:  return KEYBOARD_LEFT; /* from VK_LEFT */
    case 0x26:  return KEYBOARD_UP; /* from VK_UP */
    case 0x27:  return KEYBOARD_RIGHT; /* from VK_RIGHT */
    case 0x28:  return KEYBOARD_DOWN; /* from VK_DOWN */

    case 0x10:  return KEYBOARD_SHIFT; /* from VK_SHIFT */
    case 0x11:  return KEYBOARD_CONTROL; /* from VK_CONTROL */

    case 0xBA:  return ';'; /* from VK_OEM_1 (`;:`) */
    case 0xDE:  return '\''; /* from VK_OEM_7 (`':`) */
    }
#else
    if (signal >= 'a' && signal <= 'z')
        signal  = toupper(signal); /* SDL keys register lowercase letters. */

    switch (signal & 0xFFFF) { /* SDL 2.0 might mask in bit 30. */
    case 0x01:  case 0x02:
    case 0x012F:  case 0x0130: /* SDL 1.2 right and left Shift */
        return KEYBOARD_SHIFT;
    case 0x40:  case 0x80:
    case 0x0131:  case 0x0132: /* SDL 1.2 right and left Ctrl */
        return KEYBOARD_CONTROL;

    case 273: /* SDLK_UP in SDL 1.x */
        return KEYBOARD_UP;
    case 274: /* SDLK_DOWN in SDL 1.x */
        return KEYBOARD_DOWN;
    case 275: /* SDLK_RIGHT in SDL 1.x */
        return KEYBOARD_RIGHT;
    case 276: /* SDLK_LEFT in SDL 1.x */
        return KEYBOARD_LEFT;
    }
#endif
    return (signal);
}
