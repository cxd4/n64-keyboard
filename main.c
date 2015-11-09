#include <assert.h>
#include <malloc.h>
#include <string.h>

/* to do:  have working controller #1.1 implementation */
#define SPECS_VERSION           0x0100
#include "contr.h"
#include "buttons.h"

/* to do:  Wasn't there some old design when this could be 8? */
#define MAX_CONTROLLERS         4
static BUTTONS controllers[MAX_CONTROLLERS];

static control_stick_activity already_pressed;

static INLINE u16 swap16by8(u16 word)
{
    u16 swapped;

/*
 * MSVC 2005 (CL.EXE /O1 /Os):  ROL     ax, 8 # no function call/ret
 * GCC 4.8.1 (gcc -Os -ansi) :  XCHG    al, ah # no function call/ret
 */
    swapped = 0x0000
      | ((word & 0x00FFu) << 8)
      | ((word & 0xFF00u) >> 8)
    ;
    return (swapped &= 0xFFFFu);
}

EXPORT void CALL GetDllInfo(PLUGIN_INFO * PluginInfo)
{
    u16 * system_version, * plugin_type;
    int/* * memory_normal,*/ * memory_swapped;

    system_version = &(PluginInfo -> Version);
    plugin_type    = &(PluginInfo -> Type);
 /* memory_normal  = &(PluginInfo -> Reserved1); // could be a bug in 1.4 */
    memory_swapped = &(PluginInfo -> Reserved2); /* bug in PJ 1.4; needs TRUE */

    *(system_version) = SPECS_VERSION;
    *(plugin_type)    = PLUGIN_TYPE_CONTROLLER;
 /* *(memory_normal)  = 0; // reserved, shouldn't be set ideally */
    *(memory_swapped) = ENDIAN_M;

    strcpy(&(PluginInfo -> Name[0]), "System Keyboard");
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
    map_keys();
    return;
}

EXPORT void CALL GetKeys(int Control, BUTTONS * Keys)
{
    u16 buttons;
    const u16 control_stick_exception = /* real N64 control stick hazard */
#if (ENDIAN_M == 0)
        MASK_START_BUTTON | MASK_L_TRIG | MASK_R_TRIG;
#else
        swap16by8(MASK_START_BUTTON | MASK_L_TRIG | MASK_R_TRIG);
#endif

    assert(Control < MAX_CONTROLLERS);
    buttons = controllers[Control].Value & 0x0000FFFFul;

    assert(Keys != NULL);
    if ((buttons & control_stick_exception) == control_stick_exception) {
        controllers[Control].Value &= 0x0000FFFFul; /* analog stick reset */
        buttons ^= ENDIAN_M ? swap16by8(MASK_START_BUTTON) : MASK_START_BUTTON;
        Keys -> Value = buttons;
        already_pressed.hazard_recovery_mode = 1;
        return;
    }
    Keys -> Value = controllers[Control].Value;
    already_pressed.hazard_recovery_mode = 0;
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
    static const int magnitudes[] = {
         80, /* hardware limitation:  Strongest real N64 stick presses do 80. */
        128, /* software limitation (Hold Shift.):  stick_mask & 0xFF >= -128 */
         64, /* a little slow (Hold Ctrl.) */
         32, /* very slow (Hold Ctrl+Shift.) */
    };
    const int shift_amount = (ENDIAN_M ? 6 + 8 : 6 + 0);

    return magnitudes[(controllers[0].Value >> shift_amount) & 3];
}

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
#if (ENDIAN_M != 0)
        mask = swap16by8(mask); /* PluginInfo memory adjustment */
#endif
        controllers[0].Value |=  mask;
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
#if (ENDIAN_M != 0)
        mask = swap16by8(mask); /* PluginInfo memory adjustment */
#endif
        controllers[0].Value &= ~mask;
    }
    return;
}
