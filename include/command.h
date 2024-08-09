#pragma once

#include "compiler.h"

/* Checks if the provided command is valid */
int checkValidCommand(const char* command);

/* Checks the format of the given string */
int checkCommandFormat(const char* command);

/* Checks the validity of data in the provided string */
int checkCommandData(const char* command);

/* Writes the given string to the specified file */
void writeCommandToFile(Compiler* compiler, const char* command, FILE* file);
