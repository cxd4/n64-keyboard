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

static u16 swap16by8(u16 word)
{
    u16 swapped;

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
    int * memory_normal, * memory_swapped;

    system_version = &(PluginInfo -> Version);
    plugin_type    = &(PluginInfo -> Type);
    name           = &(PluginInfo -> Name[0]);
    memory_normal  = &(PluginInfo -> Reserved1);
    memory_swapped = &(PluginInfo -> Reserved2); /* bug in PJ 1.4; needs TRUE */

    *(system_version) = SPECS_VERSION;
    *(plugin_type)    = PLUGIN_TYPE_CONTROLLER;
    *(memory_normal)  = 0;
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
 /* to do:  implement directional/digital joypad right/left/down/up */
    press_masks['E'] = MASK_START_BUTTON;
    press_masks['O'] = MASK_Z_TRIG;
    press_masks['K'] = MASK_B_BUTTON;
    press_masks['J'] = MASK_A_BUTTON;
    return;
}

EXPORT void CALL GetKeys(int Control, BUTTONS * Keys)
{
    assert(Control < MAX_CONTROLLERS);
    assert(Keys != NULL);
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

static NOINLINE u16 translate_OS_key_press(size_t signal);

EXPORT void CALL WM_KeyDown(unsigned int wParam, i32 lParam)
{
    size_t message;

    message = wParam; /* normally the correct key code message */
    if (message == 0 && lParam != 0 && lParam <= 32767)
        message = (size_t)lParam; /* Mupen64 for Linux uses lParam instead. */
    controllers[0].Value |=  translate_OS_key_press(message);
    return;
}

EXPORT void CALL WM_KeyUp(unsigned int wParam, i32 lParam)
{
    size_t message;

    message = wParam;
    if (message == 0 && lParam != 0 && lParam <= 32767)
        message = (size_t)lParam;
    controllers[0].Value &= ~translate_OS_key_press(message);
    return;
}

static NOINLINE u16 translate_OS_key_press(size_t signal)
{
    u16 mask;

#if defined(_WIN32) || defined(_WIN64)
    switch (signal)
    {
    case 0xBA: /* VK_OEM_1 (`;:`) */
        signal = ';';
        break;
    case 0xDE: /* VK_OEM_7 (`':`) */
        signal = '\'';
        break;
    }
#elif defined(_SDL_H)
/* SDL 1.2 I think mostly maps as much to ASCII as possible...SDL 2.0 dunno? */
#else
#error Untested keyboard interface--does your operating system match ASCII?
#endif

    mask = press_masks[signal];
    return (swapped_bytes ? swap16by8(mask) : mask);
}
