
#ifndef ARRAY_H
#define ARRAY_H

#include "variable.h"

struct array {

	enum var_type type;
	struct variable ** vars;
	unsigned long num_vars;
	unsigned long total_vars;
	char * name;
};

#endif
