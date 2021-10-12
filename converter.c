
#include <assert.h>
#include "token.h"
#include <stdlib.h>
#include "variable.h"
#include "variable_list.h"
#include "function.h"


struct variable ** convert_vl_to_array(struct variable_list * start, unsigned long * num_vars_ptr) {

	struct variable ** ret = NULL;
	unsigned long num_vars = 0, index = 0;
	struct variable_list * curr = start;
	while (curr) {
		assert(curr->type == VL_VAR);
		curr = curr->next;
		num_vars++;
	}

	ret = malloc(sizeof(struct variable*)*num_vars);
	
	curr = start;
	while (index < num_vars) {
		ret[index] = curr->v;
		curr = curr->next;
		index++;
	}
	assert(curr == NULL);

	*num_vars_ptr = num_vars;
	return ret;
}

struct token_a ** convert_token_list_to_array(struct token * start) {


	struct token * curr = start, *tmp;
	int num_tokens = 0, index;
	struct token_a ** tokens;

	while (curr) {
		curr = curr->next;
		num_tokens++;
	}

	tokens = malloc(sizeof(struct token_a*)*num_tokens+1);

	curr = start;
	for (index = 0; index < num_tokens; index++) {
		tokens[index] = malloc(sizeof(struct token_a));
		tokens[index]->value = curr->value;
		tokens[index]->type = curr->type;
		tmp = curr;
		curr = curr->next;
		free(tmp);
	}
	tokens[num_tokens] = malloc(sizeof(struct token_a));
	tokens[num_tokens]->type = END_OF_FILE;

	return tokens;
}

struct variable_list ** convert_variable_list_to_array(struct variable_list * start, unsigned long * num_vars_ptr) {

	struct variable_list ** ret = NULL;
	unsigned long num_vars = 0, index;
	struct variable_list * curr = start;

	while (curr) {
		num_vars++;
		curr = curr->next;
	}

	ret = malloc(sizeof(struct variable_list*)*num_vars);
	curr = start;

	index = 0;
	while (index < num_vars) {
		ret[index] = curr;
		index++;
		curr = curr->next;
	}

	*num_vars_ptr = num_vars;
	return ret;
}

struct function ** convert_function_list_to_array(struct function_list * start, unsigned long * num_funcs_ptr) {

	struct function ** ret = NULL;
	unsigned long num_funcs = 0, index;
	struct function_list * curr = start, *tmp;

	while (curr) {
		curr = curr->next;
		num_funcs++;
	}

	ret = malloc(sizeof(struct function*)*num_funcs);
	curr = start;

	for (index = 0; index < num_funcs; index++) {
		ret[index] = curr->f;
		tmp = curr;
		curr = curr->next;
		free(tmp);
	}

	*num_funcs_ptr = num_funcs;
	return ret;
}



