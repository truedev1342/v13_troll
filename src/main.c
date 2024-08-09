#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "extra.h"
#include "compiler.h"

int main(int argc, char* argv[]) {

    const char* prefix;         /* Prefix for file names */
    char* filePath;                 /* File path */
    FILE* source;               /* Input source file (.as) */
    FILE* postParser;           /* Output file after parsing (.am) */
    FILE* entriesOutput;        /* File for entries (.ent) */
    FILE* errorsOutput;         /* Error output file (stderr) */
    FILE* externalsOutput;      /* File for externals (.ext) */
    FILE* objectOutput;         /* Object code output file (.ob) */
    Compiler* compiler;             /* Parser instance */

    if (argc != 2) {
        /* Print usage info if incorrect arguments */
        fprintf( stderr, "Usage: %s <prefix>\n", argv[0]);
        return -1; /* Exit with error */
    }

    prefix = argv[1]; /* Get prefix from command line */

    compiler = Parser_new(); /* Create new compiler */

    /* Open input source file (.as) */
    filePath = concatString(prefix, ".as");
    source = fopen(filePath, "r");
    free(filePath); /* Free dynamically allocated memory */

    if (source == NULL) {
        /* Print error if source file cannot be opened */
        fprintf( stderr, "Failed to open input file \".as\"!\n");
        return -2; /* Exit with error */
    }

    /* Create post-compiler output file (.am) */
    filePath = concatString(prefix, ".am");
    postParser = fopen(filePath, "w+");
    free(filePath); /* Free dynamically allocated memory */

    errorsOutput = stderr; /* Output errors to stderr */

    /* First pass of the compiler */
    Parser_first(compiler, source, postParser, errorsOutput);

    if (compiler->cntErrors == 0) { /* If parsing completed without errors */

        fseek(postParser, 0, SEEK_SET); /* Reset file position */

        /* Create object code output file (.ob) */
        filePath = concatString(prefix, ".ob");
        objectOutput = fopen(filePath, "w+");
        free(filePath); /* Free dynamically allocated memory */

        /* Second pass of the compiler */
        Parser_second(compiler, postParser, objectOutput);
        fclose(objectOutput); /* Close object code file */

        if (compiler->szEntries > 0) { /* If there are entries to output */

            /* Create entry output file (.ent) */
            filePath = concatString(prefix, ".ent");
            entriesOutput = fopen(filePath, "w+");
            free(filePath); /* Free dynamically allocated memory */

            Parser_fileOutputEntries(compiler, entriesOutput);
            fclose(entriesOutput); /* Close entry output file */

        }

        if (compiler->szExternals > 0) { /* If there are externals to output */

            /* Create external output file (.ext) */
            filePath = concatString(prefix, ".ext");
            externalsOutput = fopen(filePath, "w+");
            free(filePath); /* Free dynamically allocated memory */

            Parser_fileOutputExternals(compiler, externalsOutput);
            fclose(externalsOutput); /* Close external output file */

        }

    }

    fclose(postParser); /* Close post-compiler file */
    fclose(source); /* Close source file */

    Parser_delete(compiler); /* Delete compiler instance */

    return 0; /* Exit program */
}