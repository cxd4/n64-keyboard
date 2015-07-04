#include <string.h>

/* to do:  have working controller #1.1 implementation */
#define SPECS_VERSION           0x0100
#include "contr.h"

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
