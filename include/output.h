#pragma once

#include "compiler.h"

int LineCode_typeArgument(const char* argument); /* Determines argument type */
int LineCode_properArgument(const char* argument); /* Checks validity of argument */
void LineCode_outputArgument( Compiler* compiler, const char* argument, int side, FILE* outputFile); /* Outputs argument to file */

/* Outputs the object file */
void writeObjectToFile(Compiler* compiler, FILE* file, int data);

/* Outputs the external symbols file */
void writeExternalsToFile(Compiler* compiler, FILE* file);

/* Outputs the entry symbols file */
void writeEntriesToFile(Compiler* compiler, FILE* file);

/* Writes the provided phrase to the specified output file */
void LineCode_writeToFile(Compiler* compiler, const char* phrase, FILE* outputFile);

/* Writes the given string to the specified file */
void writeCommandToFile(Compiler* compiler, const char* command, FILE* file);