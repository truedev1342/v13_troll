#pragma once

#include "compiler.h"

/* Checks if the given name is valid */
int LineCode_checkName(const char* name);

/* Checks the format of the provided phrase */
int LineCode_checkForm(const char* phrase);

/* Checks the validity of the data in the phrase */
int LineCode_checkData(const char* phrase);

/* Writes the provided phrase to the specified output file */
void LineCode_writeToFile(Compiler* compiler, const char* phrase, FILE* outputFile);
