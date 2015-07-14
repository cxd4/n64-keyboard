#include <assert.h>
#include <malloc.h>
#include <stdlib.h>
#include <string.h>

/* to do:  have working controller #1.1 implementation */
#define SPECS_VERSION           0x0100
#include "contr.h"
#include "buttons.h"

/* to do:  Wasn't there some old design when this could be 8? */
#define MAX_CONTROLLERS         4
static BUTTONS controllers[MAX_CONTROLLERS];

static const int swapped_bytes =
#if (ENDIAN_M != 0)
    1
#else
    0
#endif
;

pu16 press_masks;
static control_stick_activity already_pressed;

static INLINE u16 swap16by8(u16 word)
{
    u16 swapped;

/*
 * MSVC 2005 (CL.EXE /O1 /Os):  ROL     ax, 8
 * GCC 4.8.1 (gcc -Os -ansi) :  XCHG    al, ah
 *
 * Apparently even size-only optimizing compilers are capable of optimizing
 * this function down to a single instruction, so there is no function here.
 */
    swapped = 0x0000
      | ((word & 0x00FFu) << 8)
      | ((word & 0xFF00u) >> 8)
    ;
    return (swapped &= 0xFFFFu);
}

EXPORT void CALL GetDllInfo(PLUGIN_INFO * PluginInfo)
{
    char * name;
    u16 * system_version, * plugin_type;
    int/* * memory_normal,*/ * memory_swapped;

    system_version = &(PluginInfo -> Version);
    plugin_type    = &(PluginInfo -> Type);
    name           = &(PluginInfo -> Name[0]);
 /* memory_normal  = &(PluginInfo -> Reserved1); // could be a bug in 1.4 */
    memory_swapped = &(PluginInfo -> Reserved2); /* bug in PJ 1.4; needs TRUE */

    *(system_version) = SPECS_VERSION;
    *(plugin_type)    = PLUGIN_TYPE_CONTROLLER;
 /* *(memory_normal)  = 0; // reserved, shouldn't be set ideally */
    *(memory_swapped) = swapped_bytes;

    strcpy(name, "System Keyboard");
    return;
}

EXPORT void CALL CloseDLL(void)
{
    RomClosed();
    return;
}

EXPORT void CALL RomClosed(void)
{
    free(press_masks);
    return;
}

EXPORT void CALL RomOpen(void)
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

 /* to do:  possibly implement the 2 reserved button flags as accelerators? */

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

EXPORT void CALL GetKeys(int Control, BUTTONS * Keys)
{
    u16 buttons;

    assert(Control < MAX_CONTROLLERS);
    assert(Keys != NULL);

    buttons = controllers[Control].Value & 0x0000FFFFul;
    if ((buttons & CONTROL_STICK_EXCEPTION) == CONTROL_STICK_EXCEPTION)
    { /* controller exception:  START while holding L + R */
        controllers[Control].Value &= 0x0000FFFFul; /* analog stick reset */
        Keys -> Value = buttons & ~MASK_START_BUTTON;
        return;
    }
    Keys -> Value = controllers[Control].Value;
    return;
}

#if (SPECS_VERSION > 0x0100)
EXPORT void CALL InitiateControllers(CONTROL_INFO ControlInfo)
#else
EXPORT void CALL InitiateControllers(p_void hMainWindow, CONTROL Controls[4])
#endif
{
    register int i;

    for (i = 0; i < MAX_CONTROLLERS; i++)
    {
        Controls[i].Present = FALSE;
        Controls[i].RawData = FALSE;
        Controls[i].Plugin = PLUGIN_NONE;
    }

/*
 * Raw data (low-level emulation of the controller serial commands) is not
 * yet emulated, and there is not a whole lot of open room for custom
 * settings to configure without it.  At the very least, Controller 1
 * should be plugged in, with mempak support from the core.
 */
    Controls[0].Present = TRUE;
    Controls[0].RawData = FALSE;
    Controls[0].Plugin = PLUGIN_MEMPAK;

    RomOpen();
#if (SPECS_VERSION == 0x0100)
    hMainWindow = hMainWindow; /* unused */
#endif
    return;
}

static signed char clamp_stick(signed long magnitude)
{
    if (magnitude < -128)
        return -128;
    if (magnitude > +127)
        return +127;
    return ((signed char)magnitude);
}

static int stick_range(void)
{
#if 0
    return 128; /* software limitation of the controller pad OS struct */
#else
    return clamp_stick(80); /* hardware limitation of the analog stick */
#endif
}

static NOINLINE size_t filter_OS_key_code(size_t signal);

EXPORT void CALL WM_KeyDown(size_t wParam, ssize_t lParam)
{
    size_t message;
    u16 mask;
    OS_CONT_PAD * pad;

    pad = &controllers[0].cont_pad;
    message = wParam; /* normally the correct key code message */
    if (message == 0 && lParam > 0)
        message = (size_t)lParam; /* Mupen64 for Linux uses lParam instead. */

    message = filter_OS_key_code(message);
    mask = press_masks[message];
    switch (mask)
    {
    case MASK_STICK_UP:
        if (already_pressed.up)
            break;
        pad->stick_x = clamp_stick(pad->stick_x + stick_range());
        already_pressed.up = 1;
        break;
    case MASK_STICK_DOWN:
        if (already_pressed.down)
            break;
        pad->stick_x = clamp_stick(pad->stick_x - stick_range());
        already_pressed.down = 1;
        break;
    case MASK_STICK_RIGHT:
        if (already_pressed.right)
            break;
        pad->stick_y = clamp_stick(pad->stick_y + stick_range());
        already_pressed.right = 1;
        break;
    case MASK_STICK_LEFT:
        if (already_pressed.left)
            break;
        pad->stick_y = clamp_stick(pad->stick_y - stick_range());
        already_pressed.left = 1;
        break;
    default:
        controllers[0].Value |=  swapped_bytes ? swap16by8(mask) : mask;
    }
    return;
}

EXPORT void CALL WM_KeyUp(size_t wParam, ssize_t lParam)
{
    size_t message;
    u16 mask;
    OS_CONT_PAD * pad;

    pad = &controllers[0].cont_pad;
    message = wParam;
    if (message == 0 && lParam > 0)
        message = (size_t)lParam;

    message = filter_OS_key_code(message);
    mask = press_masks[message];
    switch (mask)
    {
    case MASK_STICK_UP:
        pad->stick_x = clamp_stick(pad->stick_x - stick_range());
        already_pressed.up = 0;
        break;
    case MASK_STICK_DOWN:
        pad->stick_x = clamp_stick(pad->stick_x + stick_range());
        already_pressed.down = 0;
        break;
    case MASK_STICK_RIGHT:
        pad->stick_y = clamp_stick(pad->stick_y - stick_range());
        already_pressed.right = 0;
        break;
    case MASK_STICK_LEFT:
        pad->stick_y = clamp_stick(pad->stick_y + stick_range());
        already_pressed.left = 0;
        break;
    default:
        controllers[0].Value &= swapped_bytes ? ~swap16by8(mask) : ~mask;
    }
    return;
}

static NOINLINE size_t filter_OS_key_code(size_t signal)
{
#if defined(_WIN32) || defined(_WIN64)
    switch (signal)
    {
    case 0x25:  return KEYBOARD_LEFT; /* from VK_LEFT */
    case 0x26:  return KEYBOARD_UP; /* from VK_UP */
    case 0x27:  return KEYBOARD_RIGHT; /* from VK_RIGHT */
    case 0x28:  return KEYBOARD_DOWN; /* from VK_DOWN */

    case 0xBA:  return ';'; /* from VK_OEM_1 (`;:`) */
    case 0xDE:  return '\''; /* from VK_OEM_7 (`':`) */
    }
#else
    switch (signal & 0xFFFF) /* SDL 2.0 might mask in bit 30. */
    {
    case 273: /* SDLK_UP in SDL 1.x */
    case 82: /* SDLK_UP & SDL_SCANCODE_UP in SDL 2 */
        return KEYBOARD_UP;
    case 274: /* SDLK_DOWN in SDL 1.x */
    case 81: /* SDLK_DOWN & SDL_SCANCODE_DOWN in SDL 2 */
        return KEYBOARD_DOWN;
    case 275: /* SDLK_RIGHT in SDL 1.x */
    case 79: /* SDLK_RIGHT & SDL_SCANCODE_RIGHT in SDL 2 */
        return KEYBOARD_RIGHT;
    case 276: /* SDLK_LEFT in SDL 1.x */
    case 80: /* SDLK_LEFT & SDL_SCANCODE_LEFT in SDL 2 */
        return KEYBOARD_LEFT;
    }
#endif
    return (signal);
}
