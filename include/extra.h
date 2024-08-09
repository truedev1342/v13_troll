#pragma once

/* Maximum size of data array */
#define MAX_N (1024)

/* increase value for vector operations */
#define INCREASE_VALUE (24)

/*  append a suffix to a phrase */
char* concatString(const char* origin, const char* suffix);

/*  clone a given origin */
char* copyString(const char* origin);

/*  skip whitespace and return the rest of the origin */
const char* restString(const char* origin);

/*  take the first part of the phrase until whitespace */
const char* firstPart(const char* string);

/*  convert a phrase to an integer */
int parseInt(const char* string);
