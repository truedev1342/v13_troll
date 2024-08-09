#include <ctype.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "code.h"
#include "extra.h"
#include "output.h"

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
int checkValidCodeName(const char* name) {
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
int checkCodeFormatValidity(const char* string) {
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
int checkCodeDataValidity(const char* string) {
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
