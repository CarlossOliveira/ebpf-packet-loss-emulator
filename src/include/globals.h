#ifndef GLOBALS_H
#define GLOBALS_H

#ifdef APP
#include "../utils/utils.h"

#define BPF_MODULES_DIR "./modules"
#endif

#ifdef BPF
#define TC_ACT_OK 0
#define TC_ACT_SHOT 2

#define DROP 0
#define ACCEPT 2
#endif

#endif // GLOBALS_H