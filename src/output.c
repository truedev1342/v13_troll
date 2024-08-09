#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "extra.h"
#include "code.h"
#include "command.h"
#include "output.h"

/* List of supported instructions */
const Instruction outlistInstructions[] = {
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
const char* outlistKeywords[] = {
    ".data", ".entry", ".extern", ".string", "add",
    "bne", "cmp", "dec", "endmacr", "inc",
    "jmp", "jsr", "lea", "macr", "mov",
    "not", "prn", "red", "rts", "stop",
    "sub", NULL /* End of list marker */
};

/* Output object code to a file */
void writeObjectToFile(Compiler* compiler, FILE* file, int valueData) {
    fprintf(file, "%04d %05o\n", compiler->numProgram, 0x7FFF & valueData); /* Print formatted output to file */

    compiler->numProgram++; /* Increment the program counter */
}

/* Output external labels to a file */
void writeExternalsToFile(Compiler* compiler, FILE* file) {
    int place;
    Symbol* label;

    for (place = 0; place < compiler->szExternals; place++) {
        label = compiler->arrayExternals[place]; /* Get external label from vector */
        fprintf(file, "%s 0%d\n", label->name, label->addr); /* Print external label info to file */
    }
}

/* Output entry labels and their addresses to a file */
void writeEntriesToFile(Compiler* compiler, FILE* file) {
    const char* name;
    int placeA;
    int placeB;
    Symbol* label;

    for (placeA = 0; placeA < compiler->szEntries; placeA++) {
        name = compiler->arrayEntries[placeA]; /* Get entry name from vector */
        for (placeB = 0; placeB < compiler->szSymbols; placeB++) {
            label = compiler->arraySymbols[placeB]; /* Get local label from vector */
            if (strcmp(label->name, name) == 0) {
                fprintf(file, "%s %d\n", label->name, label->addr); /* Print entry info to file */
            }
        }
    }
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

    iterator = &outlistInstructions[0];
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

                writeObjectToFile(compiler, outputFile, valueData);

                break;

            case 1: /* One argument expected */
                valueData = ADDRESS_ABS;
                valueData |= pattern->operationCode << 11;

                piece = firstPart(string);
                string = restString(string);
                typeArgument = LineCode_typeArgument(piece);
                valueData |= typeArgument << 3;

                writeObjectToFile(compiler, outputFile, valueData);

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

                writeObjectToFile(compiler, outputFile, valueData);

                usesTwoRegisters = conditionA && conditionB;
                if (usesTwoRegisters) {
                    registerOne = parseInt(argumentOne);
                    registerTwo = parseInt(argumentTwo);

                    valueData = ADDRESS_ABS;
                    valueData |= registerOne << 6;
                    valueData |= registerTwo << 3;

                    writeObjectToFile(compiler, outputFile, valueData);
                } else {
                    LineCode_outputArgument(compiler, argumentOne, 1, outputFile);
                    LineCode_outputArgument(compiler, argumentTwo, 2, outputFile);
                }
                break;
            }
        }
    }
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
            writeObjectToFile(compiler, file, outputValue); /* Write to file */

            piece = firstPart(string); /* Get the next piece */
            string = restString(string); /* Skip to the next piece */

        } while (*piece != '\0'); /* Loop until end of line */

        return;

    }

    if (strcmp(piece, ".entry") == 0) { /* Check if it's a .entry line */
        piece = firstPart(string); /* Get the next piece */
        string = restString(string); /* Skip to the next piece */

        insertEntry(compiler, piece); /* Record the entry */

        return;

    }

    if (strcmp(piece, ".string") == 0) { /* Check if it's a .string line */
        piece = firstPart(string); /* Get the next piece */
        string = restString(string); /* Skip to the next piece */

        piece++; /* Skip the opening quote */
        while (*piece != '"') {
            outputValue = *piece; /* Get the ASCII value of the character */
            writeObjectToFile(compiler, file, outputValue); /* Write to file */
            piece++; /* Move to the next character */

        }

        writeObjectToFile(compiler, file, 0); /* Write null terminator to file */

        return;

    }

}


/* Function to output argument to file */
void LineCode_outputArgument(
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

        writeObjectToFile(compiler, outputFile, valueData); /* Output to file */

        return;
    }

    if (letter == '#') { /* Check if it's an immediate value */
        value = parseInt(argument); /* Convert argument to integer */

        valueData = ADDRESS_ABS; /* Absolute flag */
        valueData |= value << 3; /* Shift value into place */
        writeObjectToFile(compiler, outputFile, valueData); /* Output to file */

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
        writeObjectToFile(compiler, outputFile, valueData); /* Output to file */
    } else {
        insertExternal(compiler, argument, compiler->numProgram); /* Insert external symbol */
        valueData = ADDRESS_EXT; /* External flag */
        writeObjectToFile(compiler, outputFile, valueData); /* Output to file */
    }
}


/* Function to determine the type of argument */
int LineCode_typeArgument(const char* argument) {
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

    iterator = outlistKeywords;
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
int LineCode_properArgument(const char* argument) {
    int type;

    type = LineCode_typeArgument(argument); /* Get argument type */
    if (type == ARGUMENT_CLASS_NONE) {
        return 0; /* Invalid argument */
    }

    return 1; /* Valid argument */
}
