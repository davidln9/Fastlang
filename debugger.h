
#ifdef DEBUG

#ifndef DEBUG_H
#define DEBUG_H

#include "variable.h"
#include "function.h"
#include "program.h"

int debug(struct function *f);
void debug_program(struct program *p);

#endif /* DEBUG_H */

#endif /* DEBUG */
