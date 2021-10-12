
#ifndef PROGRAM_H
#define PROGRAM_H

#include "function.h"
#include "variable.h"
#include "array.h"

struct program {
        unsigned long num_functions;
        struct function ** functions;
        unsigned long num_g_vars;
        struct variable_list ** g_vars;
};

#endif /* PROGRAM_H */

