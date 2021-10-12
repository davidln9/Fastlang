
#ifndef COMPOSITE_H
#define COMPOSITE_H

#include "variable.h"
#include "function.h"

enum composite_type {
	FUNC, CONSTANT, VAR
};

struct composite {

	enum composite_type type;
	struct variable * var_ptr;
	struct function * func_ptr;
	char * var_name;
};


#endif
