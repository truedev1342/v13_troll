#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "extra.h"
#include "compiler.h"
#include "output.h"

int main(int argc, char* argv[]) {

    /* Prefix for file names */
    const char* filenameStr;
    /* Input source file name (.as) */
    FILE* inputFile;
    /* Output file name after parsing (.am) */
    FILE* outputFile;
    /* output file for entries (.ent) */
    FILE* entOutputFile;
    /* Error log output file (stderr) */
    FILE* errorLogFile;
    /* output file for externals (.ext) */
    FILE* extOutputFile;
    /* output file name for object code (.ob) */
    FILE* objOutputFile;
    /* compiler for compile code */
    Compiler* mycomp;

    /* full file name str*/
    char* fullFilePathStr;
    if (argc == 2) {
        /* get filenameStr from parameter */
        filenameStr = argv[1];

        /* initializing mycomp compiler*/
        mycomp = createCompiler();

        /* Open input source file (.as) */
        fullFilePathStr = concatString(filenameStr, ".as");
        inputFile = fopen(fullFilePathStr, "r");
        free(fullFilePathStr);

        if (inputFile == NULL) {
            /* Print error if source file cannot be opened */
            fprintf( stderr, "Failed to open input file \".as\"!\n");
            return -2; /* Exit with error */
        }

        errorLogFile = stderr; /* Output errors to stderr */

        /* Create post-compiler output file (.am) */
        fullFilePathStr = concatString(filenameStr, ".am");
        outputFile = fopen(fullFilePathStr, "w+");
        free(fullFilePathStr);

        /* confirm first compiling with mycomp*/
        firstCompile(mycomp, inputFile, outputFile, errorLogFile);
        /* when compiling is complete with no error, we will continue ...*/
        if (mycomp->cntErrors == 0) {

            /* reset file position to the other */
            fseek(outputFile, 0, SEEK_SET);

            /* Create output file and open(.ob) */
            fullFilePathStr = concatString(filenameStr, ".ob");
            objOutputFile = fopen(fullFilePathStr, "w+");
            free(fullFilePathStr);

            /* confirm second compiling with mycomp */
            secondCompile(mycomp, outputFile, objOutputFile);
            /* close object output file when complete */
            fclose(objOutputFile); 
            
            if (mycomp->szExternals != 0) {
                /* Create external output file (.ext) */
                fullFilePathStr = concatString(filenameStr, ".ext");
                extOutputFile = fopen(fullFilePathStr, "w+");
                free(fullFilePathStr);
                writeExternalsToFile(mycomp, extOutputFile);
                fclose(extOutputFile); /* Close external output file */
            }
            /* output to file when there are entries to output */
            if (mycomp->szEntries != 0) { 
                /* Create entry output file and open (.ent) */
                fullFilePathStr = concatString(filenameStr, ".ent");
                entOutputFile = fopen(fullFilePathStr, "w+");
                free(fullFilePathStr);
                writeEntriesToFile(mycomp, entOutputFile);
                /* Close entry output file */
                fclose(entOutputFile); 
            }
        }
        /* remove mycomp compiler and free memory */
        removeCompiler(mycomp);
        /* close input file*/
        fclose(inputFile);
        /* close output file*/
        fclose(outputFile);
    } else {
        /* Print error message when input incorrect parameter */
        fprintf( stderr, "Usage: %s <prefix>\n", argv[0]);
        return -1; /* Exit with error */
    }
    /* exit with normally*/
    return 0;
}