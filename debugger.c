
#ifdef DEBUG

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#include "debugger.h"
#include "stmt.h"
#include "program.h"

static int tab_level;
static struct stmt * curr;

static void dp(const char * fmt, ...) {

	va_list param;
	va_start(param, fmt);

	char output[1024] = {0};
	vsnprintf(output, sizeof(output), fmt, param);

	va_end(param);
	if (tab_level) {
		char * tabstops = malloc(tab_level + 1);

		memset(tabstops, 0, tab_level);
		int i;
		for (i = 0; i < tab_level; i++) {
			tabstops[i] = '\t';
		}
		printf("%p%s%s\n", curr, tabstops, output);
		free(tabstops);
		return;
	}
	printf("%p%s\n", curr, output);
}

		
static char* get_var_type(enum var_type type) {
	switch (type) {
		case INT_VAR:
			return "int";
		case FLOAT_VAR:
			return "float";
		case STRING_VAR:
			return "string";
		case CHAR_VAR:
			return "char";
		default:
			return "<unknown>";
	}
}

void debug_program(struct program *p) {

	int i;
	for (i = 0; i < p->num_functions; i++) {
		debug(p->functions[i]);
	}
}


int debug(struct function * f) {

	tab_level = 0;
	curr = f->start;

	while (curr) {

		switch (curr->type) {
			case NOOP_STMT:
				dp("NOOP");
				break;
			case GOTO_STMT:
				dp("GOTO %p", curr->alt);
				break;
			case VAR_DEC:
				dp("VAR_DEC: %s %s", get_var_type(curr->var->type), curr->var->name);
				break;
			case PUSH_SCOPE:
				tab_level++;
				dp("PUSH_SCOPE");
				break;
			case POP_SCOPE:
				dp("POP_SCOPE");
				tab_level--;
				break;
			case ASSIGNMENT:
				dp("ASSIGNMENT: %s", curr->assign_name);
				break;
			case IF_STMT:
				dp("IF_STMT  (alt)-> %p", curr->alt);
				break;
			case PRINT_STMT:
				dp("PRINT_STMT");
				break;
			case PRINTLN_STMT:
				dp("PRINTLN_STMT");
				break;
			case EPRINT_STMT:
				dp("EPRINT_STMT");
				break;
			case EPRINTLN_STMT:
				dp("EPRINTLN_STMT");
				break;
			case EXEC_EXPR:
				dp("EXEC_EXPR: %d", curr->num_expr_stmts);
				break;
			case RETURN_STMT:
				dp("RETURN_STMT");
				break;
			case ARRAY_DEC:
				dp("ARRAY_DEC: %s", curr->array->name);
				break;
			default:
				fprintf(stderr, "%s: UNKNOWN STMT\n", __func__);
				return 1;
		}
		curr = curr->next;
	}
	return 0;
}

#endif
