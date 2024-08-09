#include <ctype.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "code.h"
#include "extra.h"

/* Argument classes for instructions */
#define ARGUMENT_CLASS_NONE             (0)
#define ARGUMENT_CLASS_IMMEDIATE        (1)
#define ARGUMENT_CLASS_DIRECT           (2)
#define ARGUMENT_CLASS_INDIRECT         (4)
#define ARGUMENT_CLASS_REGISTER         (8)

/* Function prototypes */
static int LineCode_typeArgument(const char* argument); /* Determines argument type */
static int LineCode_properArgument(const char* argument); /* Checks validity of argument */
static void LineCode_outputArgument(
    Compiler* compiler,
    const char* argument,
    int side,
    FILE* outputFile
); /* Outputs argument to file */

/* Instruction structure */
typedef struct {
    const char* name; /* Name of the instruction */
    int operationCode; /* Operation code of the instruction */
    int arguments; /* Number of arguments the instruction takes */
} Instruction;

/* List of supported instructions */
const Instruction listInstructions[] = {
    { "mov",  0, 2 },
    { "cmp",  1, 2 },
    { "add",  2, 2 },
    { "sub",  3, 2 },
    { "lea",  4, 2 },
    { "clr",  5, 1 },
    { "not",  6, 1 },
    { "inc",  7, 1 },
    { "dec",  8, 1 },
    { "jmp",  9, 1 },
    { "bne", 10, 1 },
    { "red", 11, 1 },
    { "prn", 12, 1 },
    { "jsr", 13, 1 },
    { "rts", 14, 0 },
    { "stop", 15, 0 },
    { NULL,  0, 0 } /* End of list marker */
};

/* List of keywords */
const char* listKeywords[] = {
    ".data", ".entry", ".extern", ".string", "add",
    "bne", "cmp", "dec", "endmacr", "inc",
    "jmp", "jsr", "lea", "macr", "mov",
    "not", "prn", "red", "rts", "stop",
    "sub", NULL /* End of list marker */
};

/* Function to check if a given name is a valid instruction */
int LineCode_checkName(const char* name) {
    const Instruction* iterator;

    iterator = &listInstructions[0];
    while (iterator->name != NULL) {
        if (strcmp(iterator->name, name) == 0) {
            return 1; /* Found matching instruction */
        }
        iterator++;
    }
    return 0; /* Instruction not found */
}

/* Function to check if a given string matches the syntax of any instruction */
int LineCode_checkForm(const char* string) {
    const char* piece;
    const Instruction* pattern;
    const Instruction* iterator;

    pattern = NULL;

    piece = firstPart(string); /* Get the first piece of the string */
    string = restString(string); /* Skip over the first piece */

    iterator = &listInstructions[0];
    while (iterator->name != NULL) {
        if (strcmp(iterator->name, piece) == 0) {
            pattern = iterator; /* Match found, set the pattern */
            break;
        }
        iterator++;
    }

    if (pattern != NULL) {
        switch (pattern->arguments) {
            case 0: /* No arguments expected */
                piece = firstPart(string);
                string = restString(string);
                if (piece[0] == '\0') {
                    return 1; /* Valid syntax */
                }
                break;

            case 1: /* One argument expected */
                piece = firstPart(string);
                string = restString(string);
                if (!LineCode_properArgument(piece)) {
                    return 0; /* Invalid argument */
                }
                piece = firstPart(string);
                string = restString(string);
                if (piece[0] == '\0') {
                    return 1; /* Valid syntax */
                }
                break;

            case 2: /* Two arguments expected */
                piece = firstPart(string);
                string = restString(string);
                if (!LineCode_properArgument(piece)) {
                    return 0; /* Invalid first argument */
                }
                piece = firstPart(string);
                string = restString(string);
                if (piece[0] != ',') {
                    return 0; /* Expected comma separator */
                }
                piece = firstPart(string);
                string = restString(string);
                if (!LineCode_properArgument(piece)) {
                    return 0; /* Invalid second argument */
                }
                piece = firstPart(string);
                string = restString(string);
                if (piece[0] == '\0') {
                    return 1; /* Valid syntax */
                }
                break;
        }
    }

    return 0; /* Default: Invalid syntax */
}

/* Function to check the type of data instruction */
int LineCode_checkData(const char* string) {
    const char* piece;
    const Instruction* pattern;
    const Instruction* iterator;
    int calculatedSize;
    int conditionA;
    int conditionB;
    int usesTwoRegisters;
    int typeArgument;

    pattern = NULL;

    piece = firstPart(string); /* Get the first piece of the string */
    string = restString(string); /* Skip over the first piece */

    iterator = &listInstructions[0];
    while (iterator->name != NULL) {
        if (strcmp(iterator->name, piece) == 0) {
            pattern = iterator; /* Match found, set the pattern */
            break;
        }
        iterator++;
    }

    if (pattern != NULL) {
        switch (pattern->arguments) {
            case 0: /* No arguments expected */
                return 1; /* Single-word instruction */
            case 1: /* One argument expected */
                return 2; /* Two words (instruction + argument) */
            case 2: /* Two arguments expected */
                usesTwoRegisters = 0;

                piece = firstPart(string);
                string = restString(string);
                typeArgument = LineCode_typeArgument(piece);

                conditionA = 0;
                conditionA |= typeArgument == ARGUMENT_CLASS_REGISTER;
                conditionA |= typeArgument == ARGUMENT_CLASS_INDIRECT;

                piece = firstPart(string);
                string = restString(string);
                if (piece[0] != ',') {
                    break; /* Expected comma separator */
                }

                piece = firstPart(string);
                string = restString(string);
                typeArgument = LineCode_typeArgument(piece);

                conditionB = 0;
                conditionB |= typeArgument == ARGUMENT_CLASS_REGISTER;
                conditionB |= typeArgument == ARGUMENT_CLASS_INDIRECT;

                usesTwoRegisters = conditionA && conditionB;
                calculatedSize = usesTwoRegisters ? 2 : 3;
                return calculatedSize; /* Return size based on arguments */
        }
    }

    return 0; /* Default: Invalid instruction */
}

/* Function to write data to file based on instruction */
void LineCode_writeToFile(
    Compiler* compiler,
    const char* string,
    FILE* outputFile
) {
    const char* piece;
    char argumentOne[MAX_N];
    char argumentTwo[MAX_N];
    const Instruction* pattern;
    const Instruction* iterator;
    int valueData;
    int registerOne;
    int registerTwo;
    int typeArgument;

    pattern = NULL;

    valueData = 0;
    typeArgument = 0;

    piece = firstPart(string); /* Get the first piece of the string */
    string = restString(string); /* Skip over the first piece */

    iterator = &listInstructions[0];
    while (iterator->name != NULL) {
        if (strcmp(iterator->name, piece) == 0) {
            pattern = iterator; /* Match found, set the pattern */
            break;
        }
        iterator++;
    }

    if (pattern != NULL) {
        switch (pattern->arguments) {
            case 0: /* No arguments expected */
                valueData = ADDRESS_ABS;
                valueData |= pattern->operationCode << 11;

                Parser_fileOutputObject(compiler, outputFile, valueData);

                break;

            case 1: /* One argument expected */
                valueData = ADDRESS_ABS;
                valueData |= pattern->operationCode << 11;

                piece = firstPart(string);
                string = restString(string);
                typeArgument = LineCode_typeArgument(piece);
                valueData |= typeArgument << 3;

                Parser_fileOutputObject(compiler, outputFile, valueData);

                LineCode_outputArgument(compiler, piece, 1, outputFile);

                break;

            case 2:{ /* Two arguments expected */
                int conditionA;
                int conditionB;
                int usesTwoRegisters;
                int typeArgument;

                valueData = ADDRESS_ABS;
                valueData |= pattern->operationCode << 11;

                usesTwoRegisters = 0;

                piece = firstPart(string);
                string = restString(string);
                typeArgument = LineCode_typeArgument(piece);
                strcpy(argumentOne, piece);
                valueData |= typeArgument << 7;

                conditionA = 0;
                conditionA |= typeArgument == ARGUMENT_CLASS_REGISTER;
                conditionA |= typeArgument == ARGUMENT_CLASS_INDIRECT;

                piece = firstPart(string);
                string = restString(string);

                piece = firstPart(string);
                string = restString(string);
                typeArgument = LineCode_typeArgument(piece);
                strcpy(argumentTwo, piece);
                valueData |= typeArgument << 3;

                conditionB = 0;
                conditionB |= typeArgument == ARGUMENT_CLASS_REGISTER;
                conditionB |= typeArgument == ARGUMENT_CLASS_INDIRECT;

                Parser_fileOutputObject(compiler, outputFile, valueData);

                usesTwoRegisters = conditionA && conditionB;
                if (usesTwoRegisters) {
                    registerOne = parseInt(argumentOne);
                    registerTwo = parseInt(argumentTwo);

                    valueData = ADDRESS_ABS;
                    valueData |= registerOne << 6;
                    valueData |= registerTwo << 3;

                    Parser_fileOutputObject(compiler, outputFile, valueData);
                } else {
                    LineCode_outputArgument(compiler, argumentOne, 1, outputFile);
                    LineCode_outputArgument(compiler, argumentTwo, 2, outputFile);
                }
                break;
            }
        }
    }
}

/* Function to determine the type of argument */
static int LineCode_typeArgument(const char* argument) {
    const char* special;
    const char** iterator;
    char letter;
    int result;
    int value;

    result = 0;
    value = 0;

    letter = argument[0]; /* Get the first character of the argument */

    if (letter == 'r') { /* Check if it starts with 'r' for registers */
        result = sscanf(&argument[1], "%d", &value); /* Attempt to parse the number after 'r' */
        if (result == 1 && 0 <= value && value <= 7) { /* Check valid register number */
            return ARGUMENT_CLASS_REGISTER; /* Return register class */
        }
    }

    if (letter == '#') { /* Check if it starts with '#' for immediate values */
        result = sscanf(&argument[1], "%d", &value); /* Attempt to parse the number after '#' */
        if (result == 1) {
            return ARGUMENT_CLASS_IMMEDIATE; /* Return immediate value class */
        }
    }

    if (letter == '*') { /* Check if it starts with '*' for indirect addressing */
        letter = argument[1]; /* Check the character after '*' */
        if (letter != 'r') {
            return ARGUMENT_CLASS_NONE; /* Invalid indirect addressing */
        }

        result = sscanf(&argument[2], "%d", &value); /* Attempt to parse the number after '*r' */
        if (result == 1 && 0 <= value && value <= 7) { /* Check valid register number */
            return ARGUMENT_CLASS_INDIRECT; /* Return indirect class */
        }
    }

    iterator = listKeywords;
    while (*iterator != NULL) { /* Check against list of keywords */
        special = *iterator;
        if (strcmp(special, argument) == 0) {
            return ARGUMENT_CLASS_NONE; /* Return none class for keywords */
        }
        iterator++;
    }

    return ARGUMENT_CLASS_DIRECT; /* Default to direct class */
}

/* Function to check if argument is valid */
static int LineCode_properArgument(const char* argument) {
    int type;

    type = LineCode_typeArgument(argument); /* Get argument type */
    if (type == ARGUMENT_CLASS_NONE) {
        return 0; /* Invalid argument */
    }

    return 1; /* Valid argument */
}

/* Function to output argument to file */
static void LineCode_outputArgument(
    Compiler* compiler,
    const char* argument,
    int side,
    FILE* outputFile
) {
    int place;
    int valueData;
    int value;
    char letter;
    Symbol* label;

    letter = *argument; /* Get the first character of the argument */

    if (letter == 'r' || letter == '*') { /* Check if it's a register or indirect address */
        value = parseInt(argument); /* Convert argument to integer */
        valueData = ADDRESS_ABS; /* Absolute flag */

        switch (side) {
            case 1: /* First side */
                valueData |= value << 6; /* Shift value into place */
                break;
            case 2: /* Second side */
                valueData |= value << 3; /* Shift value into place */
                break;
        }

        Parser_fileOutputObject(compiler, outputFile, valueData); /* Output to file */

        return;
    }

    if (letter == '#') { /* Check if it's an immediate value */
        value = parseInt(argument); /* Convert argument to integer */

        valueData = ADDRESS_ABS; /* Absolute flag */
        valueData |= value << 3; /* Shift value into place */
        Parser_fileOutputObject(compiler, outputFile, valueData); /* Output to file */

        return;
    }

    label = NULL;
    for (place = 0; place < compiler->szSymbols; place++) { /* Iterate through local labels */
        if (strcmp(argument, compiler->arraySymbols[place]->name) == 0) { /* Check for matching label */
            label = compiler->arraySymbols[place]; /* Set label */
            break;
        }
    }

    if (label != NULL) {
        valueData = ADDRESS_REL; /* Relative flag */
        valueData |= label->addr << 3; /* Set address */
        Parser_fileOutputObject(compiler, outputFile, valueData); /* Output to file */
    } else {
        Parser_recordExternal(compiler, argument, compiler->programCounter); /* Record external symbol */
        valueData = ADDRESS_EXT; /* External flag */
        Parser_fileOutputObject(compiler, outputFile, valueData); /* Output to file */
    }
}
