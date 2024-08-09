#pragma once

#include "compiler.h"

/* check given command name is valid */
int checkValidCommand(const char* command);

/* check given command format is valid */
int checkCommandFormat(const char* command);

/* check given command data is valid */
int checkCommandData(const char* command);
