
#ifndef STMT_H
#define STMT_H

#include "variable.h"
#include "function.h"
#include "composite.h"
#include "label.h"
#include "array.h"


enum stmt_type {
	PRINT_STMT, IF_STMT, GOTO_STMT, NOOP_STMT, PRINTLN_STMT, EPRINT_STMT, EPRINTLN_STMT, ASSIGNMENT, PUSH_STMT, 
	EXEC_EXPR, VAR_DEC, PUSH_SCOPE, POP_SCOPE, RETURN_STMT, ARRAY_DEC, ARRAY_PREP, APPEND_STMT, POP_STMT, GET_STMT,
	INSERT_STMT, REMOVE_STMT
};

struct stmt {
	enum stmt_type type;
	struct variable * var;
	struct stmt * next;
	struct function * function_call;
	struct composite ** expr_stmts;
	int num_expr_stmts;
	struct stmt * alt;
	char * assign_name;
	struct array * array;
};


#endif

