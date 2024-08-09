#pragma once 

#include <stdio.h>

#include "template.h"

/* Address indicating absolute addressing */
#define ADDRESS_ABS (4)

/* Address indicating relative addressing */
#define ADDRESS_REL (2)

/* Address indicating external symbol */
#define ADDRESS_EXT (1)

/* Structure representing a local symbol */
typedef struct {
    char* name;     /* Name of the symbol */
    int addr;    /* Address of the symbol */
} Symbol;

/* Structure representing a compiler with various attributes */
typedef struct {
    int programCounter;         /* Program counter */

    int szCode;               /* Size of code */
    int szCommand;               /* Size of command */
    int cntErrors;                 /* Error count */

    Symbol** arraySymbols;       /* Array of local symbols */
    int szSymbols;            /* Total number of local symbols */

    Symbol** arrayExternals;    /* Array of external symbols */
    int szExternals;         /* Total number of entry symbols */

    char** arrayEntries;       /* Array of entry symbols */
    int szEntries;           /* Total number of external symbols */

    Templates* templates;             /* Pointer to templates */
    Template* templateInside;         /* Pointer to template inside */
} Compiler;

/* Creates a new compiler instance */
Compiler* Parser_new();

/* Deletes a compiler instance, freeing its memory */
void Parser_delete(Compiler* compiler);

/* Performs the first pass of parsing */
void Parser_first(Compiler* compiler,
                  FILE* fileInput,
                  FILE* fileFirstParser,
                  FILE* fileErrors);

/* Performs the second pass of parsing */
void Parser_second(Compiler* compiler,
                   FILE* fileFirstParser,
                   FILE* fileOutputObject);

/* Records an external symbol with its address */
void Parser_recordExternal(Compiler* compiler,
                           const char* external,
                           int address);

/* Records an entry symbol */
void Parser_recordEntry(Compiler* compiler,
                        const char* entry);

/* Outputs the object file */
void Parser_fileOutputObject(Compiler* compiler,
                             FILE* fileOutputObject,
                             int data);

/* Outputs the external symbols file */
void Parser_fileOutputExternals(Compiler* compiler,
                                FILE* fileOutputExternals);

/* Outputs the entry symbols file */
void Parser_fileOutputEntries(Compiler* compiler,
                              FILE* fileOutputEntries);
