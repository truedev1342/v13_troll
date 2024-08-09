#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "extra.h"
#include "code.h"
#include "command.h"
#include "compiler.h"
#include "output.h"

/* Allocate memory for a new Compiler structure and initialize its fields */
Compiler* createCompiler() {
    Compiler* mycomp;

    mycomp = malloc(sizeof(*mycomp)); /* Allocate memory for the compiler structure */

    mycomp->tempList = createTemplates(); /* Initialize templates */
    mycomp->tempInside = NULL; /* Initialize tempInside */

    mycomp->numProgram = 100; /* Initialize numProgram */
    mycomp->szCode = 0; /* Initialize szCode */
    mycomp->szCommand = 0; /* Initialize szCommand */
    mycomp->cntErrors = 0; /* Initialize cntErrors */

    mycomp->arrayExternals = NULL; /* Initialize arrayExternals */
    mycomp->szExternals = 0; /* Initialize szExternals */

    mycomp->arrayEntries = NULL; /* Initialize arrayEntries */
    mycomp->szEntries = 0; /* Initialize szEntries */

    mycomp->arraySymbols = NULL; /* Initialize arraySymbols */
    mycomp->szSymbols = 0; /* Initialize szSymbols */

    return mycomp; /* Return the initialized compiler structure */
}

/* Free memory allocated for a Compiler structure and its fields */
void removeCompiler(Compiler* mycomp) {
    int i;

    removeTemplates(mycomp->tempList); /* Free memory allocated for templates */

    /* Free memory allocated for each entry in arrayEntries */
    for (i = 0; i < mycomp->szEntries; i++) {
        free(mycomp->arrayEntries[i]);
    }

    /* Free memory allocated for each external in arrayExternals */
    for (i = 0; i < mycomp->szExternals; i++) {
        free(mycomp->arrayExternals[i]->name);
        free(mycomp->arrayExternals[i]);
    }

    /* Free memory allocated for each local in arraySymbols */
    for (i = 0; i < mycomp->szSymbols; i++) {
        free(mycomp->arraySymbols[i]->name);
        free(mycomp->arraySymbols[i]);
    }

    free(mycomp->arrayEntries); /* Free arrayEntries array */
    free(mycomp->arrayExternals); /* Free arrayExternals array */
    free(mycomp->arraySymbols); /* Free arraySymbols array */
    free(mycomp); /* Free memory allocated for the compiler structure */
}

void confirmEndmacrInFirstCompile(Compiler* mycomp, FILE* inputFile, FILE* outputFile, FILE* errorFile, int commandNum, const char* cmdString) {
    const char* firstCmd;
    if (mycomp->tempInside == NULL) { /* Check if not inside a template */
        fprintf(
            errorFile,
            "on line #%d: not expecting template closure\n",
            commandNum
        );
        mycomp->cntErrors++; /* Increment error count */
    }

    firstCmd = firstPart(cmdString); /* Get the next piece of string */
    cmdString = restString(cmdString); /* Skip to the next piece */

    if (*firstCmd != '\0') { /* Check if piece is not empty */
        fprintf(
            errorFile,
            "on line #%d: not expecting piece \"%s\" after template closure\n",
            commandNum,
            firstCmd
        );
        mycomp->cntErrors++; /* Increment error count */
    }
    mycomp->tempInside = NULL; /* Reset tempInside */
}

void confirmMacrInFirstCompile(Compiler* mycomp, FILE* inputFile, FILE* outputFile, FILE* errorFile, int commandNum, const char* cmdString) {
    const char* firstCmd;
    char templateStr[MAX_N];
    Symbol* label;
    Template* template;
    firstCmd = firstPart(cmdString); /* Get the next piece of string */
    cmdString = restString(cmdString); /* Skip to the next piece */
    strcpy(templateStr, firstCmd); /* Copy piece to stringTemplate */

    firstCmd = firstPart(cmdString); /* Get the next piece of string */
    cmdString = restString(cmdString); /* Skip to the next piece */

    if (*firstCmd != '\0') { /* Check if piece is not empty */
        fprintf(
            errorFile,
            "on line #%d: not expecting piece \"%s\" after template opening\n",
            commandNum,
            firstCmd
        );
        mycomp->cntErrors++; /* Increment error count */
    }

    label = searchSymbol(mycomp, templateStr); /* Search for local label */
    if (label != NULL) { /* Check if label exists */
        fprintf(
            errorFile,
            "on line #%d: name \"%s\" for template already used elsewhere as label\n",
            commandNum,
            templateStr
        );
        mycomp->cntErrors++; /* Increment error count */
        return; /* Continue to next iteration */
    }

    template = findInTemplates(mycomp->tempList, templateStr); /* Search for existing template */
    if (template != NULL) { /* Check if template exists */
        fprintf(
            errorFile,
            "on line #%d: template with name \"%s\" already declared elsewhere\n",
            commandNum,
            templateStr
        );
        mycomp->cntErrors++; /* Increment error count */
        return; /* Continue to next iteration */
    }

    mycomp->tempInside = insertTemplates(mycomp->tempList, templateStr); /* Insert new template */

}

/* Parse the first pass of assembler code */
void firstCompile(Compiler* mycomp, FILE* inputFile, FILE* outputFile, FILE* errorFile) {
    char bufStr[MAX_N];
    char symbolStr[MAX_N];
    char callStr[MAX_N];

    int i;
    int commandNum = 0; /* Initialize commandNum */
    int checkTemplate;

    const char* original;
    const char* firstCmd;
    const char* cmdString;
    Template* template;

    mycomp->cntErrors = 0; /* Initialize cntErrors */
    mycomp->szCode = 0; /* Initialize szCode */
    mycomp->szCommand = 0; /* Initialize szCommand */
    mycomp->numProgram = 100; /* Initialize numProgram */

    /* Read lines from fileAssembler until EOF */
    while (fgets(bufStr, MAX_N, inputFile) != NULL) {
        cmdString = bufStr; /* Initialize string */
        commandNum++; /* Increment commandNum */
        original = cmdString; /* Set base pointer to string */
        
        /* Process labels */
        while (1) {
            firstCmd = firstPart(cmdString); /* Get the next piece of string */
            cmdString = restString(cmdString); /* Skip to the next piece */
            strcpy(symbolStr, firstCmd); /* Copy piece to stringSymbol */

            firstCmd = firstPart(cmdString); /* Get the next piece of string */
            cmdString = restString(cmdString); /* Skip to the next piece */
            
            if (*firstCmd != ':') { /* Check if piece is not a colon */
                break; /* Exit loop */
            }

            original = cmdString; /* Update base pointer */

            if (!insertSymbol(mycomp, symbolStr)) { /* Insert local label */
                fprintf(
                    errorFile,
                    "on line #%d: label name \"%s\" already used elsewhere\n",
                    commandNum,
                    symbolStr
                );
                mycomp->cntErrors++; /* Increment error count */
            }
        }

        cmdString = original; /* Reset string */

        firstCmd = firstPart(cmdString); /* Get the next piece of string */
        cmdString = restString(cmdString); /* Skip to the next piece */

        if (strcmp(firstCmd, "endmacr") == 0) { /* Check for end of template */
            confirmEndmacrInFirstCompile(mycomp, inputFile, outputFile, errorFile, commandNum, cmdString);
            continue; /* Continue to next iteration */
        }

        if (strcmp(firstCmd, "macr") == 0) { /* Check for template definition */
            confirmMacrInFirstCompile(mycomp, inputFile, outputFile, errorFile, commandNum, cmdString);
            continue; /* Continue to next iteration */
        }

        if (mycomp->tempInside != NULL) { /* Check if inside a template */
            insertTemplate(mycomp->tempInside, bufStr); /* Include line in template */
            continue; /* Continue to next iteration */
        }

        cmdString = original; /* Reset string */

        firstCmd = firstPart(cmdString); /* Get the next piece of string */
        cmdString = restString(cmdString); /* Skip to the next piece */
        /* Check if piece is not empty and if not a code command and if not a data command */
        checkTemplate = *firstCmd != '\0' && !checkValidCodeName(firstCmd) && !checkValidCommand(firstCmd);

        if (checkTemplate) { /* Check if calling a template */
            strcpy(callStr, firstCmd); /* Copy piece to stringCall */

            firstCmd = firstPart(cmdString); /* Get the next piece of string */
            cmdString = restString(cmdString); /* Skip to the next piece */

            if (*firstCmd != '\0') { /* Check if piece is not empty */
                fprintf(
                    errorFile,
                    "on line #%d: not expecting piece \"%s\" after template call \"%s\"\n",
                    commandNum,
                    firstCmd,
                    callStr
                );
                mycomp->cntErrors++; /* Increment error count */
            }

            template = findInTemplates(mycomp->tempList, callStr); /* Search for template */
            if (template == NULL) { /* Check if template not found */
                fprintf(
                    errorFile,
                    "on line #%d: template call \"%s\" not found\n",
                    commandNum,
                    callStr
                );
                mycomp->cntErrors++; /* Increment error count */
                continue; /* Continue to next iteration */
            }

            /* Parse each line of the template */
            for (i = 0; i < template->size; i++) {
                const char* template_string = template->arrayCommands[i];
                compileCommand(mycomp, outputFile, errorFile, template_string, commandNum);
            }
        } else {
            compileCommand(mycomp, outputFile, errorFile, bufStr, commandNum); /* Parse current line */
        }
    }
}

/* Second pass of parsing assembler code */
void secondCompile(Compiler* mycomp, FILE* outputFile, FILE* objectFile) {
    char bufStr[MAX_N]; /* Buffer to hold input lines */

    fprintf(objectFile, "%d %d\n", mycomp->szCode, mycomp->szCommand); /* Output sizes of code and data */

    mycomp->numProgram = 100; /* Reset program counter */

    while (fgets(bufStr, MAX_N, outputFile) != NULL) {
        const char* original;
        const char* commandStr;
        const char* firstCmd;

        commandStr = bufStr; /* Initialize string pointer */

        original = commandStr;
        while (1) {
            firstCmd = firstPart(commandStr); /* Take the next piece of string */
            commandStr = restString(commandStr); /* Skip to the next piece */

            firstCmd = firstPart(commandStr); /* Take the next piece of string */
            commandStr = restString(commandStr); /* Skip to the next piece */
            if (*firstCmd != ':') {
                break; /* Exit loop if not a label */
            }

            original = commandStr; /* Update base pointer */
        }

        commandStr = original; /* Reset string pointer */

        firstCmd = firstPart(commandStr); /* Take the next piece of string */

        if (checkValidCodeName(firstCmd)) {
            LineCode_writeToFile(mycomp, commandStr, objectFile); /* Write line of code to file */
            continue; /* Continue to next iteration */
        }

        if (checkValidCommand(firstCmd)) {
            writeCommandToFile(mycomp, commandStr, objectFile); /* Write line of data to file */
            continue; /* Continue to next iteration */
        }
    }

    /* Sort arrayEntries based on label addresses */
    while (1) {
        int i, j, swp = 0;
        for (i = 1; i < mycomp->szEntries; i++) {
            char* entryA = mycomp->arrayEntries[i - 1];
            char* entryB = mycomp->arrayEntries[i - 0];
            Symbol* labelA = NULL;
            Symbol* labelB = NULL;

            for (j = 0; j < mycomp->szSymbols; j++) {
                if (strcmp(mycomp->arraySymbols[j]->name, entryA) == 0) {
                    labelA = mycomp->arraySymbols[j]; /* Find label A in locals */
                }
                if (strcmp(mycomp->arraySymbols[j]->name, entryB) == 0) {
                    labelB = mycomp->arraySymbols[j]; /* Find label B in locals */
                }
            }

            if (labelA != NULL && labelB != NULL) {
                if (labelA->addr > labelB->addr) {
                    mycomp->arrayEntries[i - 0] = entryA; /* Swap entries if necessary */
                    mycomp->arrayEntries[i - 1] = entryB;
                    swp ++; /* Increment swap count */
                }
            }
        }

        if (swp == 0) {
            break; /* Exit loop if no swaps were made */
        }
    }
}

/* Insert a local label if it doesn't already exist */
int insertSymbol(Compiler* mycomp, const char* symbolName) {
    int i;
    Symbol* label;

    /* Check if the local label already exists */
    for (i = 0; i < mycomp->szSymbols; i++) {
        if (strcmp(mycomp->arraySymbols[i]->name, symbolName) == 0) {
            return 0; /* Return 0 if local label exists */
        }
    }

    /* Resize the arraySymbols array if needed */
    mycomp->szSymbols++; /* Increment the count of total locals */
    mycomp->arraySymbols = realloc(mycomp->arraySymbols, sizeof(*mycomp->arraySymbols) * mycomp->szSymbols);

    label = malloc(sizeof(*label)); /* Allocate memory for a new Symbol structure */
    mycomp->arraySymbols[mycomp->szSymbols - 1] = label; /* Store the label in arraySymbols */

    label->name = copyString(symbolName); /* Clone and store the local label name */
    label->addr = mycomp->numProgram; /* Assign the program counter to the local label */

    

    return 1; /* Return 1 indicating successful inserting */
}

/* Search for a local label by name */
Symbol* searchSymbol(Compiler* mycomp, const char* symbolName) {
    int i;
    Symbol* label = NULL;

    /* Iterate through arraySymbols to find the local label */
    for (i = 0; i < mycomp->szSymbols; i++) {
        if (strcmp(mycomp->arraySymbols[i]->name, symbolName) == 0) {
            label = mycomp->arraySymbols[i]; /* Assign the label if found */
            break; /* Exit loop if label is found */
        }
    }

    return label; /* Return the found label or NULL if not found */
}

/* Parse a line of assembler code during the second pass */
void compileCommand(Compiler* mycomp, FILE* fileAfterTemplate, FILE* fileErrors, const char* string, int commandNum) {
    const char* original;
    const char* firstCmd;
    int szCalculated;

    fputs(string, fileAfterTemplate); /* Output the line to fileAfterTemplate */

    original = string;
    while (1) {
        firstCmd = firstPart(string); /* Take the next piece of string */
        string = restString(string); /* Skip to the next piece */

        firstCmd = firstPart(string); /* Take the next piece of string */
        string = restString(string); /* Skip to the next piece */
        if (*firstCmd != ':') {
            break; /* Exit loop if not a label */
        }

        original = string; /* Update base pointer */
    }

    string = original; /* Reset string pointer */

    firstCmd = firstPart(string); /* Take the next piece of string */

    if (checkValidCodeName(firstCmd)) {
        if (!checkCodeFormatValidity(string)) {
            fprintf(fileErrors, "on line #%d: malformed instruction\n", commandNum); /* Output error message */
            mycomp->cntErrors++; /* Increment error count */
            return; /* Return if instruction is malformed */
        }

        szCalculated = checkCodeDataValidity(string); /* Check data size */
        mycomp->numProgram += szCalculated; /* Increment program counter */
        mycomp->szCode += szCalculated; /* Increment code size */
        return;
    }

    if (checkValidCommand(firstCmd)) {
        if (!checkCommandFormat(string)) {
            fprintf(fileErrors, "on line #%d: malformed directive\n", commandNum); /* Output error message */
            mycomp->cntErrors++; /* Increment error count */
            return; /* Return if directive is malformed */
        }

        szCalculated = checkCommandData(string); /* Check data size */
        mycomp->numProgram += szCalculated; /* Increment program counter */
        mycomp->szCommand += szCalculated; /* Increment data size */
        return;
    }
}


/* Insert an external label with its associated local address */
void insertExternal(Compiler* mycomp, const char* ext, int addr) {
    Symbol* label;

    /* reallocate arrayExternals array needs resizing */
    mycomp->szExternals++; /* Increment the count of total externals */
    mycomp->arrayExternals = realloc(mycomp->arrayExternals, sizeof(*mycomp->arrayExternals) * mycomp->szExternals);

    label = malloc(sizeof(*label)); /* Allocate memory for a new Symbol structure */
    mycomp->arrayExternals[mycomp->szExternals - 1] = label; /* Store the label in arrayExternals */

    label->name = copyString(ext); /* Clone and store the external label name */
    label->addr = addr; /* Assign the local address to the external label */
}

/* Insert an entry label if it doesn't already exist */
void insertEntry(Compiler* mycomp, const char* ent) {
    int i;

    /* Check if the entry label already exists */
    for (i = 0; i < mycomp->szEntries; i++) {
        if (strcmp(mycomp->arrayEntries[i], ent) == 0) {
            return; /* Return if the entry label is found */
        }
    }

    /* Resize the arrayEntries array if needed */
    mycomp->szEntries++; /* Increment the count of total entries */
    mycomp->arrayEntries = realloc(mycomp->arrayEntries, sizeof(*mycomp->arrayEntries) * mycomp->szEntries);

    mycomp->arrayEntries[mycomp->szEntries - 1] = copyString(ent); /* Clone and store the entry label */
}
