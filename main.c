
#include "token.h"
#include <stdio.h>
#include <stdlib.h>

#include "parser.h"
#include "converter.h"
#include "myutils.h"
#include "executor.h"
#include "debugger.h"
#include "variable_list.h"

int main(const int argc, const char ** argv) {

        struct token * start_token;
	struct token_a ** tokens;
	struct program * p;
	int i = 0, ret = 1;

	start_token = get_token_list(argv[1]);
	tokens = convert_token_list_to_array(start_token);

	p = preparse_program(tokens);
	if (p) {
		if (parse_program(tokens, p, argc, argv) == 0) {
#ifdef DEBUG
			printf("************************************\n");
			printf("******PROGRAM INSTRUCTIONS**********\n");
			printf("************************************\n");
			debug_program(p);
#endif
			execute_program(p);
		} else {
			fprintf(stderr, "error in parse 2\n");
			goto bail;
		}
	} else {
		fprintf(stderr, "error in parse 1\n");
		goto bail;
	}

	ret = 0;
bail:

	while (tokens[i]->type != END_OF_FILE) {
		free(tokens[i]->value);
		free(tokens[i]);
		i++;
	}

	free(tokens[i]);
	free(tokens);

	for (i = 0; i < p->num_functions; i++) {
		free_function(p->functions[i]);
	}
	if (p->num_functions) {
		free(p->functions);
	}
	for (i = 0; i < p->num_g_vars; i++) {
		if (p->g_vars[i]->type == VL_VAR) {
			free(p->g_vars[i]->v);
		} else if (p->g_vars[i]->type == VL_ARRAY) {
			unsigned long idx;
			for (idx = 0; idx < p->g_vars[i]->v_array->num_vars; idx++) {
				free(p->g_vars[i]->v_array->vars[i]);
			}
			free(p->g_vars[i]->v_array);
		}
		free(p->g_vars[i]);
	}
	if (p->num_g_vars) {
		free(p->g_vars);
	}

	return ret;


}
