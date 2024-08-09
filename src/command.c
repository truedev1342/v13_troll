#include <string.h>

#include "extra.h"
#include "command.h"
#include "output.h"

static const char* validCommandList[] = {
    ".data",
    ".entry",
    ".extern",
    ".string",
    NULL
};

/* Function to check if a given command corresponds to a valid line data directive */
int checkValidCommand(const char* command) {
    int i, sz = 4;
    for (i = 0; i < sz; i ++) {
        if (strcmp(validCommandList[i], command) == 0) {
            return 1;
        }
    }
    return 0; /* No match found */
}

/* check if a given string conforms to the format of entry command */

int checkEntryCommand(const char* origin) {
    const char* firstCmd;
    firstCmd = firstPart(origin); /* Get the next piece */
    origin = restString(origin); /* Skip to the next piece */

    if (*firstCmd == '\0') {
        return 0; /* Unexpected end of line */
    }

    firstCmd = firstPart(origin); /* Check for additional unexpected pieces */
    if (*firstCmd != '\0') {
        return 0; /* Additional unexpected pieces found */
    }
    return 1; /* Valid .entry format */
}

/* check if a given string conforms to the format of data command */
int checkDataCommand(const char* origin) {
    const char* firstCmd;
    int result = 0, value = 0;
    do {
        firstCmd = firstPart(origin); /* Get the next piece */
        origin = restString(origin); /* Skip to the next piece */

        if (*firstCmd == '\0') {
            return 0; /* Unexpected end of line */
        }

        result = sscanf(firstCmd, "%d", &value); /* Attempt to parse as integer */
        if (result != 1) {
            return 0; /* Failed to parse as integer */
        }

        firstCmd = firstPart(origin); /* Get the next piece */
        origin = restString(origin); /* Skip to the next piece */

        if (*firstCmd != ',' && *firstCmd != '\0') {
            return 0; /* Invalid separator or end of line expected */
        }

    } while (*firstCmd != '\0'); /* Loop until end of line */

    return 1; /* Valid .data format */
}

/* check if a given string conforms to the format of string command */
int checkStringCommand(const char* origin) {
    const char* firstCmd;
    firstCmd = firstPart(origin); /* Get the next piece */
    origin = restString(origin); /* Skip to the next piece */

    if (*firstCmd == '\0' || *firstCmd != '"' || firstCmd[strlen(firstCmd) - 1] != '"') {
        return 0; /* Invalid string format */
    }

    firstCmd = firstPart(origin); /* Check for additional unexpected pieces */
    if (*firstCmd != '\0') {
        return 0; /* Additional unexpected pieces found */
    }

    return 1; /* Valid .string format */
}

/* check if a given string conforms to the format of extern command */
int checkExternCommand(const char* origin) {
    const char* firstCmd;
    firstCmd = firstPart(origin); /* Get the next piece */
    origin = restString(origin); /* Skip to the next piece */

    if (*firstCmd == '\0') {
        return 0; /* Unexpected end of line */
    }

    firstCmd = firstPart(origin); /* Check for additional unexpected pieces */
    if (*firstCmd != '\0') {
        return 0; /* Additional unexpected pieces found */
    }

    return 1; /* Valid .extern format */
}

/* Function to check if a given string conforms to the format of a line data directive */
int checkCommandFormat(const char* origin) {
    const char* firstCmd;

    firstCmd = firstPart(origin); /* Get the first piece of the string */
    origin = restString(origin); /* Skip to the next piece */

    if (strcmp(firstCmd, ".entry") == 0) { /* Check if it's a .entry line */
        return checkEntryCommand(origin);
    }
    else if (strcmp(firstCmd, ".data") == 0) { /* Check if it's a .data line */
        return checkDataCommand(origin);
    }
    else if (strcmp(firstCmd, ".string") == 0) { /* Check if it's a .string line */
        return checkStringCommand(origin);
    }
    else if (strcmp(firstCmd, ".extern") == 0) { /* Check if it's a .extern line */
        return checkExternCommand(origin);
    }

    return 0; /* Not recognized as any valid line data directive */
}
/* check given command data(data) is valid */
int checkDataInData(const char* origin){
    const char* firstCommand;
    int szCalculated = 0;
    do {
        firstCommand = firstPart(origin); /* Get the next piece */
        origin = restString(origin); /* Skip to the next piece */

        if (*firstCommand == '\0') {
            return 0; /* Unexpected end of line */
        }

        szCalculated++; /* Count the number of data entries */

        firstCommand = firstPart(origin); /* Get the next piece */
        origin = restString(origin); /* Skip to the next piece */

        if (*firstCommand != ',' && *firstCommand != '\0') {
            return 0; /* Invalid separator or end of line expected */
        }

    } while (*firstCommand != '\0'); /* Loop until end of line */

    return szCalculated; /* Return the calculated size of data entries */
}

/* check given command data(string) is valid */
int checkDataInString(const char* origin){
    const char* firstCommand;
    int szCalculated = 0;
    firstCommand = firstPart(origin); /* Get the next piece */
    origin = restString(origin); /* Skip to the next piece */

    szCalculated = strlen(firstCommand) - 1; /* Calculate the length of the string excluding quotes */

    return szCalculated; /* Return the calculated size of the string */
}

/* check given command data is valid */
int checkCommandData(const char* origin) {
    const char* firstCommand;
    

    firstCommand = firstPart(origin); /* Get the first piece of the string */
    origin = restString(origin); /* Skip to the next piece */

    if (strcmp(firstCommand, ".data") == 0) { /* Check if it's a .data line */
        return checkDataInData(origin);
    }

    else if (strcmp(firstCommand, ".string") == 0) { /* Check if it's a .string line */
        return checkDataInString(origin);
    }

    return 0; /* Not recognized as any valid data directive */
}
