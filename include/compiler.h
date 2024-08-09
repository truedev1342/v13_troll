#pragma once 

#include <stdio.h>

#include "template.h"

#define ADDRESS_ABS (4)

#define ADDRESS_REL (2)

#define ADDRESS_EXT (1)

typedef struct {
    char* name;     /* Name  */
    int addr;    /* Address */
} Symbol;

/* structure type of compiler */
typedef struct {
    int numProgram;         /* Program counter */

    int szCode;               /* Size of code */
    int szCommand;               /* Size of command */
    int cntErrors;                 /* Error count */

    Symbol** arraySymbols;       /* Array of local symbols */
    int szSymbols;            /* Total number of local symbols */

    Symbol** arrayExternals;    /* Array of external symbols */
    int szExternals;         /* Total number of entry symbols */

    char** arrayEntries;       /* Array of entry symbols */
    int szEntries;           /* Total number of external symbols */

    Templates* tempList;             /* Pointer to templates */
    Template* tempInside;         /* Pointer to template inside */
} Compiler;

/*
 * function name: createCompiler
 * params: 
 * desc: create a new compiler
*/
Compiler* createCompiler();

/*
 * function name: removeCompiler
 * params: 
 *      mycomp: Compiler* (compiler to remove)
 * desc: remove and free compiler
*/
void removeCompiler(Compiler* mycomp);

/*
 * function name: firstCompile
 * params: 
 *      mycomp: Compiler* (compiler to compile)
 *      inputFile: input file name
 *      outputFile: output file name
 *      errorFile: error log file name
 * desc: perform first compiling
*/
void firstCompile(Compiler* mycomp, FILE* inputFile, FILE* outputFile, FILE* errorFile);

/*
 * function name: secondCompile
 * params: 
 *      mycomp: Compiler* (compiler to compile)
 *      outputFile: output file name
 *      objectFile: final object file name
 * desc: perform second compile
*/
void secondCompile(Compiler* mycomp, FILE* outputFile, FILE* objectFile);

/*
 * function name: insertExternal
 * params: 
 *      mycomp: Compiler* (compiler to insert)
 *      ext: external name
 *      addr: address
 * desc: insert external symbol address
*/
void insertExternal(Compiler* mycomp, const char* ext, int addr);

/*
 * function name: insertEntry
 * params: 
 *      mycomp: Compiler* (compiler to insert)
 *      ent: entry name
 * desc: insert entry symbol
*/
void insertEntry(Compiler* mycomp, const char* ent);

/*
 * function name: insertSymbol
 * params: 
 *      mycomp: Compiler* (compiler to insert)
 *      name: symbol name
 * desc: insert symbol
*/
int insertSymbol(Compiler* mycomp, const char* name);

/*
 * function name: searchSymbol
 * params: 
 *      mycomp: Compiler* (compiler to search)
 *      name: symbol name
 * desc: search symbol
*/
Symbol* searchSymbol(Compiler* mycomp, const char* name);


/*
 * function name: compileCommand
 * params: 
 *      mycomp: Compiler* (compiler to insert)
 *      name: symbol name
 * desc: compile command
*/
void compileCommand(Compiler* mycomp, FILE* fileAfterTemplate, FILE* fileErrors, const char* string, int lineNumber);


/* function declarations */

