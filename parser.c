
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>

#include "token.h"
#include "variable.h"
#include "function.h"
#include "composite.h"
#include "program.h"
#include "converter.h"
#include "variable_list.h"

#ifdef DEBUG
#include "debugger.h"
#endif

static struct token_a ** g_tokens = NULL;
static int c_token = 0, g_num_functions;

static struct function ** g_functions = NULL;

static int parse_stmt(struct function * f);
static int parse_function_call(struct function * f);
static int g_parse;

static struct function_list * g_funcs = NULL;
static struct composite ** output_queue = NULL;
static struct function ** operator_stack = NULL;
static struct token_a ** operator_stack_preparse = NULL;
static int oq_len = 0, op_height = 0, op_height_preparse;
static int total_oq_len = 0, total_op_height = 0, total_op_height_preparse = 0;
static struct variable_list * g_vars_start = NULL;


static void set_tokens(struct token_a ** tokens) {
	g_tokens = tokens;
	c_token = 0;
}

static void add_function(struct function * f) {

	if (!g_funcs) {
		g_funcs = malloc(sizeof(struct function_list));
		g_funcs->next = NULL;
		g_funcs->f = f;
		return;
	}

	struct function_list * curr = g_funcs, *prev;

	while (curr) {
		prev = curr;
		curr = curr->next;
	}
	curr = prev;

	curr->next = malloc(sizeof(struct function_list));
	curr = curr->next;
	curr->next = NULL;
	curr->f = f;
}


static void add_global_var(struct variable * v) {

	assert(g_parse);
	if (g_vars_start == NULL) {
	       g_vars_start = malloc(sizeof(struct variable_list));
	       g_vars_start->next = NULL;
	       g_vars_start->v = v;
	       g_vars_start->type = VL_VAR;
	       return;
	}
	struct variable_list * curr = g_vars_start, *prev;

	while (curr) {
		prev = curr;
		curr = curr->next;
	}
	curr = prev;
	curr->next = malloc(sizeof(struct variable_list));
	curr = curr->next;
	curr->next = NULL;
	curr->v = v;
	curr->type = VL_VAR;
}

static void add_array_g(struct array * array) {
	assert(g_parse);

	if (g_vars_start == NULL) {
		g_vars_start = malloc(sizeof(struct variable_list));
		g_vars_start->next = NULL;
		g_vars_start->v_array = array;
		g_vars_start->type = VL_ARRAY;
		return;
	}

	struct variable_list * curr = g_vars_start, *prev;

	while (curr) {
		prev = curr;
		curr = curr->next;
	}
	curr = prev;
	curr->next = malloc(sizeof(struct variable_list));
	curr = curr->next;
	curr->next = NULL;
	curr->v_array = array;
	curr->type = VL_ARRAY;
}

static void pop_scope(struct function * f) {

	assert(g_parse);
	assert(f->scopes_start);
	struct scope * curr = f->scopes_start, *prev = NULL, *prev2;

	while (curr) {
		prev2 = prev;
		prev = curr;
		curr = curr->next;
	}

	if (prev2) {
		struct variable_list * curr_v = prev->vars_start, *prev_l;
		while (curr_v) {
			prev_l = curr_v;
			curr_v = curr_v->next;
			free(prev_l);
		}
		free(prev2->next);
		prev2->next = NULL;
	} else {
		struct variable_list * curr_v = f->scopes_start->vars_start, *prev_l;
		while (curr_v) {
			prev_l = curr_v;
			curr_v = curr_v->next;
			free(prev_l);
		}
		free(f->scopes_start);
		f->scopes_start = NULL;
	}
}

static void push_scope(struct function * f) {

	assert(g_parse);
	if (!f->scopes_start) {
		f->scopes_start = malloc(sizeof(struct scope));
		f->scopes_start->vars_start = NULL;
		return;
	}

	struct scope * curr = f->scopes_start, *prev;

	while (curr) {
		prev = curr;
		curr = curr->next;
	}

	curr = prev;
	curr->next = malloc(sizeof(struct scope));
	curr = curr->next;

	curr->next = NULL;
	curr->vars_start = NULL;
}

#if 1
static void add_var(struct function * f, struct variable * v) {

	assert(g_parse);
	assert(f->scopes_start);

	struct scope * curr_scope = f->scopes_start, *prev_scope;

	while (curr_scope) {
		prev_scope = curr_scope;
		curr_scope = curr_scope->next;
	}
	curr_scope = prev_scope;
	if (!curr_scope->vars_start) {
		curr_scope->vars_start = malloc(sizeof(struct variable_list));
		curr_scope->vars_start->next = NULL;
		curr_scope->vars_start->v = v;
		curr_scope->vars_start->type = VL_VAR;
		return;
	}

	struct variable_list * curr = curr_scope->vars_start, *prev;

	while (curr) {
		prev = curr;
		curr = curr->next;
	}
	curr = prev;
	curr->next = malloc(sizeof(struct variable_list));
	curr = curr->next;
	curr->next = NULL;
	curr->v = v;
	curr->type = VL_VAR;
}


static void add_array(struct function * f, struct array * array) {

	assert(g_parse);
	assert(f->scopes_start);

	struct scope * curr_scope = f->scopes_start, *prev_scope;

	while (curr_scope) {
		prev_scope = curr_scope;
		curr_scope = curr_scope->next;
	}
	curr_scope = prev_scope;
	if (!curr_scope->vars_start) {
		curr_scope->vars_start = malloc(sizeof(struct variable_list));
		curr_scope->vars_start->next = NULL;
		curr_scope->vars_start->v_array = array;
		curr_scope->vars_start->type = VL_ARRAY;
		return;
	}
	struct variable_list * curr = curr_scope->vars_start, *prev;

	while (curr) {
		prev = curr;
		curr = curr->next;
	}
	curr = prev;
	curr->next = malloc(sizeof(struct variable_list));
	curr = curr->next;
	curr->next = NULL;
	curr->v_array = array;
	curr->type = VL_ARRAY;
}
#endif


static struct stmt * new_stmt(struct function * f) {

	assert(g_parse);
        assert(f);

        if (!f->start) {
                f->start = malloc(sizeof(struct stmt));
                f->start->next = NULL;
                return f->start;
        }

        struct stmt * tmp = f->start, *prev;

        while (tmp) {
                prev = tmp;
                tmp = tmp->next;
        }
        tmp = prev;
        tmp->next = malloc(sizeof(struct stmt));
        tmp->next->next = NULL;

        return tmp->next;
}

static int push_op_stack(struct function * t) {

	assert(g_parse);
        if (op_height == 0) {
                operator_stack = malloc(sizeof(struct function*)*10);
                operator_stack[0] = t;
                op_height = total_op_height = 1;
                return 0;
        }

        operator_stack[op_height] = t;

        if (total_op_height == op_height) {

                op_height++;
                total_op_height++;
                if (op_height%10 == 0) {
                        operator_stack = realloc(operator_stack, sizeof(struct function*)*(op_height+10));
                }
                return 0;
        }

        op_height++;

	return 0;
}

static int push_op_stack_preparse(struct token_a * t) {

        assert(!g_parse);
        if (op_height_preparse == 0) {
                operator_stack_preparse = malloc(sizeof(struct token_a*)*10);
                operator_stack_preparse[0] = t;
                op_height_preparse = total_op_height_preparse = 1;
                return 0;
        }

        operator_stack_preparse[op_height_preparse] = t;

        if (total_op_height_preparse == op_height_preparse) {

                op_height_preparse++;
                total_op_height_preparse++;
                if (op_height_preparse%10 == 0) {
                        operator_stack_preparse = realloc(operator_stack_preparse, sizeof(struct token_a*)*(op_height_preparse+10));
                }
                return 0;
        }

        op_height_preparse++;

        return 0;
}

static int add_to_output_queue(struct composite * token) {

	assert(g_parse);
        if (oq_len == 0) {
                output_queue = malloc(sizeof(struct composite*)*10);
                output_queue[0] = token;
                oq_len = total_oq_len = 1;
                return 0;
        }

        output_queue[oq_len] = token;

        if (oq_len == total_oq_len) {
                oq_len++;
                total_oq_len++;
                if (oq_len%10 == 0) {
                        // expand the output_queue
                        output_queue = realloc(output_queue, sizeof(struct composite*)*(oq_len+10));
                }
                return 0;
        }

        oq_len++;
	return 0;
}

static struct function * pop_op_stack() {

	assert(g_parse);
        if (op_height == 0) {
                return NULL;
        }
        op_height--;
        struct function * f = operator_stack[op_height];
        operator_stack[op_height] = NULL;
        return f;
}

static struct token_a * pop_op_stack_preparse() {

	assert(!g_parse);
	if (op_height_preparse == 0) {
		return NULL;
	}
	op_height_preparse--;
	struct token_a * t = operator_stack_preparse[op_height_preparse];
	operator_stack_preparse[op_height_preparse] = NULL;
	return t;
}

static int get_precedence(struct function * f) {

	assert(f->ftype == MATH_FUNC);
	if (f->mtype == GT_FUNC || f->mtype == LT_FUNC ||
			f->mtype == GTEQ_FUNC || f->mtype == LTEQ_FUNC
			|| f->mtype == EQEQ_FUNC || f->mtype == NEQ_FUNC) {
		return 1;
	}
        if (f->mtype == POWER_FUNC) {
                return 4;
        }
        if (f->mtype == MULT_FUNC || f->mtype == DIV_FUNC) {
                return 3;
        }
        if (f->mtype == ADD_FUNC || f->mtype == SUB_FUNC) {
                return 2;
        }

        fprintf(stderr, "ERROR IN get_precedence!\n");
        assert(0);
}

static int get_precedence_preparse(enum token_type type) {
	
	if (type == GT || type == LT || type == GTEQ || type == LTEQ
			|| type == EQEQ || type == NEQ) {
		return 1;
	}
	if (type == POWER) {
		return 4;
	}
	if (type == MULT || type == DIV) {
		return 3;
	}
	if (type == PLUS || type == MINUS) {
		return 2;
	}
	fprintf(stderr, "ERROR IN get_precedence_preparse\n");
	assert(0);
}

static int left_associative(enum token_type type) {

        assert(type == POWER || type == MULT || type == DIV || type == PLUS || type == MINUS);

        return type != POWER;
}

static struct stmt * get_label(struct function * f, char * name) {

	assert(g_parse);
	struct label * curr = f->labels_start;
	while (curr) {
		if (!strcmp(curr->name, name)) {
			return curr->landing;
		}
	}
	fprintf(stderr, "%s: goto stmt cannot find label!\n", __func__);
	return NULL;
}

#if 0
static int is_operator(enum token_type type) {

        return type == POWER || type == MULT ||
                type == DIV || type == PLUS ||
                type == MINUS;
}
#endif

#if 0
static struct variable * get_var(char * name, struct function * f) {

	struct scope * curr = f->scopes_start;
	while (curr) {
		struct variable_list * curr_v = curr->vars_start;
		while (curr_v) {
			if (!strcmp(curr_v->v->name, name)) {
				return curr_v->v;
			}
			curr_v = curr_v->next;
		}
		curr = curr->next;
	}

	struct variable_list * gvs = g_vars_start;

	while (gvs) {
		if (!strcmp(gvs->v->name, name)) {
			return gvs->v;
		}
		gvs = gvs->next;
	}
	fprintf(stderr, "%s: could not find variable '%s'\n", __func__, name);
	return NULL;
		
}
#endif

static struct token_a * get_token() {

	struct token_a * tmp = g_tokens[c_token];
	c_token++;
	return tmp;
}

static void unget_token() {

	c_token--;
}

static int is_math_function(enum token_type type) {

	return type == POWER || type == PLUS || type == MINUS ||
		type == MULT || type == DIV || type == EQEQ || 
		type == GT || type == GTEQ || type == LT ||
		type == LTEQ || type == NEQ;
}

static struct function * get_math_function(enum token_type type) {

	assert(g_parse);
	if (type == POWER) {
		struct function * f = malloc(sizeof(struct function));
		f->ftype = MATH_FUNC;
		f->mtype = POWER_FUNC;
		return f;
	}
	if (type == PLUS) {
		struct function * f = malloc(sizeof(struct function));
		f->ftype = MATH_FUNC;
		f->mtype = ADD_FUNC;
		return f;
	}
	if (type == MINUS) {
		struct function * f = malloc(sizeof(struct function));
		f->ftype = MATH_FUNC;
		f->mtype = SUB_FUNC;
		return f;
	}
	if (type == MULT) {
		struct function * f = malloc(sizeof(struct function));
		f->ftype = MATH_FUNC;
		f->mtype = MULT_FUNC;
		return f;
	}
	if (type == DIV) {
		struct function * f = malloc(sizeof(struct function));
		f->ftype = MATH_FUNC;
		f->mtype = DIV_FUNC;
		return f;
	}
	if (type == NEQ) {
		struct function * f = malloc(sizeof(struct function));
		f->ftype = MATH_FUNC;
		f->mtype = NEQ_FUNC;
		return f;
	}
	if (type == EQEQ) {
		struct function * f = malloc(sizeof(struct function));
		f->ftype = MATH_FUNC;
		f->mtype = EQEQ_FUNC;
		return f;
	}
	if (type == GT) {
		struct function * f = malloc(sizeof(struct function));
		f->ftype = MATH_FUNC;
		f->mtype = GT_FUNC;
		return f;
	}
	if (type == GTEQ) {
		struct function * f = malloc(sizeof(struct function));
		f->ftype = MATH_FUNC;
		f->mtype = GTEQ_FUNC;
		return f;
	}
	if (type == LT) {
		struct function * f = malloc(sizeof(struct function));
		f->ftype = MATH_FUNC;
		f->mtype = LT_FUNC;
		return f;
	}
	if (type == LTEQ) {
		struct function * f = malloc(sizeof(struct function));
		f->ftype = MATH_FUNC;
		f->mtype = LTEQ_FUNC;
		return f;
	}
	return NULL;
}

static struct function * get_function(char * name) {

	assert(g_parse);
	int i;

	if (!strcmp(name, "__array_get__")) {
		struct function * f = malloc(sizeof(struct function));
		f->ftype = ARRAY_FUNC;
		return f;
	}

	if (!strcmp(name, "len")) {
		struct function * f = malloc(sizeof(struct function));
		f->ftype = LEN_FUNC;
		return f;
	}
	for (i = 0; i < g_num_functions; i++) {
		if (!strcmp(g_functions[i]->name, name)) {
			return g_functions[i];
		}
	}
	fprintf(stderr, "%s: returning NULL\n", __func__);
	return NULL;
}


static int parse_expr(struct function * f) {

	if (!f) {
		fprintf(stderr, "CALLING parse_expr() WITHOUT FUNCTION\n");
		exit(1);
	}

	int lparen_stack = 0;
	struct function * t12;
	while (1) {

		struct token_a *curr = get_token();

		if (lparen_stack == 0) {
			if (curr->type == RPAREN || curr->type == SEMICOLON || curr->type == RBRACKET) {
				unget_token();
				goto bail;
			}
		}
		if (curr->type == COMMA) {
			continue; // ignore commas
		}
		struct function * f0;
		if (curr->type == INT || curr->type == FLOAT || curr->type == CHAR || curr->type == SINGLE_QUOTE_STRING || curr->type == DOUBLE_QUOTE_STRING) {
			if (g_parse) {
				struct composite * co = malloc(sizeof(struct composite));
				co->type = CONSTANT;
				struct variable * conv = malloc(sizeof(struct variable));
				co->var_ptr = conv;

				if (curr->type == INT) {
					conv->type = INT_VAR;
					conv->int_value = atoi(curr->value);
				} else if (curr->type == FLOAT) {
					conv->type = FLOAT_VAR;
					conv->float_value = atof(curr->value);
				} else if (curr->type == SINGLE_QUOTE_STRING || curr->type == DOUBLE_QUOTE_STRING) {
					conv->type = STRING_VAR;
					conv->string_value = curr->value;
				} else if (curr->type == CHAR) {
					conv->type = CHAR_VAR;
					conv->char_value = curr->value[0];
				}
					
				add_to_output_queue(co);
			} // ignore if else
		} else if (curr->type == IDENTIFIER || curr->type == LEN) { 
			struct token_a * tmp = curr;
			curr = get_token();
			if (curr->type == LPAREN) {
				if (g_parse) {
					push_op_stack(get_function(tmp->value));
				} else {
					push_op_stack_preparse(tmp);
				}
			} else if (curr->type == LBRACKET) {
				if (g_parse) {
					struct function * ts = get_function("__array_get__");
					ts->name = tmp->value;
					push_op_stack(ts);
				} else {
					push_op_stack_preparse(tmp);
				}
			} else {
				if (g_parse) {
					struct composite * ce2 = malloc(sizeof(struct composite));
					ce2->type = VAR;
					ce2->var_name = tmp->value;
					add_to_output_queue(ce2);
				} // ignore first time around
			}
			unget_token();
		} else if (is_math_function(curr->type)) {
			if (g_parse) {
				f0 = get_math_function(curr->type);
			}
			while (1) {
				struct function * top = NULL;
				struct token_a * top_preparse = NULL;
				if (g_parse) {
					top = pop_op_stack();
				} else {
					top_preparse = pop_op_stack_preparse();
				}
				if (top || top_preparse) {
					int o1precedence;
					int o2precedence;
					if ((g_parse && (top->ftype != LPAREN_PLACEHOLDER && top->ftype != LBRACKET_PLACEHOLDER)) || 
							(!g_parse && (top_preparse->type != LPAREN && top_preparse->type != LBRACKET))) {
						if (g_parse) {
							o1precedence = get_precedence(f0);
							o2precedence = get_precedence(top);
						} else {
							o1precedence = get_precedence_preparse(curr->type);
							o2precedence = get_precedence_preparse(top_preparse->type);
						}
						int cond1 = o2precedence > o1precedence;
						int cond2 = o2precedence == o1precedence;
						int cond3 = left_associative(curr->type);
						if (cond1 || (cond2 && cond3)) {
							if (g_parse) {
								struct composite * comp = malloc(sizeof(struct composite));
								comp->type = FUNC;
								comp->func_ptr = top;
								add_to_output_queue(comp);
							}
						} else {
							if (g_parse) {
								push_op_stack(top);
							} else {
								push_op_stack_preparse(top_preparse);
							}
							break;
						}
					} else {
						if (g_parse) {
							push_op_stack(top);
						} else {
							push_op_stack_preparse(top_preparse);
						}
						break;
					}
				} else {
					break;
				}
			}
			if (g_parse) {
				push_op_stack(f0);
			} else {
				push_op_stack_preparse(curr);
			}
		} else if (curr->type == LPAREN || curr->type == LBRACKET) {
			if (g_parse) {
				struct function * f0 = malloc(sizeof(struct function));
				if (curr->type == LPAREN) {
					f0->ftype = LPAREN_PLACEHOLDER;
				} else {
					f0->ftype = LBRACKET_PLACEHOLDER;
				}
				push_op_stack(f0);
			} else {
				push_op_stack_preparse(curr);
			}
			lparen_stack++;
		} else if (curr->type == RPAREN || curr->type == RBRACKET) {
			struct function * top;
			struct token_a * top_preparse;
			lparen_stack--;
			while (1) {
				if (g_parse) {
					top = pop_op_stack();
				} else {
					top_preparse = pop_op_stack_preparse();
				}
				if (curr->type == RPAREN) {
					if ((g_parse && top->ftype != LPAREN_PLACEHOLDER) || (!g_parse && top_preparse->type != LPAREN)) {
						if (g_parse) {
							struct composite * comp = malloc(sizeof(struct composite));
							comp->type = FUNC;
							comp->func_ptr = top;
							add_to_output_queue(comp);
						}
					} else {
						if (g_parse) {
							free(top);
						}
						break;
					}
				} else {
					if ((g_parse && top->ftype != LBRACKET_PLACEHOLDER) || (!g_parse && top_preparse->type != LBRACKET)) {
						if (g_parse) {
							struct composite * comp = malloc(sizeof(struct composite));
							comp->type = FUNC;
							comp->func_ptr = top;
							add_to_output_queue(comp);
						}
					} else {
						if (g_parse) {
							free(top);
						}
						break;
					}
				}
			}

			if (g_parse) {
				top = pop_op_stack();
				if (top) {
					if (top->ftype == USER_DEFINED || top->ftype == ARRAY_FUNC || top->ftype == LEN_FUNC) {
						struct composite * c = malloc(sizeof(struct composite));
						c->type = FUNC;
						c->func_ptr = top;
						add_to_output_queue(c);
					} else {
						push_op_stack(top);
					}
				} else {
					fprintf(stderr, "WARNING: POPPED NULL FROM OP STACK\n");
				}
			} else {
				top_preparse = pop_op_stack_preparse();
				if (top_preparse && !is_math_function(top_preparse->type)) {
					// ignore
				} else if (top_preparse) {
					push_op_stack_preparse(top_preparse);
				} else {
					fprintf(stderr, "WARNING: POPPED NULL FROM OP STACK PREPARSE\n");
				}
			}
		}

	}

bail:

	if (g_parse) {
		while ((t12=pop_op_stack())) {
			assert(t12->ftype != LPAREN_PLACEHOLDER);
			struct composite * comp = malloc(sizeof(struct composite));
			comp->type = FUNC;
			comp->func_ptr = t12;
			add_to_output_queue(comp);
		}


		if (operator_stack) {
			free(operator_stack);
			operator_stack = NULL;
		}
		struct composite ** done = malloc(sizeof(struct composite*)*oq_len);
		int idx;
		for (idx = 0; idx < oq_len; idx++) {
			done[idx] = output_queue[idx];
		}

		struct stmt * st = new_stmt(f);
		st->type = EXEC_EXPR;
		st->expr_stmts = done;
		st->num_expr_stmts = oq_len;

		oq_len = 0;
		free(output_queue);
		output_queue = NULL;
	} else {
		while (pop_op_stack_preparse()) {
			// ignore
		}

		if (operator_stack_preparse) {
			free(operator_stack_preparse);
			operator_stack_preparse = NULL;
		}
	}

	return 0;
}




static int parse_assignment(struct function * f) {

	struct token_a * t = get_token();

	if (t->type == KEYWORD_STRING || t->type == KEYWORD_CHAR || t->type == KEYWORD_FLOAT
			|| t->type == KEYWORD_INT) {
		enum var_type type;
		if (t->type == KEYWORD_STRING) {
			type = STRING_VAR;
		} else if (t->type == KEYWORD_CHAR) {
			type = CHAR_VAR;
		} else if (t->type == KEYWORD_FLOAT) {
			type = FLOAT_VAR;
		} else {
			type = INT_VAR;
		}
		t = get_token();
		if (t->type == IDENTIFIER) {
			char * name = t->value;
			t = get_token();
			if (t->type == EQUALS) {
				if (parse_expr(f) == 0) {
					t = get_token();
					if (t->type == SEMICOLON) {
						if (g_parse) {
							struct variable * var = malloc(sizeof(struct variable));
							var->type = type;
							var->name = name;
							struct stmt * stmt = new_stmt(f);
							stmt->type = VAR_DEC;
							stmt->var = var;
							add_var(f, var);
							stmt->next = malloc(sizeof(struct stmt));
							stmt = stmt->next;
							stmt->next = NULL;
							stmt->type = ASSIGNMENT;
							stmt->assign_name = name;
						}
						return 0;
					} else {
						fprintf(stderr, "%s: expected SEMICOLON got '%s'\n", __func__, t->value);
						return 1;
					}
				} else {
					fprintf(stderr, "%s: error in parse_expr()\n", __func__);
					return 1;
				}
			} else {
				fprintf(stderr, "%s: expected EQUALS got '%s'\n", __func__, t->value);
				return 1;
			} 
		} else {
			fprintf(stderr, "%s: expected IDENTIFIER got '%s'\n", __func__, t->value);
			return 1;
		}
	} else if (t->type == IDENTIFIER) {

		char * tmp = t->value;
		t = get_token();
		if (t->type == EQUALS) {
			if (parse_expr(f) == 0) {
				if (g_parse) {
					struct stmt * stmt = new_stmt(f);
					stmt->type = ASSIGNMENT;
					stmt->assign_name = tmp;
				}
				t = get_token();
				if (t->type == SEMICOLON) {
					return 0;
				} else {
					fprintf(stderr, "%s: expected SEMICOLON but got '%s'\n", __func__, t->value);
					return 1;
				}
			} else {
				fprintf(stderr, "%s: error in parse_expr()\n", __func__);
				return 1;
			}
		} else {
			fprintf(stderr, "%s: expected EQUALS got '%s' 1\n", __func__, t->value);
			return 1;
		}
	} else {
		fprintf(stderr, "%s: expected KEYWORD_STRING or KEYWORD_CHAR or KEYWORD_FLOAT or KEYWORD_INT or IDENTIFIER got '%s'\n", __func__, t->value);
		return 1;
	}
}

				
static int parse_function_call(struct function * f) {

	int x = parse_expr(f);	
	if (x == 0) {
		struct token_a * t = get_token();
		if (t->type == SEMICOLON) {
			return 0;
		} else {
			fprintf(stderr, "%s: expected SEMICOLON got '%s'\n", __func__, t->value);
			return 1;
		}
	} else {
		fprintf(stderr, "%s: failed in parse_expr()\n", __func__);
		return 1;
	}
	

}


static int parse_var_decl(struct function * f) {

	struct token_a * t = get_token();
	enum var_type type;
	char * name;

	if (t->type == KEYWORD_STRING || t->type == KEYWORD_FLOAT || 
			t->type == KEYWORD_CHAR || t->type == KEYWORD_INT) {

		if (t->type == KEYWORD_INT) {
			type = INT_VAR;
		} else if (t->type == KEYWORD_FLOAT) {
			type = FLOAT_VAR;
		} else if (t->type == KEYWORD_CHAR) {
			type = CHAR_VAR;
		} else {
			type = STRING_VAR;
		}

		t = get_token();
		if (t->type == IDENTIFIER) {

			name = t->value;
			t = get_token();
			if (t->type == SEMICOLON) {
				if (g_parse) {
					struct variable * v = malloc(sizeof(struct variable));
					v->type = type;
					v->name = name;

					if (f) {
						struct stmt * stmt = new_stmt(f);
						stmt->type = VAR_DEC;
						stmt->var = v;
						add_var(f, v);
					} else {
						add_global_var(v);
					}
				}

				return 0;
			}
			if (t->type == EQUALS) {

				if (!f) {
					fprintf(stderr, "cannot assign variable outside of function!\n");
					return 1;
				}
				int x = parse_expr(f);

				if (x == 0) {
					t = get_token();
					if (t->type == SEMICOLON) {
						if (g_parse) {
							struct variable * v = malloc(sizeof(struct variable));
							v->type = type;
							v->name = name;
							struct stmt * stmt = new_stmt(f);
							stmt->type = VAR_DEC;
							stmt->var = v;
							stmt->next = malloc(sizeof(struct stmt));
							stmt = stmt->next;
							stmt->next = NULL;
							stmt->type = ASSIGNMENT;
							stmt->var = v;
						}
						return 0;
					} else {
						fprintf(stderr, "BAD TOKEN 1\n");
						return 1;
					}
				} else {
					fprintf(stderr, "error in parse_expr\n");
					return 1;
				}
			} 
			if (t->type == LPAREN) {

				unget_token();
				unget_token();
				unget_token();
				return 1;
			}
			return 1;
		} else {
			unget_token();
			fprintf(stderr, "bad token d\n");
			return 1;
		}
	} else {
		unget_token();
		return 1;
	}
}

static int parse_return_stmt(struct function * f) {

	struct token_a * t = get_token();

	if (t->type == RETURN) {
		t = get_token();

		if (t->type != SEMICOLON) {
			unget_token();
			int s = parse_expr(f);
			if (s == 0) {
				t = get_token();
				if (t->type == SEMICOLON) {
					if (g_parse) {
						struct stmt * stmt = new_stmt(f);
						stmt->type = RETURN_STMT;
					}
					return 0;
				} else {
					fprintf(stderr, "unexpected token '%s' dfd\n", t->value);
					return 1;
				}
			} else {
				fprintf(stderr, "%s: error in parse_expr\n", __func__);
				return 1;
			}
		} else {
			return 0;
		}
	} else {
		fprintf(stderr, "%s: unexpected token: '%s' df\n", __func__, t->value);
		return 1;
	}
}

static int parse_for_loop() {

	fprintf(stderr, "unimplemented for loop\n");
	exit(1);
}

static int parse_do_while_loop(struct function * f) {

	assert(f);
	struct token_a * t = get_token();
	if (t->type == DO) {
		t = get_token();
		if (t->type == LBRACE) {
			struct stmt * nop;
			if (g_parse) {
				nop = new_stmt(f);
				nop->type = NOOP_STMT;
				struct stmt * stmt = new_stmt(f);
				stmt->type = PUSH_SCOPE;
				push_scope(f);
			}
			while (1) {
				t = get_token();
				if (t->type == RBRACE) {
					goto finish_while_loop;
				}
				unget_token();
				if (parse_stmt(f)) {
					fprintf(stderr, "%s: error in parse_stmt()\n", __func__);
					return 1;
				}
			}

finish_while_loop:
			t = get_token();
			if (t->type == WHILE) {
				t = get_token();
				if (t->type == LPAREN) {
					int x = parse_expr(f);
					if (x == 0) {
						t = get_token();
						if (t->type == RPAREN) {
							t = get_token();
							if (t->type == SEMICOLON) {
								if (g_parse) {
									struct stmt * stmt = new_stmt(f);
									stmt->type = POP_SCOPE;
									pop_scope(f);
									stmt->next = malloc(sizeof(struct stmt));
									stmt = stmt->next;
									stmt->type = IF_STMT;
									struct stmt * if_stmt = stmt;
									stmt->next = malloc(sizeof(struct stmt));
									stmt = stmt->next;
									stmt->type = GOTO_STMT;
									stmt->alt = nop;
									stmt->next = malloc(sizeof(struct stmt));
									stmt->next->type = NOOP_STMT;
									if_stmt->alt = stmt->next;
									stmt->next->next = NULL;
								}
								return 0;
							} else {
								fprintf(stderr, "%s; expected SEMICOLON got '%s'\n", __func__, t->value);
								return 1;
							}
						} else {
							fprintf(stderr, "%s: expected RPAREN got '%s'\n", __func__, t->value);
							return 1;
						}
					} else {
						fprintf(stderr, "%s: error in parse_expr()\n", __func__);
						return 1;
					}
				} else {
					fprintf(stderr, "%s: expected LPAREN got '%s'\n", __func__, t->value);
					return 1;
				}
			} else {
				fprintf(stderr, "%s: expected WHILE got '%s'\n", __func__, t->value);
				return 1;
			}
		} else {
			fprintf(stderr, "%s: expected LBRACE got '%s'\n", __func__, t->value);
			return 1;
		}
	} else {
		fprintf(stderr, "%s: expected DO got '%s'\n", __func__, t->value);
		return 1;
	}



}

static int parse_while_loop(struct function * f) {


	struct token_a * t = get_token();

	if (t->type == WHILE) {

		t = get_token();

		if (t->type == LPAREN) {
			struct stmt * nop;
			if (g_parse) {
				nop = new_stmt(f);
				nop->type = NOOP_STMT;
			}

			int x = parse_expr(f);
			if (x == 0) {

				t = get_token();
				if (t->type == RPAREN) {

					struct stmt * stmt, *ifstmt;
					if (g_parse) {
						stmt = new_stmt(f);
						stmt->type = IF_STMT;
						ifstmt = stmt;
					}
					t = get_token();
					if (t->type == LBRACE) {
						if (g_parse) {
							stmt->next = malloc(sizeof(struct stmt));
							stmt = stmt->next;
							stmt->type = PUSH_SCOPE;
						}

						t = get_token();
						if (t->type == RBRACE) {
							if (g_parse) {
								stmt->next = malloc(sizeof(struct stmt));
								stmt = stmt->next;
								stmt->type = POP_SCOPE;
								stmt->next = malloc(sizeof(struct stmt));
								stmt = stmt->next;
								stmt->type = GOTO_STMT;
								stmt->alt = nop;
								stmt->next = malloc(sizeof(struct stmt));
								stmt = stmt->next;
								stmt->next = NULL;
								stmt->type = NOOP_STMT;
								ifstmt->alt = stmt;
							}
							return 0;
						}
						if (g_parse) {
							push_scope(f);
							stmt->next = NULL;
						}
						unget_token();
						while (1) {
							if (parse_stmt(f) == 0) {
								t = get_token();
								if (t->type == RBRACE) {
									if (g_parse) {

										stmt = new_stmt(f);
										stmt->type = POP_SCOPE;
										pop_scope(f);
										stmt->next = malloc(sizeof(struct stmt));
										stmt = stmt->next;
										stmt->type = GOTO_STMT;
										stmt->alt = nop;
										stmt->next = malloc(sizeof(struct stmt));
										stmt = stmt->next;
										stmt->next = NULL;
										stmt->type = NOOP_STMT;
										ifstmt->alt = stmt;
									}
									return 0;
								}
								unget_token();
							} else {
								fprintf(stderr, "%s: error in parse_stmt()\n", __func__);
								return 1;
							}
						}
					} else {
						fprintf(stderr, "%s: expected LBRACE got '%s'\n", __func__, t->value);
						return 1;
					}
				} else {
					fprintf(stderr, "%s: expected RPAREN got '%s'\n", __func__, t->value);
					return 1;
				}
			} else {
				fprintf(stderr, "%s: error in parse_expr()\n", __func__);
				return 1;
			}
		} else {
			fprintf(stderr, "%s: expected LPAREN got '%s'\n", __func__, t->value);
			return 1;
		}
	} else {
		fprintf(stderr, "%s: expected WHILE got '%s'\n", __func__, t->value);
		return 1;
	}
}

static int parse_print_stmt(struct function * f) {

	struct token_a * t = get_token();
	if (t->type == PRINT) {
		t = get_token();
		if (t->type == LPAREN) {
			if (parse_expr(f) == 0) {
				t = get_token();
				if (t->type == RPAREN) {
					t = get_token();
					if (t->type == SEMICOLON) {
						if (g_parse) {
							struct stmt * stmt = new_stmt(f);
							stmt->type = PRINT_STMT;
						}
						return 0;
					} else {
						fprintf(stderr, "%s: expected SEMICOLON got '%s'\n", __func__, t->value);
						return 1;
					}
				} else {
					fprintf(stderr, "%s: expected RPAREN got '%s'\n", __func__, t->value);
					return 1;
				}
			} else {
				fprintf(stderr, "%s: error in parse_expr\n", __func__);
				return 1;
			}
		} else {
			fprintf(stderr, "%s: expected LPAREN got '%s'\n", __func__, t->value);
			return 1;
		}
	} else {
		fprintf(stderr, "%s: expected PRINT got '%s'\n", __func__, t->value);
		return 1;
	}
}

static int parse_if_stmt(struct function * f, struct stmt * end) {

	struct token_a * t = get_token();
	if (t->type == IF) {
		t = get_token();
		if (t->type == LPAREN) {
			struct stmt * nop;
			if (g_parse && end == NULL) {
				nop = malloc(sizeof(struct stmt));
				nop->type = NOOP_STMT;
				nop->next = NULL;
			}
			if (parse_expr(f) == 0) {
				t = get_token();
				if (t->type == RPAREN) {
					t = get_token();
					if (t->type == LBRACE) {
						struct stmt * stmt, *if_stmt;
						if (g_parse) {
							stmt = new_stmt(f);
							stmt->type = IF_STMT;
							if_stmt = stmt;

							stmt->next = malloc(sizeof(struct stmt));
							stmt = stmt->next;
							stmt->next = NULL;
							stmt->type = PUSH_SCOPE;
							push_scope(f);
						}
						t = get_token();
						if (t->type == RBRACE) {
							goto finish_scope;
#if 0
							if (g_parse) {
								stmt->next = malloc(sizeof(struct stmt));
								stmt = stmt->next;
								stmt->type = POP_SCOPE;
								stmt->next = malloc(sizeof(struct stmt));
								stmt = stmt->next;
								stmt->type = NOOP_STMT;
								stmt->next = NULL;
								if_stmt->alt = stmt;
							}
#endif
						}
						unget_token();
						while (1) {
							if (parse_stmt(f) == 0) {
								t = get_token();
								if (t->type == RBRACE) {
									finish_scope:
									if (g_parse) {
										stmt = new_stmt(f);
										stmt->type = POP_SCOPE;
										stmt->next = malloc(sizeof(struct stmt));
										pop_scope(f);
										stmt = stmt->next;
										stmt->type = GOTO_STMT;
										struct stmt * sw;
										if (end == NULL) {
											sw = nop;
										} else {
											sw = end;
										}
										stmt->alt = sw;
										stmt->next = malloc(sizeof(struct stmt));
										stmt = stmt->next;
										stmt->next = NULL;
										stmt->type = NOOP_STMT;
										if_stmt->alt = stmt;
									}
									t = get_token();
									if (t->type == ELSE) {
										t = get_token();
										if (t->type == IF) {
											unget_token();
											struct stmt * sw = NULL;
											if (g_parse) {
												if (end == NULL) {
													sw = nop;
												} else {
													sw = end;
												}
											}
											if (parse_if_stmt(f, sw) == 0) {
												if (g_parse) {
													//struct stmt * st = new_stmt(f);
													//st->type = NOOP_STMT;
													//st->next = sw;
#ifdef DEBUG
													printf("leaving parse_if_stmt 2\n");
													debug(f);
#endif
												}
												return 0;
											} else {
												fprintf(stderr, "%s: error in parse_if_stmt\n", __func__);
												return 1;
											}
										} else if (t->type == LBRACE) {
											t = get_token();
											if (g_parse) {
												struct stmt * st = new_stmt(f);
												st->type = PUSH_SCOPE;
											}

											if (t->type == RBRACE) {
												goto finish_else;
											}
											unget_token();
											while (1) {
												if (parse_stmt(f) == 0) {
													t = get_token();
													if (t->type == RBRACE) {
														finish_else:
														if (g_parse) {
															struct stmt * st = new_stmt(f);
															st->type = POP_SCOPE;
#ifdef DEBUG
															printf("leaving parse_if_stmt 1\n");
															debug(f);
#endif
														}
														return 0;
													} else {
														unget_token();
													}
												} else {
													fprintf(stderr, "%s: ERROR IN PARSE_STMT()\n", __func__);
													return 1;
												}
											}
										} else {
											fprintf(stderr, "%s: expected LBRACE got '%s'\n", __func__, t->value);
											return 1;
										}
									} else {
										if (g_parse) {
											if (end == NULL) {
												stmt->next = nop;
											} else {
												stmt->next = end;
											}
#ifdef DEBUG
											printf("leaving parse_if_stmt 0\n");
											debug(f);
#endif
										}
										unget_token();
										return 0;
									}
								} else {
									unget_token();
								}
							} else {
								fprintf(stderr, "%s: error in parse_stmt()\n", __func__);
								return 1;
							}
						} /* end while(1) */
					} else {
						fprintf(stderr, "%s: expected LBRACE got '%s'\n", __func__, t->value);
						return 1;
					}
				} else {
					fprintf(stderr, "%s: expected RPAREN got '%s'\n", __func__, t->value);
					return 1;
				} 
			} else {
				fprintf(stderr, "%s: error in parse_expr()\n", __func__);
				return 1;
			}
		} else {
			fprintf(stderr, "%s: expected LPAREN got '%s'\n", __func__, t->value);
			return 1;
		}
	} else {
		fprintf(stderr, "%s: expected IF got '%s'\n", __func__, t->value);
		return 1;
	}
}

															
static int parse_println_stmt(struct function * f) {

	struct token_a * t = get_token();

	if (t->type == PRINTLN) {
		t = get_token();
		if (t->type == LPAREN) {
			if (parse_expr(f) == 0) {
				t = get_token();
				if (t->type == RPAREN) {
					t = get_token();
					if (t->type == SEMICOLON) {
						if (g_parse) {
							struct stmt * stmt = new_stmt(f);
							stmt->type = PRINTLN_STMT;
						}
						return 0;
					} else {
						fprintf(stderr, "%s: expected SEMICOLON got '%s'\n", __func__, t->value);
						return 1;
					}
				} else {
					fprintf(stderr, "%s: expected RPAREN got '%s'\n", __func__, t->value);
					return 1;
				}
			} else {
				fprintf(stderr, "%s: error in parse_expr()\n", __func__);
				return 1;
			}
		} else {
			fprintf(stderr, "%s: expected LPAREN got '%s'\n", __func__, t->value);
			return 1;
		}
	} else {
		fprintf(stderr, "%s: expected PRINTLN got '%s'\n", __func__, t->value);
		return 1;
	}
}

static int parse_eprint_stmt(struct function *f) {

        struct token_a * t = get_token();

        if (t->type == EPRINT) {
                t = get_token();
                if (t->type == LPAREN) {
                        if (parse_expr(f) == 0) {
                                t = get_token();
                                if (t->type == RPAREN) {
                                        t = get_token();
                                        if (t->type == SEMICOLON) {
						if (g_parse) {
							struct stmt * stmt = new_stmt(f);
							stmt->type = EPRINT_STMT;
						}
                                                return 0;
                                        } else {
                                                fprintf(stderr, "%s: expected SEMICOLON got '%s'\n", __func__, t->value);
                                                return 1;
                                        }
                                } else {
                                        fprintf(stderr, "%s: expected RPAREN got '%s'\n", __func__, t->value);
                                        return 1;
                                }
                        } else {
                                fprintf(stderr, "%s: error in parse_expr()\n", __func__);
                                return 1;
                        }
                } else {
                        fprintf(stderr, "%s: expected LPAREN got '%s'\n", __func__, t->value);
                        return 1;
                }
        } else {
                fprintf(stderr, "%s: expected EPRINT got '%s'\n", __func__, t->value);
                return 1;
        }
}

static int parse_eprintln_stmt(struct function * f) {

        struct token_a * t = get_token();

        if (t->type == EPRINTLN) {
                t = get_token();
                if (t->type == LPAREN) {
                        if (parse_expr(f) == 0) {
                                t = get_token();
                                if (t->type == RPAREN) {
                                        t = get_token();
                                        if (t->type == SEMICOLON) {
						if (g_parse) {
							struct stmt * stmt = new_stmt(f);
							stmt->type = EPRINTLN_STMT;
						}
                                                return 0;
                                        } else {
                                                fprintf(stderr, "%s: expected SEMICOLON got '%s'\n", __func__, t->value);
                                                return 1;
                                        }
                                } else {
                                        fprintf(stderr, "%s: expected RPAREN got '%s'\n", __func__, t->value);
                                        return 1;
                                }
                        } else {
                                fprintf(stderr, "%s: error in parse_expr()\n", __func__);
                                return 1;
                        }
                } else {
                        fprintf(stderr, "%s: expected LPAREN got '%s'\n", __func__, t->value);
                        return 1;
                }
        } else {
                fprintf(stderr, "%s: expected EPRINTLN got '%s'\n", __func__, t->value);
                return 1;
        }
}

static int parse_array_decl(struct function * f) {

	struct token_a * t = get_token();
	if (t->type == KEYWORD_STRING || t->type == KEYWORD_FLOAT
			|| t->type == KEYWORD_INT || t->type == KEYWORD_CHAR) {
		enum var_type type;
		if (g_parse) {
			if (t->type == KEYWORD_STRING) {
				type = STRING_VAR;
			} else if (t->type == KEYWORD_CHAR) {
				type = CHAR_VAR;
			} else if (t->type == KEYWORD_FLOAT) {
				type = FLOAT_VAR;
			} else {
				type = INT_VAR;
			}
		}
		t = get_token(f);
		if (t->type == IDENTIFIER) {
			char * name;
			if (g_parse) {
				name = t->value;
			}
			t = get_token();
			if (t->type == LBRACKET) {
				t = get_token();
				if (t->type == RBRACKET) {
					t = get_token();
					if (t->type == EQUALS) {
						t = get_token();
						if (t->type == LBRACE) {
							struct variable_list * avl_start, *avl_curr;
							if (g_parse) {
								avl_start = NULL;
							}
							while (1) {
								t = get_token();
								if (t->type == RBRACE) {
									break;
								}
								if (t->type == COMMA) {
									t = get_token();
								}
								if (g_parse) {
									struct variable * v = malloc(sizeof(struct variable));
									if (t->type == INT) {
										assert(type == INT_VAR);
										v->type = INT_VAR;
										v->int_value = atoi(t->value);
									} else if (t->type == FLOAT) {
										assert(type == FLOAT_VAR);
										v->type = FLOAT_VAR;
										v->float_value = atof(t->value);
									} else if (t->type == SINGLE_QUOTE_STRING || t->type == DOUBLE_QUOTE_STRING) {
										assert(type == STRING_VAR);
										v->type = STRING_VAR;
										v->string_value = t->value;
									} else if (t->type == CHAR) {
										assert(type == CHAR_VAR);
										assert(strlen(t->value) == 1);
										v->type = CHAR_VAR;
										v->char_value = t->value[0];
									} else {
										fprintf(stderr, "UNKNOWN TYPE IN ARRAY DECL\n");
										exit(1);
									}
									if (avl_start) {
										struct variable_list * avl_prev;
										avl_curr = avl_start;
										while (avl_curr) {
											avl_prev = avl_curr;
											avl_curr = avl_curr->next;
										}
										avl_curr = avl_prev;
										avl_curr->next = malloc(sizeof(struct variable_list));
										avl_curr = avl_curr->next;
									} else {
										avl_start = malloc(sizeof(struct variable_list));
										avl_start->next = NULL;
										avl_curr = avl_start;
									}
									avl_curr->type = VL_VAR;
									avl_curr->v = v;
									avl_curr->next = NULL;
								}
							}
							t = get_token();
							if (t->type == SEMICOLON) {
								if (g_parse) {
									struct array * arr = malloc(sizeof(struct array));
									arr->vars = convert_vl_to_array(avl_start, &arr->num_vars);
									arr->name = name;
									arr->type = type;
									if (f) {
										struct stmt * stmt = new_stmt(f);
										stmt->type = ARRAY_DEC;
										stmt->array = arr;
										add_array(f, arr);
									} else {
										add_array_g(arr);
									}
								}
								return 0;
							} else {
								fprintf(stderr, "%s: expected SEMICOLON got '%s'\n", __func__, t->value);
								return 1;
							}
						} else {
							fprintf(stderr, "%s: expected LBRACKET got '%s'\n", __func__, t->value);
							return 1;
						}
					} else if (t->type == SEMICOLON) {
						if (g_parse) {
							struct array * array = malloc(sizeof(struct array));
							array->vars = NULL;
							array->num_vars = 0;
							array->name = name;
							array->type = type;
							if (f) {
								struct stmt * stmt = new_stmt(f);
								stmt->type = ARRAY_DEC;
								stmt->array = array;
								add_array(f, array);
							} else {
								add_array_g(array);
							}
						}
						return 0;
					} else {
						fprintf(stderr, "%s: expected EQUALS or SEMICOLON got '%s'\n", __func__, t->value);
						return 1;
					}
				} else {
					fprintf(stderr, "%s: expected RBRACKET got '%s'\n", __func__, t->value);
					return 1;
				}
			} else {
				fprintf(stderr, "%s: expected LBRACKET got '%s'\n", __func__, t->value);
				return 1;
			}
		} else {
			fprintf(stderr, "%s: expected IDENTIFIER got '%s'\n", __func__, t->value);
			return 1;
		}
	} else {
		fprintf(stderr, "%s: expected KEYWORD_* got '%s'\n", __func__, t->value);
		return 1;
	}
}


static int parse_goto_stmt(struct function * f) {

	struct token_a * t = get_token();

	if (t->type == GOTO) {
		t = get_token();
		if (t->type == IDENTIFIER) {
			if (g_parse) {
				struct stmt * stmt = new_stmt(f);
				stmt->type = GOTO_STMT;
				stmt->alt = get_label(f, t->value);
			}
			t = get_token();
			if (t->type == SEMICOLON) {
				return 0;
			} else {
				fprintf(stderr, "%s: expected SEMICOLON got '%s'\n", __func__, t->value);
				return 1;
			}
		} else {
			fprintf(stderr, "%s: expected IDENTIFIER got '%s'\n", __func__, t->value);
			return 1;
		}
	} else {
		fprintf(stderr, "%s: expected GOTO got '%s'\n", __func__, t->value);
		return 1;
	}
}

static int parse_label(struct function * f) {

	struct token_a * t = get_token();

	if (t->type == IDENTIFIER) {
		char * tmp = t->value;
		t = get_token();
		if (t->type == COLON) {
			if (g_parse) {
				struct label * curr = f->labels_start, *prev;
				while (curr) {
					prev = curr;
					curr = curr->next;
				}
				curr = prev;
				curr->next = malloc(sizeof(struct label));
				curr = curr->next;
				curr->name = tmp;
				struct stmt * nop = new_stmt(f);
				nop->type = NOOP_STMT;
				curr->landing = nop;
				curr->next = NULL;
			}
			return 0;
		} else {
			fprintf(stderr, "%s: expected COLON got '%s'\n", __func__, t->value);
			return 1;
		}
	} else {
		fprintf(stderr, "%s: expected IDENTIFIER got '%s'\n", __func__, t->value);
		return 1;
	}
}

static int parse_array_assignment(struct function * f) {

	struct token_a * t = get_token();
	if (t->type == IDENTIFIER) {
		char * name;
		if (g_parse) {
			name = t->value;
		}
		t = get_token();
		if (t->type == LBRACKET) {
			if (parse_expr(f) == 0) {
				if (g_parse) {
					struct stmt * st = new_stmt(f);
					st->type = ARRAY_PREP;
				}
				t = get_token();
				if (t->type == RBRACKET) {
					t = get_token();
					if (t->type == EQUALS) {
						if (parse_expr(f) == 0) {
							t = get_token();
							if (t->type == SEMICOLON) {
								if (g_parse) {
									struct stmt * stmt = new_stmt(f);
									stmt->type = ASSIGNMENT;
									stmt->assign_name = name;
								}
								return 0;
							} else {
								fprintf(stderr, "%s: expected SEMICOLON got '%s'\n", __func__, t->value);
								return 1;
							}
						} else {
							fprintf(stderr, "%s: error in parse_expr() 2\n", __func__);
							return 1;
						}
					} else {
						fprintf(stderr, "%s: expected EQUALS got '%s'\n", __func__, t->value);
						return 1;
					}
				} else {
					fprintf(stderr, "%s: expected RBRACKET got '%s'\n", __func__, t->value);
					return 1;
				}
			} else {
				fprintf(stderr, "%s: error in parse_expr() 1\n", __func__);
				return 1;
			}
		} else {
			fprintf(stderr, "%s: expected LBRACKET got '%s'\n", __func__, t->value);
			return 1;
		}
	} else {
		fprintf(stderr, "%s: expected IDENTIFIER got '%s'\n", __func__, t->value);
		return 1;
	}
}

static int parse_push_stmt(struct function * f) {

	struct token_a * t = get_token();
	if (t->type == PUSH) {
		t = get_token();
		if (t->type == LPAREN) {
			t = get_token();
			if (t->type == IDENTIFIER) {
				char * name;
				if (g_parse) {
					name = t->value;
				}
				t = get_token();
				if (t->type == COMMA) {
					if (parse_expr(f) == 0) {
						t = get_token();
						if (t->type == RPAREN) {
							t = get_token();
							if (t->type == SEMICOLON) {
								if (g_parse) {
									struct stmt * stmt = new_stmt(f);
									stmt->type = PUSH_STMT;
									stmt->assign_name = name;
								}
								return 0;
							} else {
								fprintf(stderr, "%s: expected SEMICOLON got '%s'\n", __func__, t->value);
								return 1;
							}
						} else {
							fprintf(stderr, "%s: expected RPAREN got '%s'\n", __func__, t->value);
							return 1;
						}
					} else {
						fprintf(stderr, "%s: expected STRING or INT or FLOAT or CHAR or IDENTIFIER got '%s'\n", __func__, t->value);
						return 1;
					}
				} else {
					fprintf(stderr, "%s: expected COMMA got '%s'\n", __func__, t->value);
					return 1;
				}
			} else {
				fprintf(stderr, "%s: expected IDENTIFIER got '%s'\n", __func__, t->value);
				return 1;
			}
		} else {
			fprintf(stderr, "%s: expected LPAREN got '%s'\n", __func__, t->value);
			return 1;
		}
	} else {
		fprintf(stderr, "%s: expected PUSH got '%s'\n", __func__, t->value);
		return 1;
	}
}

static int parse_pop_stmt(struct function * f) {

	struct token_a * t = get_token();

	if (t->type == POP) {
		t = get_token();
		if (t->type == LPAREN) {
			t = get_token();
			if (t->type == IDENTIFIER) {
				char * name;
				if (g_parse) {
					name = t->value;
				}
				t = get_token();
				if (t->type == RPAREN) {
					t = get_token();
					if (t->type == SEMICOLON) {
						if (g_parse) {
							struct stmt * stmt = new_stmt(f);
							stmt->type = POP_STMT;
							stmt->assign_name = name;
						}
						return 0;
					} else {
						fprintf(stderr, "%s: expected SEMICOLON got '%s'\n", __func__, t->value);
						return 1;
					}
				} else {
					fprintf(stderr, "%s: expected RPAREN got '%s'\n", __func__, t->value);
					return 1;
				}
			} else {
				fprintf(stderr, "%s: expected IDENTIFIER got '%s'\n", __func__, t->value);
				return 1;
			}
		} else {
			fprintf(stderr, "%s: expected LPAREN got '%s'\n", __func__, t->value);
			return 1;
		}
	} else {
		fprintf(stderr, "%s: expected POP got '%s'\n", __func__, t->value);
		return 1;
	}
}

static int parse_insert_stmt(struct function * f) {

	struct token_a * t = get_token();
	if (t->type == INSERT) {
		t = get_token();
		if (t->type == LPAREN) {
			t = get_token();
			if (t->type == IDENTIFIER) {
				char * name;
				if (g_parse) {
					name = t->value;
				}
				t = get_token();
				if (t->type == COMMA) {
					if (parse_expr(f) == 0) { // value to be inserted
						if (g_parse) {
							struct stmt * st = new_stmt(f);
							st->type = ARRAY_PREP;
						}
						t = get_token();
						if (t->type == COMMA) {
							if (parse_expr(f) == 0) { // index to be inserted
								t = get_token();
								if (t->type == RPAREN) {
									t = get_token();
									if (t->type == SEMICOLON) {
										if (g_parse) {
											struct stmt * stmt = new_stmt(f);
											stmt->type = INSERT_STMT;
											stmt->assign_name = name;
										}
										return 0;
									} else {
										fprintf(stderr, "%s: expected SEMICOLON got '%s'\n", __func__, t->value);
										return 1;
									}
								} else {
									fprintf(stderr, "%s: expected RPAREN got '%s'\n", __func__, t->value);
									return 1;
								}
							} else {
								fprintf(stderr, "%s: error in parse_expr()\n", __func__);
								return 1;
							}
						} else {
							fprintf(stderr, "%s: expected COMMA got '%s'\n", __func__, t->value);
							return 1;
						}
					} else {
						fprintf(stderr, "%s: error in parse_expr()\n", __func__);
						return 1;
					}
				} else {
					fprintf(stderr, "%s: expected COMMA got '%s'\n", __func__, t->value);
					return 1;
				}

			} else {
				fprintf(stderr, "%s: expected IDENTIFIER got '%s'\n", __func__, t->value);
				return 1;
			}
		} else {
			fprintf(stderr, "%s: expected LPAREN got '%s'\n", __func__, t->value);
			return 1;
		}
	} else {
		fprintf(stderr, "%s: expected INSERT got '%s'\n", __func__, t->value);
		return 1;
	}
}
										
static int parse_remove_stmt(struct function * f) {

	struct token_a * t = get_token();
	if (t->type == REMOVE) {
		t = get_token();
		if (t->type == LPAREN) {
			t = get_token();
			if (t->type == IDENTIFIER) {
				char * name;
				if (g_parse) {
					name = t->value;
				}
				t = get_token();
				if (t->type == COMMA) {
					if (parse_expr(f) == 0) {
						t = get_token();
						if (t->type == RPAREN) {
							t = get_token();
							if (t->type == SEMICOLON) {
								if (g_parse) {
									struct stmt * stmt = new_stmt(f);
									stmt->type = REMOVE_STMT;
									stmt->assign_name = name;
								}
								return 0;
							} else {
								fprintf(stderr, "%s: expected SEMICOLON got '%s'\n", __func__, t->value);
								return 1;
							}
						} else {
							fprintf(stderr, "%s: expected RPAREN got '%s'\n", __func__, t->value);
							return 1;
						}
					} else {
						fprintf(stderr, "%s: error in parse_expr()\n", __func__);
						return 1;
					}
				} else {
					fprintf(stderr, "%s: expected COMMA got '%s'\n", __func__, t->value);
					return 1;
				}
			} else {
				fprintf(stderr, "%s: expected IDENTIFIER got '%s'\n", __func__, t->value);
				return 1;
			}
		} else {
			fprintf(stderr, "%s: expected LPAREN got '%s'\n", __func__, t->value);
			return 1;
		}
	} else {
		fprintf(stderr, "%s: expected REMOVE got '%s'\n", __func__, t->value);
		return 1;
	}
}


static int parse_stmt(struct function * f) {

	struct token_a * t = get_token();
	if (t->type == IF) {
		unget_token();
		return parse_if_stmt(f, NULL);
	}
	if (t->type == FOR) {
		unget_token();
		return parse_for_loop(f);
	}
	if (t->type == DO) {
		unget_token();
		return parse_do_while_loop(f);
	}
	if (t->type == WHILE) {
		unget_token();
		return parse_while_loop(f);
	}
	if (t->type == GOTO) {
		unget_token();
		return parse_goto_stmt(f);
	}
	if (t->type == IDENTIFIER) {
		t = get_token();
		if (t->type == LPAREN) {
			unget_token();
			unget_token();
			return parse_function_call(f);
		}
		if (t->type == EQUALS) {
			unget_token();
			unget_token();
			return parse_assignment(f);
		}
		if (t->type == COLON) {
			unget_token();
			unget_token();
			return parse_label(f);
		}
		if (t->type == LBRACKET) {
			unget_token();
			unget_token();
			return parse_array_assignment(f);
		}
			
		fprintf(stderr, "unexpected token 12\n");
		return 1;
	}
	if (t->type == PUSH) {
		unget_token();
		return parse_push_stmt(f);
	}
	if (t->type == POP) {
		unget_token();
		return parse_pop_stmt(f);
	} 
	if (t->type == INSERT) {
		unget_token();
		return parse_insert_stmt(f);
	}
	if (t->type == REMOVE) {
		unget_token();
		return parse_remove_stmt(f);
	}
	if (t->type == PRINT) {
		unget_token();
		return parse_print_stmt(f);
	}
	if (t->type == PRINTLN) {
		unget_token();
		return parse_println_stmt(f);
	}
	if (t->type == EPRINT) {
		unget_token();
		return parse_eprint_stmt(f);
	}
	if (t->type == EPRINTLN) {
		unget_token();
		return parse_eprintln_stmt(f);
	}
	if (t->type == RETURN) {
		unget_token();
		return parse_return_stmt(f);
	}
	if (t->type == KEYWORD_INT || t->type == KEYWORD_FLOAT || 
			t->type == KEYWORD_STRING || t->type == KEYWORD_CHAR) {
		t = get_token();
		if (t->type == IDENTIFIER) {
			t = get_token();
			if (t->type == EQUALS) {
				unget_token();
				unget_token();
				unget_token();
				return parse_assignment(f);
			} else if (t->type == SEMICOLON) {
				unget_token();
				unget_token();
				unget_token();
				return parse_var_decl(f);
			} else if (t->type == LBRACKET) {
				unget_token();
				unget_token();
				unget_token();
				return parse_array_decl(f);
			} else {
				fprintf(stderr, "%s: expected EQUALS or SEMICOLON or LBRACKET got '%s'\n", __func__, t->value);
				return 1;
			}

		} else {
			fprintf(stderr, "%s: expected IDENTIFIER got '%s'\n", __func__, t->value);
			return 1;
		}
	} else {
		return 1;
	}
}

static void add_param(struct function * f, struct variable_list * new_var) {

	assert(g_parse);
	f->num_args++;
	if (f->num_args == 1) {
		f->args = malloc(sizeof(struct variable_list*));
		f->args[0] = new_var;
	} else {
		f->args = realloc(f->args, sizeof(struct variable*)*(f->num_args));
		f->args[f->num_args - 1] = new_var;
	}
}

static int parse_param(struct function * f) {

	struct token_a * t = get_token();

	if (t->type == KEYWORD_STRING || t->type == KEYWORD_CHAR ||
			t->type == KEYWORD_FLOAT || t->type == KEYWORD_INT) {

		enum token_type type = t->type;
		t = get_token();
		if (t->type == IDENTIFIER) {
			char * tmp = t->value;
			t = get_token();
			if (t->type == LBRACKET) {
				t = get_token();
				if (t->type == RBRACKET) {
					t = get_token();
					if (t->type == COMMA || t->type == RPAREN) {
						unget_token();
						if (g_parse) {
							struct variable_list * v = malloc(sizeof(struct variable_list));
							v->type = VL_ARRAY;
							v->v_array = malloc(sizeof(struct array));
							v->v_array->name = tmp;
							if (type == KEYWORD_STRING) {
								v->v_array->type = STRING_VAR;
							} else if (type == KEYWORD_CHAR) {
								v->v_array->type = CHAR_VAR;
							} else if (type == KEYWORD_INT) {
								v->v_array->type = INT_VAR;
							} else if (type == KEYWORD_FLOAT) {
								v->v_array->type = FLOAT_VAR;
							}
							add_param(f, v);
						}
						return 0;
					} else {
						fprintf(stderr, "%s: expected COMMA or RPAREN got '%s'\n", __func__, t->value);
						return 1;
					}
				} else {
					fprintf(stderr, "%s: expected RBRACKET got '%s'\n", __func__, t->value);
					return 1;
				}
			} else if (t->type == COMMA || t->type == RPAREN) {
				unget_token();
				if (g_parse) {
					struct variable_list * v = malloc(sizeof(struct variable_list));
					v->type = VL_VAR;
					v->v = malloc(sizeof(struct variable));
					if (type == KEYWORD_STRING) {
						v->v->type = STRING_VAR;
					} else if (type == KEYWORD_CHAR) {
						v->v->type = CHAR_VAR;
					} else if (type == KEYWORD_FLOAT) {
						v->v->type = FLOAT_VAR;
					} else {
						v->v->type = INT_VAR;
					}
					v->v->name = tmp;
					add_param(f, v);
				}
				return 0;
			} else {
				fprintf(stderr, "%s: expected COMMA or RPAREN got '%s'\n", __func__, t->value);
				return 1;
			}
		} else {
			fprintf(stderr, "%s: expected IDENTIFIER got '%s'\n", __func__, t->value);
			return 1;
		}
	} else {
		fprintf(stderr, "%s: expected KEYWORD_STRING or KEYWORD_CHAR or KEYWORD_FLOAT or KEYWORD_INT got '%s'\n", __func__, t->value);
		return 1;
	}

}

static int parse_function_decl() {

	struct token_a * t = get_token();

	if (t->type == KEYWORD_INT || t->type == KEYWORD_STRING || t->type == KEYWORD_FLOAT
			|| t->type == KEYWORD_CHAR) {

		enum var_type type;
		if (!g_parse) {
			if (t->type == KEYWORD_INT) {
				type = INT_VAR;
			} else if (t->type == KEYWORD_STRING) {
				type = STRING_VAR;
			} else if (t->type == KEYWORD_FLOAT) {
				type = FLOAT_VAR;
			} else {
				type = CHAR_VAR;
			}
		}
		t = get_token();
		if (t->type == IDENTIFIER) {
			char * tmp = t->value;
			t = get_token();
			if (t->type == LPAREN) {

				
				struct function * f;
				if (g_parse) {
					f = get_function(tmp);
					assert(f);
					struct stmt * stmt = new_stmt(f);
					stmt->type = PUSH_SCOPE;
					push_scope(f);
				} else {
					f = malloc(sizeof(struct function));
					f->num_args = 0;
					f->args = NULL;
					f->name = tmp;
					f->start = NULL;
					f->scopes_start = NULL;
					f->labels_start = NULL;
					f->ftype = USER_DEFINED;
					f->type = type;
				}

				t = get_token();
				if (t->type == RPAREN) {
					goto skip_params;
				}

				unget_token();
				while (1) {
					parse_param(f);

					t = get_token();
					if (t->type == RPAREN) {
						break;
					}
					if (t->type == COMMA) {
						continue;
					}

					fprintf(stderr, "unexpected token: '%s'\n", t->value);
					return 1;
				}

skip_params:
				t = get_token();
				if (t->type == LBRACE) {
					while (1) {
						if (parse_stmt(f) == 0) {
							t = get_token();
							if (t->type == RBRACE) {
								if (g_parse) {
									struct stmt * stmt = new_stmt(f);
									stmt->type = POP_SCOPE;
									pop_scope(f);
								} else {
									add_function(f);
								}
								return 0;
							}
							unget_token();
						} else {
							fprintf(stderr, "error in parse_stmt\n");
							return 1;
						}
					}
				} else {
					fprintf(stderr, "expected LBRACE got '%s'\n", t->value);
					return 1;
				}

			} else {
				fprintf(stderr, "expected LPAREN got '%s'\n", t->value);
				return 1;
			}
		} else {
			fprintf(stderr, "expected IDENTIFIER got '%s'\n", t->value);
			return 1;
		}

	} else {
		fprintf(stderr, "unexpected token sdfd\n");
		return 1;
	}
}
						

struct program * preparse_program(struct token_a ** tokens) {


	g_parse = 0;

	set_tokens(tokens);
        while (get_token()->type != END_OF_FILE) {
                unget_token();
                if (parse_var_decl(NULL) == 0) {
                        continue;
                }
                if (parse_function_decl() == 0) {
                        continue;
                }
                fprintf(stderr, "failed at %s\n", __func__);
                return NULL;
        }

        struct program * p = malloc(sizeof(struct program));

        p->functions = convert_function_list_to_array(g_funcs, &p->num_functions);

        return p;

}


int parse_program(struct token_a ** tokens, struct program * p, const int argc, const char ** argv) {

	g_parse = 1;

	g_functions = p->functions;
	g_num_functions = p->num_functions;
	set_tokens(tokens);

	while (get_token()->type != END_OF_FILE) {
                unget_token();
                if (parse_var_decl(NULL) == 0) {
                        continue;
                }
                if (parse_function_decl() == 0) {
                        continue;
                }
                fprintf(stderr, "failed at %s\n", __func__);
		return 1;
        }

	p->g_vars = convert_variable_list_to_array(g_vars_start, &p->num_g_vars);
	if (argc > 2) {
		int i;
		for (i = 0; i < p->num_functions; i++) {
			if (!strcmp(p->functions[i]->name, "main")) {
				if (p->functions[i]->num_args == 1) {
					if (p->functions[i]->args[0]->type == VL_ARRAY &&
							p->functions[i]->args[0]->v_array->type == STRING_VAR) {
						p->functions[i]->args[0]->v_array->num_vars = argc - 2;
						p->functions[i]->args[0]->v_array->vars = malloc(sizeof(struct variable*)*(argc-2));
						int x;
						for (x = 2; x < argc; x++) {
							p->functions[i]->args[0]->v_array->vars[x-2] = malloc(sizeof(struct variable));
							p->functions[i]->args[0]->v_array->vars[x-2]->type = STRING_VAR;
							p->functions[i]->args[0]->v_array->vars[x-2]->string_value = malloc(strlen(argv[x])+1);
							p->functions[i]->args[0]->v_array->vars[x-2]->string_value[0] = '\0';
							strcpy(p->functions[i]->args[0]->v_array->vars[x-2]->string_value, argv[x]);
						}
						return 0;
					}
				}
			}
		}
	}


	return 0;
	
}	



