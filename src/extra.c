
#include <ctype.h>
#include <stdlib.h>
#include <string.h>

#include "extra.h"

/* Creates a copy of the origin string */
char* copyString(const char* origin) {
    char* result;
    int i;
    int sz = strlen(origin);
    result = malloc(sz + 1);   /* Allocate memory for the result */
    for (i = 0; i < sz; i ++) {
        result[i] = origin[i];
    }
    result[sz] = '\0';
    return result;                         /* Return the result string */
}

/* Concatenates two strings and returns the result */
char* concatString(const char* origin, const char* suffix) {
    char* result;
    int i;
    int sz_o = strlen(origin);
    int sz_s = strlen(suffix);
    result = malloc(sz_o + sz_s + 1);   /* Allocate memory for the result */

    for (i = 0; i < sz_o; i ++) {
        result[i] = origin[i];
    }
    for (i = 0; i < sz_s; i ++) {
        result[i + sz_o] = suffix[i];
    }

    result[sz_s + sz_o] = '\0';                  /* Terminate the concatenated string */

    return result;              /* Return the concatenated string */
}

/* Converts a string to an integer */
int parseInt(const char* origin) {
    int result = 0, sign = 1, sz = strlen(origin), i;
    for (i = 0; i < sz; i ++) {
        if (origin[i] == '-') {
            sign = -1;
        }
        if (isdigit(origin[i])) {
            result = result * 10 + origin[i] - '0';
        }
    }
    return result * sign;
}

/* Checks if a character is a separator */
static int checkSeperator(char chr) {
    return (chr == '\0') | (chr == ',') | (chr == ';') | (chr == ':') | isspace(chr);
}

/* Returns the first segment of the string until a separator */
const char* firstPart(const char* origin) {
    char chr;
    int sz = 0;
    char* result;
    while (isspace(*origin)) {
        origin++;
    }

    chr = *origin;
    if (chr == ';') {
        chr = '\0';
    }
    if (checkSeperator(chr)) {
        result = malloc(2);
        result[0] = chr;
        result[1] = '\0';
        return result;                         /* Return result if separator */
    }
    result = malloc(strlen(origin) + 1);
    for (chr = *origin; !checkSeperator(chr); origin ++, chr = *origin) {
        result[sz ++] = chr;
    }
    result[sz] = '\0'; /* Terminate result */
    return result;                             /* Return result */
}

/* Skips separators and whitespace in the string */
const char* restString(const char* origin) {
    char chr;
    chr = *origin;
    if (chr == ',' || chr == ':') {
        return origin + 1;                     /* Skip ',' or ':' */
    }
    while (isspace(*origin)) {
        origin++;
    }
    while (1) {
        chr = *origin;
        if (chr == ';' || chr == '\0') {
            return origin;                     /* Return string on separator or end */
        }

        if (checkSeperator(chr)) {
            break;                             /* Break loop on separator */
        }
        origin++;
    }
    return origin;                             /* Return string */
}


