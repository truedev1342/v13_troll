#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "extra.h"
#include "code.h"
#include "command.h"
#include "compiler.h"

/* Static function declarations */
static int Parser_recordSymbol(Compiler* compiler, const char* name);
static Symbol* Parser_searchSymbol(Compiler* compiler, const char* name);
static void Parser_parseLine(Compiler* compiler, FILE* fileAfterTemplate, FILE* fileErrors, const char* string, int lineNumber);

/* Allocate memory for a new Compiler structure and initialize its fields */
Compiler* Parser_new() {
    Compiler* compiler;

    compiler = malloc(sizeof(*compiler)); /* Allocate memory for the compiler structure */

    compiler->templates = createTemplates(); /* Initialize templates */
    compiler->templateInside = NULL; /* Initialize templateInside */

    compiler->programCounter = 100; /* Initialize programCounter */
    compiler->szCode = 0; /* Initialize szCode */
    compiler->szCommand = 0; /* Initialize szCommand */
    compiler->cntErrors = 0; /* Initialize cntErrors */

    compiler->arrayExternals = NULL; /* Initialize arrayExternals */
    compiler->szExternals = 0; /* Initialize szExternals */

    compiler->arrayEntries = NULL; /* Initialize arrayEntries */
    compiler->szEntries = 0; /* Initialize szEntries */

    compiler->arraySymbols = NULL; /* Initialize arraySymbols */
    compiler->szSymbols = 0; /* Initialize szSymbols */

    return compiler; /* Return the initialized compiler structure */
}

/* Free memory allocated for a Compiler structure and its fields */
void Parser_delete(Compiler* compiler) {
    int place;

    removeTemplates(compiler->templates); /* Free memory allocated for templates */

    /* Free memory allocated for each entry in arrayEntries */
    for (place = 0; place < compiler->szEntries; place++) {
        free(compiler->arrayEntries[place]);
    }

    /* Free memory allocated for each external in arrayExternals */
    for (place = 0; place < compiler->szExternals; place++) {
        free(compiler->arrayExternals[place]->name);
        free(compiler->arrayExternals[place]);
    }

    /* Free memory allocated for each local in arraySymbols */
    for (place = 0; place < compiler->szSymbols; place++) {
        free(compiler->arraySymbols[place]->name);
        free(compiler->arraySymbols[place]);
    }

    free(compiler->arrayEntries); /* Free arrayEntries array */
    free(compiler->arrayExternals); /* Free arrayExternals array */
    free(compiler->arraySymbols); /* Free arraySymbols array */
    free(compiler); /* Free memory allocated for the compiler structure */
}

/* Parse the first pass of assembler code */
void Parser_first(Compiler* compiler, FILE* fileAssembler, FILE* fileAfterTemplate, FILE* fileErrors) {
    char buffer[MAX_N];
    char stringTemplate[MAX_N];
    char stringSymbol[MAX_N];
    char stringCall[MAX_N];

    int place;
    int lineNumber;
    int callTemplate;

    const char* base;
    const char* piece;
    const char* string;

    Symbol* label;
    Template* template;

    lineNumber = 0; /* Initialize lineNumber */

    compiler->szCode = 0; /* Initialize szCode */
    compiler->szCommand = 0; /* Initialize szCommand */
    compiler->cntErrors = 0; /* Initialize cntErrors */

    compiler->programCounter = 100; /* Initialize programCounter */

    /* Read lines from fileAssembler until EOF */
    while (fgets(buffer, MAX_N, fileAssembler) != NULL) {
        string = buffer; /* Initialize string */

        lineNumber++; /* Increment lineNumber */

        base = string; /* Set base pointer to string */
        
        /* Process labels */
        while (1) {
            piece = firstPart(string); /* Get the next piece of string */
            string = restString(string); /* Skip to the next piece */
            strcpy(stringSymbol, piece); /* Copy piece to stringSymbol */

            piece = firstPart(string); /* Get the next piece of string */
            string = restString(string); /* Skip to the next piece */
            
            if (*piece != ':') { /* Check if piece is not a colon */
                break; /* Exit loop */
            }

            base = string; /* Update base pointer */

            if (!Parser_recordSymbol(compiler, stringSymbol)) { /* Record local label */
                fprintf(
                    fileErrors,
                    "on line #%d: label name \"%s\" already used elsewhere\n",
                    lineNumber,
                    stringSymbol
                );
                compiler->cntErrors++; /* Increment error count */
            }
        }

        string = base; /* Reset string */

        piece = firstPart(string); /* Get the next piece of string */
        string = restString(string); /* Skip to the next piece */

        if (strcmp(piece, "endmacr") == 0) { /* Check for end of template */
            if (compiler->templateInside == NULL) { /* Check if not inside a template */
                fprintf(
                    fileErrors,
                    "on line #%d: not expecting template closure\n",
                    lineNumber
                );
                compiler->cntErrors++; /* Increment error count */
            }

            piece = firstPart(string); /* Get the next piece of string */
            string = restString(string); /* Skip to the next piece */

            if (*piece != '\0') { /* Check if piece is not empty */
                fprintf(
                    fileErrors,
                    "on line #%d: not expecting piece \"%s\" after template closure\n",
                    lineNumber,
                    piece
                );
                compiler->cntErrors++; /* Increment error count */
            }

            compiler->templateInside = NULL; /* Reset templateInside */
            continue; /* Continue to next iteration */
        }

        if (strcmp(piece, "macr") == 0) { /* Check for template definition */
            piece = firstPart(string); /* Get the next piece of string */
            string = restString(string); /* Skip to the next piece */
            strcpy(stringTemplate, piece); /* Copy piece to stringTemplate */

            piece = firstPart(string); /* Get the next piece of string */
            string = restString(string); /* Skip to the next piece */

            if (*piece != '\0') { /* Check if piece is not empty */
                fprintf(
                    fileErrors,
                    "on line #%d: not expecting piece \"%s\" after template opening\n",
                    lineNumber,
                    piece
                );
                compiler->cntErrors++; /* Increment error count */
            }

            label = Parser_searchSymbol(compiler, stringTemplate); /* Search for local label */
            if (label != NULL) { /* Check if label exists */
                fprintf(
                    fileErrors,
                    "on line #%d: name \"%s\" for template already used elsewhere as label\n",
                    lineNumber,
                    stringTemplate
                );
                compiler->cntErrors++; /* Increment error count */
                continue; /* Continue to next iteration */
            }

            template = findInTemplates(compiler->templates, stringTemplate); /* Search for existing template */
            if (template != NULL) { /* Check if template exists */
                fprintf(
                    fileErrors,
                    "on line #%d: template with name \"%s\" already declared elsewhere\n",
                    lineNumber,
                    stringTemplate
                );
                compiler->cntErrors++; /* Increment error count */
                continue; /* Continue to next iteration */
            }

            compiler->templateInside = recordTemplates(compiler->templates, stringTemplate); /* Record new template */
            continue; /* Continue to next iteration */
        }

        if (compiler->templateInside != NULL) { /* Check if inside a template */
            insertTemplate(compiler->templateInside, buffer); /* Include line in template */
            continue; /* Continue to next iteration */
        }

        string = base; /* Reset string */

        piece = firstPart(string); /* Get the next piece of string */
        string = restString(string); /* Skip to the next piece */

        callTemplate = *piece != '\0'; /* Check if piece is not empty */
        callTemplate = callTemplate && !LineCode_checkName(piece); /* Check if not a code line */
        callTemplate = callTemplate && !checkValidCommand(piece); /* Check if not a data line */

        if (callTemplate) { /* Check if calling a template */
            strcpy(stringCall, piece); /* Copy piece to stringCall */

            piece = firstPart(string); /* Get the next piece of string */
            string = restString(string); /* Skip to the next piece */

            if (*piece != '\0') { /* Check if piece is not empty */
                fprintf(
                    fileErrors,
                    "on line #%d: not expecting piece \"%s\" after template call \"%s\"\n",
                    lineNumber,
                    piece,
                    stringCall
                );
                compiler->cntErrors++; /* Increment error count */
            }

            template = findInTemplates(compiler->templates, stringCall); /* Search for template */
            if (template == NULL) { /* Check if template not found */
                fprintf(
                    fileErrors,
                    "on line #%d: template call \"%s\" not found\n",
                    lineNumber,
                    stringCall
                );
                compiler->cntErrors++; /* Increment error count */
                continue; /* Continue to next iteration */
            }

            /* Parse each line of the template */
            for (place = 0; place < template->size; place++) {
                const char* template_string = template->arrayCommands[place];
                Parser_parseLine(compiler, fileAfterTemplate, fileErrors, template_string, lineNumber);
            }
        } else {
            Parser_parseLine(compiler, fileAfterTemplate, fileErrors, buffer, lineNumber); /* Parse current line */
        }
    }
}

/* Record an external label with its associated local address */
void Parser_recordExternal(Compiler* compiler, const char* ext, int local) {
    Symbol* label;

    /* reallocate arrayExternals array needs resizing */
    compiler->szExternals++; /* Increment the count of total externals */
    compiler->arrayExternals = realloc(compiler->arrayExternals, sizeof(*compiler->arrayExternals) * compiler->szExternals);

    label = malloc(sizeof(*label)); /* Allocate memory for a new Symbol structure */
    compiler->arrayExternals[compiler->szExternals - 1] = label; /* Store the label in arrayExternals */

    label->name = copyString(ext); /* Clone and store the external label name */
    label->addr = local; /* Assign the local address to the external label */

    
}

/* Record an entry label if it doesn't already exist */
void Parser_recordEntry(Compiler* compiler, const char* entry) {
    int place;

    /* Check if the entry label already exists */
    for (place = 0; place < compiler->szEntries; place++) {
        if (strcmp(compiler->arrayEntries[place], entry) == 0) {
            return; /* Return if the entry label is found */
        }
    }

    /* Resize the arrayEntries array if needed */
    compiler->szEntries++; /* Increment the count of total entries */
    compiler->arrayEntries = realloc(compiler->arrayEntries, sizeof(*compiler->arrayEntries) * compiler->szEntries);

    compiler->arrayEntries[compiler->szEntries - 1] = copyString(entry); /* Clone and store the entry label */
    
}

/* Output object code to a file */
void Parser_fileOutputObject(Compiler* compiler, FILE* file, int valueData) {
    fprintf(file, "%04d %05o\n", compiler->programCounter, 0x7FFF & valueData); /* Print formatted output to file */

    compiler->programCounter++; /* Increment the program counter */
}

/* Second pass of parsing assembler code */
void Parser_second(Compiler* compiler, FILE* fileAfterTemplate, FILE* fileObject) {
    char buffer[MAX_N]; /* Buffer to hold input lines */

    fprintf(fileObject, "%d %d\n", compiler->szCode, compiler->szCommand); /* Output sizes of code and data */

    compiler->programCounter = 100; /* Reset program counter */

    while (fgets(buffer, MAX_N, fileAfterTemplate) != NULL) {
        const char* base;
        const char* string;
        const char* piece;

        string = buffer; /* Initialize string pointer */

        base = string;
        while (1) {
            piece = firstPart(string); /* Take the next piece of string */
            string = restString(string); /* Skip to the next piece */

            piece = firstPart(string); /* Take the next piece of string */
            string = restString(string); /* Skip to the next piece */
            if (*piece != ':') {
                break; /* Exit loop if not a label */
            }

            base = string; /* Update base pointer */
        }

        string = base; /* Reset string pointer */

        piece = firstPart(string); /* Take the next piece of string */

        if (LineCode_checkName(piece)) {
            LineCode_writeToFile(compiler, string, fileObject); /* Write line of code to file */
            continue; /* Continue to next iteration */
        }

        if (checkValidCommand(piece)) {
            writeCommandToFile(compiler, string, fileObject); /* Write line of data to file */
            continue; /* Continue to next iteration */
        }
    }

    /* Sort arrayEntries based on label addresses */
    while (1) {
        int placeA;
        int placeB;
        int swaps;

        swaps = 0;
        for (placeA = 1; placeA < compiler->szEntries; placeA++) {
            char* entryA = compiler->arrayEntries[placeA - 1];
            char* entryB = compiler->arrayEntries[placeA - 0];
            Symbol* labelA = NULL;
            Symbol* labelB = NULL;

            for (placeB = 0; placeB < compiler->szSymbols; placeB++) {
                if (strcmp(compiler->arraySymbols[placeB]->name, entryA) == 0) {
                    labelA = compiler->arraySymbols[placeB]; /* Find label A in locals */
                }
                if (strcmp(compiler->arraySymbols[placeB]->name, entryB) == 0) {
                    labelB = compiler->arraySymbols[placeB]; /* Find label B in locals */
                }
            }

            if (labelA != NULL && labelB != NULL) {
                if (labelA->addr > labelB->addr) {
                    compiler->arrayEntries[placeA - 0] = entryA; /* Swap entries if necessary */
                    compiler->arrayEntries[placeA - 1] = entryB;
                    swaps++; /* Increment swap count */
                }
            }
        }

        if (swaps == 0) {
            break; /* Exit loop if no swaps were made */
        }
    }
}

/* Output external labels to a file */
void Parser_fileOutputExternals(Compiler* compiler, FILE* fileExternals) {
    int place;
    Symbol* label;

    for (place = 0; place < compiler->szExternals; place++) {
        label = compiler->arrayExternals[place]; /* Get external label from vector */
        fprintf(fileExternals, "%s 0%d\n", label->name, label->addr); /* Print external label info to file */
    }
}

/* Output entry labels and their addresses to a file */
void Parser_fileOutputEntries(Compiler* compiler, FILE* fileEntries) {
    const char* name;
    int placeA;
    int placeB;
    Symbol* label;

    for (placeA = 0; placeA < compiler->szEntries; placeA++) {
        name = compiler->arrayEntries[placeA]; /* Get entry name from vector */
        for (placeB = 0; placeB < compiler->szSymbols; placeB++) {
            label = compiler->arraySymbols[placeB]; /* Get local label from vector */
            if (strcmp(label->name, name) == 0) {
                fprintf(fileEntries, "%s %d\n", label->name, label->addr); /* Print entry info to file */
            }
        }
    }
}

/* Record a local label if it doesn't already exist */
static int Parser_recordSymbol(Compiler* compiler, const char* name) {
    int place;
    Symbol* label;

    /* Check if the local label already exists */
    for (place = 0; place < compiler->szSymbols; place++) {
        if (strcmp(compiler->arraySymbols[place]->name, name) == 0) {
            return 0; /* Return 0 if local label exists */
        }
    }

    /* Resize the arraySymbols array if needed */
    compiler->szSymbols++; /* Increment the count of total locals */
    compiler->arraySymbols = realloc(compiler->arraySymbols, sizeof(*compiler->arraySymbols) * compiler->szSymbols);

    label = malloc(sizeof(*label)); /* Allocate memory for a new Symbol structure */
    compiler->arraySymbols[compiler->szSymbols - 1] = label; /* Store the label in arraySymbols */

    label->name = copyString(name); /* Clone and store the local label name */
    label->addr = compiler->programCounter; /* Assign the program counter to the local label */

    

    return 1; /* Return 1 indicating successful recording */
}

/* Search for a local label by name */
static Symbol* Parser_searchSymbol(Compiler* compiler, const char* name) {
    int place;
    Symbol* label = NULL;

    /* Iterate through arraySymbols to find the local label */
    for (place = 0; place < compiler->szSymbols; place++) {
        if (strcmp(compiler->arraySymbols[place]->name, name) == 0) {
            label = compiler->arraySymbols[place]; /* Assign the label if found */
            break; /* Exit loop if label is found */
        }
    }

    return label; /* Return the found label or NULL if not found */
}

/* Parse a line of assembler code during the second pass */
static void Parser_parseLine(Compiler* compiler, FILE* fileAfterTemplate, FILE* fileErrors, const char* string, int lineNumber) {
    const char* base;
    const char* piece;
    int calculatedSize;

    fputs(string, fileAfterTemplate); /* Output the line to fileAfterTemplate */

    base = string;
    while (1) {
        piece = firstPart(string); /* Take the next piece of string */
        string = restString(string); /* Skip to the next piece */

        piece = firstPart(string); /* Take the next piece of string */
        string = restString(string); /* Skip to the next piece */
        if (*piece != ':') {
            break; /* Exit loop if not a label */
        }

        base = string; /* Update base pointer */
    }

    string = base; /* Reset string pointer */

    piece = firstPart(string); /* Take the next piece of string */

    if (LineCode_checkName(piece)) {
        if (!LineCode_checkForm(string)) {
            fprintf(fileErrors, "on line #%d: malformed instruction\n", lineNumber); /* Output error message */
            compiler->cntErrors++; /* Increment error count */
            return; /* Return if instruction is malformed */
        }

        calculatedSize = LineCode_checkData(string); /* Check data size */
        compiler->programCounter += calculatedSize; /* Increment program counter */
        compiler->szCode += calculatedSize; /* Increment code size */
        return;
    }

    if (checkValidCommand(piece)) {
        if (!checkCommandFormat(string)) {
            fprintf(fileErrors, "on line #%d: malformed directive\n", lineNumber); /* Output error message */
            compiler->cntErrors++; /* Increment error count */
            return; /* Return if directive is malformed */
        }

        calculatedSize = checkCommandData(string); /* Check data size */
        compiler->programCounter += calculatedSize; /* Increment program counter */
        compiler->szCommand += calculatedSize; /* Increment data size */
        return;
    }
}
