#include <assert.h>
#include <string.h>

/* to do:  have working controller #1.1 implementation */
#define SPECS_VERSION           0x0100
#include "contr.h"

/* to do:  Wasn't there some old design when this could be 8? */
#define MAX_CONTROLLERS         4
static BUTTONS controllers[MAX_CONTROLLERS];

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
    *(memory_swapped) = 1; /* Project64 1.4 reads this as an endian flag. */

    strcpy(name, "System Keyboard");
    return;
}

EXPORT void CALL CloseDLL(void)
{
    return;
}

EXPORT void CALL RomClosed(void)
{
    return;
}

EXPORT void CALL RomOpen(void)
{
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

#if (SPECS_VERSION == 0x0100)
    hMainWindow = hMainWindow; /* unused */
#endif
    return;
}

static NOINLINE u32 translate_OS_key_press(size_t signal);

EXPORT void CALL WM_KeyDown(unsigned int wParam, i32 lParam)
{
    size_t message;

    assert(wParam == 0 || lParam == 0); /* Linux seems to expect lParam. */
    message = (wParam != 0) ? wParam : (size_t)lParam;
    controllers[0].Value |=  translate_OS_key_press(message);
    return;
}

EXPORT void CALL WM_KeyUp(unsigned int wParam, i32 lParam)
{
    size_t message;

    assert(wParam == 0 || lParam == 0);
    message = (wParam != 0) ? wParam : (size_t)lParam;
    controllers[0].Value &= ~translate_OS_key_press(message);
    return;
}

static NOINLINE u32 translate_OS_key_press(size_t signal)
{
    u32 mask;

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

    mask = 0x00000000;
    switch (signal)
    {
/* N64 button masks go here. */
    }
    return (mask);
}
