#ifndef COMMANDS_H
#define COMMANDS_H

#include "clear.h"
#include "config.h"
#include "exit.h"
#include "help.h"
#include "list.h"
#include "load.h"
#include "stats.h"
#include "unload.h"
#include "set-default.h"
#include "exec.h"

#define HELP_FLAG (input && input[1] && (strcmp(input[1], "--help") == 0 || strcmp(input[1], "-h") == 0))

#endif // !COMMANDS_H
