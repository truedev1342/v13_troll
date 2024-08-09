#include <string.h>

#include "extra.h"
#include "command.h"

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

/* Function to check if a given string conforms to the format of a line data directive */
int checkCommandFormat(const char* string) {
    const char* piece;
    int result;
    int value;

    result = 0;
    value = 0;

    piece = firstPart(string); /* Get the first piece of the string */
    string = restString(string); /* Skip to the next piece */

    if (strcmp(piece, ".data") == 0) { /* Check if it's a .data line */
        do {
            piece = firstPart(string); /* Get the next piece */
            string = restString(string); /* Skip to the next piece */

            if (*piece == '\0') {
                return 0; /* Unexpected end of line */
            }

            result = sscanf(piece, "%d", &value); /* Attempt to parse as integer */
            if (result != 1) {
                return 0; /* Failed to parse as integer */
            }

            piece = firstPart(string); /* Get the next piece */
            string = restString(string); /* Skip to the next piece */

            if (*piece != ',' && *piece != '\0') {
                return 0; /* Invalid separator or end of line expected */
            }

        } while (*piece != '\0'); /* Loop until end of line */

        return 1; /* Valid .data format */

    }

    if (strcmp(piece, ".entry") == 0) { /* Check if it's a .entry line */
        piece = firstPart(string); /* Get the next piece */
        string = restString(string); /* Skip to the next piece */

        if (*piece == '\0') {
            return 0; /* Unexpected end of line */
        }

        piece = firstPart(string); /* Check for additional unexpected pieces */
        if (*piece != '\0') {
            return 0; /* Additional unexpected pieces found */
        }

        return 1; /* Valid .entry format */

    }

    if (strcmp(piece, ".extern") == 0) { /* Check if it's a .extern line */
        piece = firstPart(string); /* Get the next piece */
        string = restString(string); /* Skip to the next piece */

        if (*piece == '\0') {
            return 0; /* Unexpected end of line */
        }

        piece = firstPart(string); /* Check for additional unexpected pieces */
        if (*piece != '\0') {
            return 0; /* Additional unexpected pieces found */
        }

        return 1; /* Valid .extern format */

    }

    if (strcmp(piece, ".string") == 0) { /* Check if it's a .string line */
        piece = firstPart(string); /* Get the next piece */
        string = restString(string); /* Skip to the next piece */

        if (*piece == '\0' || *piece != '"' || piece[strlen(piece) - 1] != '"') {
            return 0; /* Invalid string format */
        }

        piece = firstPart(string); /* Check for additional unexpected pieces */
        if (*piece != '\0') {
            return 0; /* Additional unexpected pieces found */
        }

        return 1; /* Valid .string format */

    }

    return 0; /* Not recognized as any valid line data directive */
}

int checkCommandData(const char* string) {
    const char* piece;
    int calculatedSize;

    calculatedSize = 0;

    piece = firstPart(string); /* Get the first piece of the string */
    string = restString(string); /* Skip to the next piece */

    if (strcmp(piece, ".data") == 0) { /* Check if it's a .data line */
        do {
            piece = firstPart(string); /* Get the next piece */
            string = restString(string); /* Skip to the next piece */

            if (*piece == '\0') {
                return 0; /* Unexpected end of line */
            }

            calculatedSize++; /* Count the number of data entries */

            piece = firstPart(string); /* Get the next piece */
            string = restString(string); /* Skip to the next piece */

            if (*piece != ',' && *piece != '\0') {
                return 0; /* Invalid separator or end of line expected */
            }

        } while (*piece != '\0'); /* Loop until end of line */

        return calculatedSize; /* Return the calculated size of data entries */

    }

    if (strcmp(piece, ".string") == 0) { /* Check if it's a .string line */
        piece = firstPart(string); /* Get the next piece */
        string = restString(string); /* Skip to the next piece */

        calculatedSize = strlen(piece) - 1; /* Calculate the length of the string excluding quotes */

        return calculatedSize; /* Return the calculated size of the string */

    }

    return 0; /* Not recognized as any valid data directive */
}

void writeCommandToFile(
    Compiler* compiler,
    const char* string,
    FILE* file
) {
    const char* piece;
    int outputValue;
    int value;

    outputValue = 0;
    value = 0;

    piece = firstPart(string); /* Get the first piece of the string */
    string = restString(string); /* Skip to the next piece */

    if (strcmp(piece, ".data") == 0) { /* Check if it's a .data line */
        do {
            piece = firstPart(string); /* Get the next piece */
            string = restString(string); /* Skip to the next piece */

            sscanf(piece, "%d", &value); /* Convert the piece to an integer */

            outputValue = value; /* Set the output value */
            Parser_fileOutputObject(compiler, file, outputValue); /* Write to file */

            piece = firstPart(string); /* Get the next piece */
            string = restString(string); /* Skip to the next piece */

        } while (*piece != '\0'); /* Loop until end of line */

        return;

    }

    if (strcmp(piece, ".entry") == 0) { /* Check if it's a .entry line */
        piece = firstPart(string); /* Get the next piece */
        string = restString(string); /* Skip to the next piece */

        Parser_recordEntry(compiler, piece); /* Record the entry */

        return;

    }

    if (strcmp(piece, ".string") == 0) { /* Check if it's a .string line */
        piece = firstPart(string); /* Get the next piece */
        string = restString(string); /* Skip to the next piece */

        piece++; /* Skip the opening quote */
        while (*piece != '"') {
            outputValue = *piece; /* Get the ASCII value of the character */
            Parser_fileOutputObject(compiler, file, outputValue); /* Write to file */
            piece++; /* Move to the next character */

        }

        Parser_fileOutputObject(compiler, file, 0); /* Write null terminator to file */

        return;

    }

}
