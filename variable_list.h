
#ifndef VARIABLE_LIST_H
#define VARIABLE_LIST_H

#include "variable.h"
#include "array.h"

enum vl_type {
	VL_VAR, VL_ARRAY
};

struct variable_list {
	enum vl_type type;
	struct variable * v;
	struct array * v_array;
	struct variable_list * next;
};

#endif
