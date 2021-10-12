
#ifndef FUNCTION_H
#define FUNCTION_H

#include "variable.h"
#include "stmt.h"
#include "label.h"


enum func_type {
	MATH_FUNC, USER_DEFINED, LPAREN_PLACEHOLDER, LBRACKET_PLACEHOLDER, ARRAY_FUNC, LEN_FUNC
};

enum math_func {
	POWER_FUNC, ADD_FUNC, SUB_FUNC, MULT_FUNC, DIV_FUNC, GTEQ_FUNC, GT_FUNC, LTEQ_FUNC, LT_FUNC,
	EQEQ_FUNC, NEQ_FUNC
};

struct scope {
	struct variable_list * vars_start;
	struct variable ** vars;
	int num_vars;
	struct scope * next;
};

struct function {
	enum func_type ftype;
	enum math_func mtype;
	enum var_type type;
	struct stmt * start;
	int num_args;
	struct variable_list ** args;
	char * name;
	struct scope * scopes_start;
	struct label * labels_start;
};

struct function_list {
	struct function * f;
	struct function_list * next;
};

#endif
