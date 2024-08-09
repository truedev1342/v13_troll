#pragma once

#include "compiler.h"

/* Argument classes for instructions */
#define ARGUMENT_CLASS_NONE             (0)
#define ARGUMENT_CLASS_IMMEDIATE        (1)
#define ARGUMENT_CLASS_DIRECT           (2)
#define ARGUMENT_CLASS_INDIRECT         (4)
#define ARGUMENT_CLASS_REGISTER         (8)

/* Instruction structure */
typedef struct {
    const char* name; /* Name of the instruction */
    int operationCode; /* Operation code of the instruction */
    int arguments; /* Number of arguments the instruction takes */
} Instruction;

/* Checks validity if the given name is valid */
int checkValidCodeName(const char* name);

/* Checks validity the format of the provided string */
int checkCodeFormatValidity(const char* string);

/* Checks the validity of the code data in the string */
int checkCodeDataValidity(const char* string);

