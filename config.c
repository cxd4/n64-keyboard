#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "contr.h"
#include "analog.h"

CONTROL controls[MAX_CONTROLLERS];

EXPORT void CALL DllConfig(void * hParent)
{
    static char line[80 + 1];
    size_t number;
    long parsed_value;
    char * position, * result;
    FILE * conf;
    register int i;

    for (i = 0; i < MAX_CONTROLLERS; i++) {
        controls[i].Present = -1;
        controls[i].RawData = -1;
        controls[i].Plugin  = -1;
    } /* Reset to negative values (signals to skip updating). */
    conf = fopen("keyboard.cfg", "r");
    if (conf == NULL)
        return; /* Absence of a configuration file defaults to dummy input. */
    number = 0;

    while (!feof(conf)) {
        result = fgets(&line[0], sizeof(line), conf);
        if (result == NULL)
            break;

        position = strchr(&line[0], '=');
        if (position == NULL)
            continue;

        *(position) = '\0'; /* Divide into a key string and a value string. */
        result = strchr(&line[0], '[');
        if (strchr(&line[0], ']') + 1 == position && result < position) {
            *(position - 1) = '\0';
            number = (size_t)strtoul(result + 1, NULL, 0);
            if (number >= MAX_CONTROLLERS)
	        continue;
            *(result) = '\0';
        }

        parsed_value = strtol(position + 1, NULL, 0);
        if (strcmp(&line[0], "CONNECT") == 0) { /* plugged in to control deck */
            controls[number].Present = (int)parsed_value;
            continue;
        }
        if (strcmp(&line[0], "RAW_PIF") == 0) { /* raw PIF command emulation */
            controls[number].RawData = (int)parsed_value;
            continue;
        }
        if (strcmp(&line[0], "DEVICE") == 0) { /* Controller Pak accessory */
            controls[number].Plugin  = (int)parsed_value;
            continue;
        }

/*
 * To do:
 * Per-ROM controller configuration?  Re-map buttons to other keys?
 */
    }

    if (ferror(conf))
        fputs("Failed to read one or more bytes from settings file.\n", stderr);
    while (fclose(conf) != 0)
        ;
    return;
}
