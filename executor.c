
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "function.h"
#include "variable.h"
#include "stmt.h"
#include "executor.h"
#include "composite.h"
#include "power.h"
#include "program.h"
#include "variable_list.h"


static struct variable_list * execute_function(struct function * f);
static struct variable_list ** g_vars;
static unsigned long num_g_vars;
static struct variable* array_prep;

static int abort_flag;
int execute_program(struct program * p) {

	num_g_vars = p->num_g_vars;
	g_vars = p->g_vars;
	
	abort_flag = 0;
	int i;
	for (i = 0; i < p->num_functions; i++) {
		if (!strcmp(p->functions[i]->name, "main")) {
			execute_function(p->functions[i]);
			return abort_flag;
		}
	}

	fprintf(stderr, "cannot locate main function\n");
	return 1;
}

static struct variable_list * get_ref(struct scope * curr, struct function * f, char * name) {

	struct variable_list * curr_l = curr->vars_start;

	while (curr_l) {
		if (curr_l->type == VL_ARRAY) {
			if (!strcmp(curr_l->v_array->name, name)) {
				return curr_l;
			}
		} else if (curr_l->type == VL_VAR) {
			if (!strcmp(curr_l->v->name, name)) {
				return curr_l;
			}
		}
		curr_l = curr_l->next;
	}

	struct scope * d = f->scopes_start;

	while (d != curr) {
		curr_l = d->vars_start;
		while (curr_l) {
			if (curr_l->type == VL_ARRAY && !strcmp(curr_l->v_array->name, name)) {
				return curr_l;
			} else if (curr_l->type == VL_VAR && !strcmp(curr_l->v->name, name)) {
				return curr_l;
			}
			curr_l = curr_l->next;
		}
		d = d->next;
	}

	unsigned long i;
	for (i = 0; i < f->num_args; i++) {
		if (f->args[i]->type == VL_ARRAY && !strcmp(f->args[i]->v_array->name, name)) {
			return f->args[i];
		}
		if (f->args[i]->type == VL_VAR && !strcmp(f->args[i]->v->name, name)) {
			return f->args[i];
		}
	}

	for (i = 0; i < num_g_vars; i++) {
		if (g_vars[i]->type == VL_VAR && !strcmp(g_vars[i]->v->name, name)) {
			return g_vars[i];
		} else if (g_vars[i]->type == VL_ARRAY && !strcmp(g_vars[i]->v_array->name, name)) {
			return g_vars[i];
		}
	}

	return NULL;
}



static struct variable_list * execute_function(struct function * f) {

	struct stmt * curr = f->start;
	struct variable_list * tmp_mem = NULL;

	struct scope * curr_scope = NULL;

	array_prep = NULL;

	while (curr) {

		if (curr->type == NOOP_STMT) {
			curr = curr->next;
		} else if (curr->type == GOTO_STMT) {
			curr = curr->alt;
		} else if (curr->type == PRINTLN_STMT || curr->type == EPRINTLN_STMT) {

			assert(tmp_mem);

			FILE * fp;
			if (curr->type == PRINTLN_STMT) {
				fp = stdout;
			} else {
				fp = stderr;
			}
			if (tmp_mem->type == VL_VAR) {
				switch (tmp_mem->v->type) {
					case INT_VAR:
						fprintf(fp, "%d\n", tmp_mem->v->int_value);
						break;
					case FLOAT_VAR:
						fprintf(fp, "%f\n", tmp_mem->v->float_value);
						break;
					case CHAR_VAR:
						fprintf(fp, "%c\n", tmp_mem->v->char_value);
						break;
					case STRING_VAR:
						fprintf(fp, "%s\n", tmp_mem->v->string_value);
						break;
					default:
						fprintf(stderr, "BAD VAR\n");
						exit(1);
				}
			} else if (tmp_mem->type == VL_ARRAY) {
				fprintf(fp, "{");
				unsigned long idx;
				switch (tmp_mem->v_array->type) {
					case INT_VAR:
						for (idx = 0; idx < tmp_mem->v_array->num_vars; idx++) {
							fprintf(fp, "%d", tmp_mem->v_array->vars[idx]->int_value);
							if (idx < tmp_mem->v_array->num_vars-1) {
								fprintf(fp, ", ");
							}
						}
						break;
					case FLOAT_VAR:
						for (idx = 0; idx < tmp_mem->v_array->num_vars; idx++) {
							fprintf(fp, "%f", tmp_mem->v_array->vars[idx]->float_value);
							if (idx < tmp_mem->v_array->num_vars-1) {
								fprintf(fp, ", ");
							}
						}
						break;
					case CHAR_VAR:
						for (idx = 0; idx < tmp_mem->v_array->num_vars; idx++) {
							fprintf(fp, "%c", tmp_mem->v_array->vars[idx]->char_value);
							if (idx < tmp_mem->v_array->num_vars-1) {
								fprintf(fp, ", ");
							}
						}
						break;
					case STRING_VAR:
						for (idx = 0; idx < tmp_mem->v_array->num_vars; idx++) {
							fprintf(fp, "\"%s\"", tmp_mem->v_array->vars[idx]->string_value);
							if (idx < tmp_mem->v_array->num_vars-1) {
								fprintf(fp, ", ");
							}
						}
					default:
						assert(0);
				}
				fprintf(fp, "}\n");
			} else {
				assert(0);
			}
						

			tmp_mem = NULL;
			curr = curr->next;
		} else if (curr->type == PRINT_STMT || curr->type == EPRINT_STMT) {

			assert(tmp_mem);

			FILE * fp;
			if (curr->type == PRINT_STMT) {
				fp = stdout;
			} else {
				fp = stderr;
			}

			if (tmp_mem->type == VL_VAR) {
				switch (tmp_mem->v->type) {
					case INT_VAR:
						fprintf(fp, "%d", tmp_mem->v->int_value);
						break;
					case FLOAT_VAR:
						fprintf(fp, "%f", tmp_mem->v->float_value);
						break;
					case CHAR_VAR:
						fprintf(fp, "%c", tmp_mem->v->char_value);
						break;
					case STRING_VAR:
						fprintf(fp, "%s", tmp_mem->v->string_value);
						break;
					default:
						fprintf(stderr, "BAD VAR 1\n");
						exit(1);
				}
			} else if (tmp_mem->type == VL_ARRAY) {
				fprintf(fp, "{");
				unsigned long idx, n = tmp_mem->v_array->num_vars;
				switch (tmp_mem->v_array->type) {
					case INT_VAR:
						for (idx = 0; idx < n; idx++) {
							fprintf(fp, "%d", tmp_mem->v_array->vars[idx]->int_value);
							if (idx < n-1) {
								fprintf(fp, ", ");
							}
						}
						break;
					case FLOAT_VAR:
						for (idx = 0; idx < n; idx++) {
							fprintf(fp, "%f", tmp_mem->v_array->vars[idx]->float_value);
							if (idx < n-1) {
								fprintf(fp, ", ");
							}
						}
						break;
					case CHAR_VAR:
						for (idx = 0; idx < n; idx++) {
							fprintf(fp, "%c", tmp_mem->v_array->vars[idx]->char_value);
							if (idx < n-1) {
								fprintf(fp, ", ");
							}
						}
						break;
					case STRING_VAR:
						for (idx = 0; idx < n; idx++) {
							fprintf(fp, "%s", tmp_mem->v_array->vars[idx]->string_value);
							if (idx < n-1) {
								fprintf(fp, ", ");
							}
						}
						break;
					default:
						assert(0);
				}
				fprintf(fp, "}");
			} else {
				assert(0);
			}
						
			tmp_mem = NULL;
			curr = curr->next;
		} else if (curr->type == PUSH_SCOPE) {

			if (!f->scopes_start) {
				f->scopes_start = malloc(sizeof(struct scope));
				f->scopes_start->vars_start = NULL;
				f->scopes_start->next = NULL;
				curr_scope = f->scopes_start;
			} else {
				curr_scope->next = malloc(sizeof(struct scope));
				curr_scope = curr_scope->next;
				curr_scope->next = NULL;
				curr_scope->vars_start = NULL;
			}
			curr = curr->next;
		} else if (curr->type == POP_SCOPE) {

			if (!f->scopes_start) {
				fprintf(stderr, "%s: CALLING POP_SCOPE ON EMPTY SCOPE\n", __func__);
				exit(1);
			}
			if (!f->scopes_start->next) {
				struct variable_list * tmp1 = f->scopes_start->vars_start, *prev1;
				while (tmp1) {
					prev1 = tmp1;
					tmp1 = tmp1->next;
					if (prev1->type == VL_VAR) {
						free(prev1->v);
					} else if (prev1->type == VL_ARRAY) {
						unsigned long idx;
						for (idx = 0; idx < prev1->v_array->num_vars; idx++) {
							free(prev1->v_array->vars[idx]);
						}
						free(prev1->v_array->vars);
						free(prev1->v_array);
					}
					free(prev1);
				}
				free(f->scopes_start);
				f->scopes_start = NULL;
				curr_scope = NULL;
				curr = curr->next;
				continue;
			}

			struct scope * tmp = f->scopes_start, *prev = NULL, *prev_2;
			while (tmp) {
				prev_2 = prev;
				prev = tmp;
				tmp = tmp->next;
			}
			struct variable_list * curr_l = prev_2->next->vars_start, *prev_l;
			while (curr_l) {
				prev_l = curr_l;
				curr_l = curr_l->next;
				free(prev_l);
			}
			free(prev_2->next);
			prev_2->next = NULL;
			curr_scope = prev_2;
			curr = curr->next;


		} else if (curr->type == VAR_DEC) {

			struct variable_list * curr_l = curr_scope->vars_start, *prev;
			
			struct variable * v = malloc(sizeof(struct variable)), *t = curr->var;
			v->type = t->type;
			v->name = t->name;

			if (!curr_l) {
				curr_scope->vars_start = malloc(sizeof(struct variable_list));
				curr_scope->vars_start->next = NULL;
				curr_scope->vars_start->type = VL_VAR;
				curr_scope->vars_start->v = v;
			} else {
				while (curr_l) {
					prev = curr_l;
					curr_l = curr_l->next;
				}
				curr_l = prev;
				curr_l->next = malloc(sizeof(struct variable_list));
				curr_l->next->type = VL_VAR;
				curr_l->next->v = v;
				curr_l->next->next = NULL;
			}
			curr = curr->next;
		} else if (curr->type == ASSIGNMENT) {

			assert(tmp_mem);

			struct variable_list * assign_addr = get_ref(curr_scope, f, curr->assign_name);
			assert(assign_addr);

			if (assign_addr->type == VL_VAR && assign_addr->v->type == CHAR_VAR && tmp_mem->type == VL_VAR && tmp_mem->v->type == STRING_VAR) {
				tmp_mem->v->type = CHAR_VAR;
				tmp_mem->v->char_value = tmp_mem->v->string_value[0];
			}

			if (assign_addr->type == VL_VAR) {
				switch (assign_addr->v->type) {
					case INT_VAR:
						assert(tmp_mem->type == VL_VAR);
						assign_addr->v->int_value = tmp_mem->v->int_value;
						break;
					case FLOAT_VAR:
						assert(tmp_mem->type == VL_VAR);
						assign_addr->v->float_value = tmp_mem->v->float_value;
						break;
					case CHAR_VAR:
						assert(tmp_mem->type == VL_VAR);
						assign_addr->v->char_value = tmp_mem->v->char_value;
						break;
					case STRING_VAR:
						if (tmp_mem->type == VL_VAR) {
							assign_addr->v->string_value = tmp_mem->v->string_value;
						} else if (tmp_mem->type == VL_ARRAY) {

							if (tmp_mem->v_array->type == CHAR_VAR) {
								char * str = malloc(tmp_mem->v_array->num_vars+1);
								unsigned long idx;
								for (idx = 0; idx < tmp_mem->v_array->num_vars; idx++) {
									str[idx] = tmp_mem->v_array->vars[idx]->char_value;
								}
								str[idx] = '\0';
								assign_addr->v->string_value = str;
							} else {
								assert(0);
							}
						} else {
							assert(0);
						}
						break;
					default:
						fprintf(stderr, "%s: BAD VAR 5\n", __func__);
						break;
				}
			} else if (assign_addr->type == VL_ARRAY) {
				if (!array_prep) {
					switch(assign_addr->v_array->type) {
						case INT_VAR:
						case FLOAT_VAR:
						case CHAR_VAR:
						case STRING_VAR:
							assert(tmp_mem->type == VL_ARRAY);
							assign_addr->v_array->num_vars = tmp_mem->v_array->num_vars;
							assign_addr->v_array->vars = tmp_mem->v_array->vars;
							break;
						default:
							assert(0);
					}
				} else {
					assert(tmp_mem->type == VL_VAR);
					assert(array_prep->type == INT_VAR);
					int idx = array_prep->int_value;
					assert(assign_addr->v_array->type == tmp_mem->v->type);
					switch (assign_addr->v_array->type) {
						case INT_VAR:
							assign_addr->v_array->vars[idx]->int_value = tmp_mem->v->int_value;
							break;
						case FLOAT_VAR:
							assign_addr->v_array->vars[idx]->float_value = tmp_mem->v->float_value;
							break;
						case CHAR_VAR:
							assign_addr->v_array->vars[idx]->char_value = tmp_mem->v->char_value;
							break;
						case STRING_VAR:
							assign_addr->v_array->vars[idx]->string_value = tmp_mem->v->string_value;
							break;
						default:
							assert(0);
					}
				}

			} else {
				assert(0);
			}
				


			tmp_mem = NULL;
			curr = curr->next;
		} else if (curr->type == RETURN_STMT) {

			// pop all the scopes
			struct scope * curr_s = f->scopes_start, *prev;

			while (curr_s) {
				struct variable_list * curr_l = curr_s->vars_start, *prev_l;
				while (curr_l) {
					prev_l = curr_l;
					curr_l = curr_l->next;
					if (prev_l->v->type == STRING_VAR) {
						free(prev_l->v->string_value);
					}
					free(prev_l->v);
					free(prev_l);
				}
				prev = curr_s;
				curr_s = curr_s->next;
				free(prev);
			}
			f->scopes_start = NULL;

			return tmp_mem;
		} else if (curr->type == EXEC_EXPR) {

			int i;
			struct variable_list ** value_stack = malloc(sizeof(struct variable_list*)*10);
			int stack_height = 0, total_stack_height = 0;
			for (i = 0; i < curr->num_expr_stmts; i++) {
				struct composite * comp = curr->expr_stmts[i];
				if (comp->type == VAR || comp->type == CONSTANT) {
					if (comp->type == VAR) {
						value_stack[stack_height] = get_ref(curr_scope, f, comp->var_name);
					} else {
						struct variable_list * s = malloc(sizeof(struct variable_list));
						s->type = VL_VAR;
						s->v = comp->var_ptr;
						value_stack[stack_height] = s;
					}
					if (stack_height == total_stack_height) {
						stack_height++;
						total_stack_height++;
						if (stack_height%10 == 0) {
							value_stack = realloc(value_stack, sizeof(struct variable_list*)*(stack_height+10));
						}
					} else {
						stack_height++;
					}
				} else if (comp->type == FUNC) {
					struct variable_list * rt = malloc(sizeof(struct variable_list));
					struct variable * result = malloc(sizeof(struct variable));
					rt->type = VL_VAR;
					rt->v = result;
					if (comp->func_ptr->ftype == LEN_FUNC) {
						struct variable_list * top = value_stack[stack_height - 1];
						stack_height--;
						result->type = INT_VAR;
						if (top->type == VL_ARRAY) {
							result->int_value = top->v_array->num_vars;
						} else if (top->type == VL_VAR && top->v->type == STRING_VAR) {
							result->int_value = (int)strlen(top->v->string_value);
						} else {
							fprintf(stderr, "%s: attempt to call len on something other than an array or a string\n", __func__);
							assert(0);
						}

					} else if (comp->func_ptr->ftype == ARRAY_FUNC) {
						struct variable_list * top = value_stack[stack_height - 1];
						struct variable_list * arr = get_ref(curr_scope, f, comp->func_ptr->name);
						stack_height--;
						assert(arr->type == VL_ARRAY);
						result->type = arr->v_array->type;
						assert(top->type == VL_VAR && top->v->type == INT_VAR);
						if (top->v->int_value >= arr->v_array->num_vars) {
							fprintf(stderr, "attempt to index '%s' beyond last element\n", arr->v_array->name);
							abort_flag = 1;
							return NULL;
						}
						if (arr->v_array->type == INT_VAR) {
							result->int_value = arr->v_array->vars[top->v->int_value]->int_value;
						} else if (arr->v_array->type == FLOAT_VAR) {
							result->float_value = arr->v_array->vars[top->v->int_value]->float_value;
						} else if (arr->v_array->type == CHAR_VAR) {
							result->char_value = arr->v_array->vars[top->v->int_value]->char_value;
						} else if (arr->v_array->type == STRING_VAR) {
							result->string_value = arr->v_array->vars[top->v->int_value]->string_value;
						}


					} else if (comp->func_ptr->ftype == MATH_FUNC) {
						assert(value_stack[stack_height-1]->type == VL_VAR);
						assert(value_stack[stack_height-2]->type == VL_VAR);
						struct variable * right = value_stack[stack_height - 1]->v;
						struct variable * left = value_stack[stack_height - 2]->v;
						stack_height--;
						value_stack[stack_height] = NULL;
						stack_height--;
						if (comp->func_ptr->mtype == ADD_FUNC) {
							if (left->type == INT_VAR && right->type == INT_VAR) {
								result->int_value = left->int_value + right->int_value;
								result->type = INT_VAR;
							} else if (left->type == FLOAT_VAR && right->type == FLOAT_VAR) {
								result->float_value = left->float_value + right->float_value;
								result->type = FLOAT_VAR;
							} else if (left->type == INT_VAR && right->type == FLOAT_VAR) {
								result->type = FLOAT_VAR;
								result->float_value = (float)left->int_value + right->float_value;
							} else if (left->type == FLOAT_VAR && right->type == INT_VAR) {
								result->type = FLOAT_VAR;
								result->float_value = left->float_value + (float)right->int_value;
							} else if (left->type == STRING_VAR && right->type == STRING_VAR) {
								result->type = STRING_VAR;
								result->string_value = malloc(strlen(left->string_value)+strlen(right->string_value)+2);
								result->string_value[0] = '\0';
								strcpy(result->string_value, left->string_value);
								strcat(result->string_value, right->string_value);
							} else if (left->type == STRING_VAR && right->type == CHAR_VAR) {
								result->type = STRING_VAR;
								result->string_value = malloc(strlen(left->string_value)+3);
								result->string_value[0] = '\0';
								char stuff[2];
								stuff[0] = right->char_value;
								stuff[1] = '\0';
								strcpy(result->string_value, left->string_value);
								strcat(result->string_value, stuff);
							} else if (left->type == STRING_VAR && right->type == INT_VAR) {
								result->type = STRING_VAR;
								int len = snprintf(NULL, 0, "%d", right->int_value);
								char * stuff = malloc(len+1);
								stuff[0] = '\0';
								snprintf(stuff, len+1, "%d", right->int_value);
								result->string_value = malloc(len + strlen(left->string_value) + 2);
								result->string_value[0] = '\0';
								strcpy(result->string_value, left->string_value);
								strcat(result->string_value, stuff);
								free(stuff);
							} else if (left->type == STRING_VAR && right->type == FLOAT_VAR) {
								result->type = STRING_VAR;
								int len = snprintf(NULL, 0, "%f", right->float_value);
								char * stuff = malloc(len + 1);
								stuff[0] = '\0';
								snprintf(stuff, len+1, "%f", right->float_value);
								result->string_value = malloc(len + strlen(left->string_value) + 2);
								result->string_value[0] = '\0';
								strcpy(result->string_value, left->string_value);
								strcat(result->string_value, stuff);
							} else if (left->type == CHAR_VAR && right->type == STRING_VAR) {
								result->type = STRING_VAR;
								result->string_value = malloc(strlen(right->string_value) + 3);
								char stuff[2];
								stuff[0] = left->char_value;
								stuff[1] = '\0';
								result->string_value[0] = '\0';
								strcpy(result->string_value, stuff);
								strcat(result->string_value, right->string_value);
							} else if (left->type == INT_VAR && right->type == STRING_VAR) {
								result->type = STRING_VAR;
								int len = snprintf(NULL, 0, "%d", left->int_value);
								char * stuff = malloc(len + 1);
								stuff[0] = '\0';
								snprintf(stuff, len+1, "%d", left->int_value);
								result->string_value = malloc(strlen(right->string_value)+len+2);
								result->string_value[0] = '\0';
								strcpy(result->string_value, stuff);
								strcat(result->string_value, right->string_value);
								free(stuff);
							} else if (left->type == FLOAT_VAR && right->type == STRING_VAR) {
								result->type = STRING_VAR;
								int len = snprintf(NULL, 0, "%f", left->float_value);
								char * stuff = malloc(len + 1);
								stuff[0] = '\0';
								snprintf(stuff, len+1, "%f", left->float_value);
								result->string_value = malloc(strlen(right->string_value)+len+2);
								result->string_value[0] = '\0';
								strcpy(result->string_value, stuff);
								strcat(result->string_value, right->string_value);
								free(stuff);
							} else {
								fprintf(stderr, "%s: BAD ADDING\n", __func__);
								exit(1);
							}
						} else if (comp->func_ptr->mtype == SUB_FUNC) {
							if (left->type == INT_VAR && right->type == INT_VAR) {
								result->type = INT_VAR;
								result->int_value = left->int_value - right->int_value;
							} else if (left->type == FLOAT_VAR && right->type == FLOAT_VAR) {
								result->type = FLOAT_VAR;
								result->float_value = left->float_value - right->float_value;
							} else if (left->type == INT_VAR && right->type == FLOAT_VAR) {
								result->type = FLOAT_VAR;
								result->float_value = (float)left->int_value - right->float_value;
							} else if (left->type == FLOAT_VAR && right->type == INT_VAR) {
								result->type = FLOAT_VAR;
								result->float_value = left->float_value - (float)right->int_value;
							} else {
								fprintf(stderr, "%s: BAD SUBBING\n", __func__);
								exit(1);
							}
						} else if (comp->func_ptr->mtype == MULT_FUNC) {
							if (left->type == INT_VAR && right->type == INT_VAR) {
								result->type = INT_VAR;
								result->int_value = left->int_value * right->int_value;
							} else if (left->type == FLOAT_VAR && right->type == FLOAT_VAR) {
								result->type = FLOAT_VAR;
								result->float_value = left->float_value * right->float_value;
							} else if (left->type == INT_VAR && right->type == FLOAT_VAR) {
								result->type = FLOAT_VAR;
								result->float_value = (float)left->int_value * right->float_value;
							} else if (left->type == FLOAT_VAR && right->type == INT_VAR) {
								result->type = FLOAT_VAR;
								result->float_value = left->float_value * (float)right->int_value;
							} else {
								fprintf(stderr, "%s: BAD MULT\n", __func__);
								exit(1);
							}
						} else if (comp->func_ptr->mtype == DIV_FUNC) {
							if (left->type == INT_VAR && right->type == INT_VAR) {
								result->type = INT_VAR;
								result->int_value = left->int_value / right->int_value;
							} else if (left->type == FLOAT_VAR && right->type == FLOAT_VAR) {
								result->type = FLOAT_VAR;
								result->float_value = left->float_value / right->float_value;
							} else if (left->type == INT_VAR && right->type == FLOAT_VAR) {
								result->type = FLOAT_VAR;
								result->float_value = (float)left->int_value / right->float_value;
							} else if (left->type == FLOAT_VAR && right->type == INT_VAR) {
								result->type = FLOAT_VAR;
								result->float_value = left->float_value / (float)right->int_value;
							} else {
								fprintf(stderr, "%s: BAD DIV\n", __func__);
								exit(1);
							}
						} else if (comp->func_ptr->mtype == POWER_FUNC) {
							if (left->type == INT_VAR && right->type == INT_VAR) {
								result->type = INT_VAR;
								result->int_value = power_int(left->int_value, right->int_value);
							} else if (left->type == FLOAT_VAR && right->type == FLOAT_VAR) {
								result->type = FLOAT_VAR;
								result->float_value = power_fl_fr(left->float_value, right->float_value);
							} else if (left->type == INT_VAR && right->type == FLOAT_VAR) {
								result->type = FLOAT_VAR;
								result->float_value = power_il_fr(left->int_value, right->float_value);
							} else if (left->type == FLOAT_VAR && right->type == INT_VAR) {
								result->type = FLOAT_VAR;
								result->float_value = power_fl_ir(left->float_value, right->int_value);
							} else {
								fprintf(stderr, "%s: BAD POWER\n", __func__);
								exit(1);
							}
						} else if (comp->func_ptr->mtype == EQEQ_FUNC) {
							result->type = INT_VAR;
							if (left->type == INT_VAR && right->type == INT_VAR) {
								result->int_value = left->int_value == right->int_value;
							} else if (left->type == FLOAT_VAR && right->type == FLOAT_VAR) {
								result->int_value = (int)left->float_value == (int)right->float_value;
							} else if (left->type == INT_VAR && right->type == FLOAT_VAR) {
								result->int_value = left->int_value == (int)left->float_value;
							} else if (left->type == INT_VAR && right->type == CHAR_VAR) {
								result->int_value = left->int_value == (int)right->char_value;
							} else if (left->type == FLOAT_VAR && right->type == INT_VAR) {
								result->int_value = (int)left->float_value == right->int_value;
							} else if (left->type == CHAR_VAR && right->type == INT_VAR) {
								result->int_value = (int)left->char_value == right->char_value;
							} else if (left->type == CHAR_VAR && right->type == CHAR_VAR) {
								result->int_value = left->char_value == right->char_value;
							} else if (left->type == STRING_VAR && right->type == STRING_VAR) {
								result->int_value = !strcmp(left->string_value, right->string_value);
							} else {
								fprintf(stderr, "%s: BAD EQEQ\n", __func__);
								exit(1);
							}
						} else if (comp->func_ptr->mtype == NEQ_FUNC) {
							result->type = INT_VAR;
							if (left->type == INT_VAR && right->type == INT_VAR) {
								result->int_value = left->int_value != right->int_value;
							} else if (left->type == FLOAT_VAR && right->type == FLOAT_VAR) {
								result->int_value = (int)left->float_value != (int)right->float_value;
							} else if (left->type == INT_VAR && right->type == FLOAT_VAR) {
								result->int_value = left->int_value != (int)left->float_value;
							} else if (left->type == INT_VAR && right->type == CHAR_VAR) {
								result->int_value = left->int_value != (int)right->char_value;
							} else if (left->type == FLOAT_VAR && right->type == INT_VAR) {
								result->int_value = (int)left->float_value != right->int_value;
							} else if (left->type == CHAR_VAR && right->type == INT_VAR) {
								result->int_value = (int)left->char_value != right->char_value;
							} else if (left->type == CHAR_VAR && right->type == CHAR_VAR) {
								result->int_value = left->char_value != right->char_value;
							} else if (left->type == STRING_VAR && right->type == STRING_VAR) {
								result->int_value = strcmp(left->string_value, right->string_value);
							} else {
								fprintf(stderr, "%s: BAD NEQ\n", __func__);
								assert(0);
							}
						} else if (comp->func_ptr->mtype == GT_FUNC) {
							result->type = INT_VAR;
							if (left->type == INT_VAR && right->type == INT_VAR) {
								result->int_value = left->int_value > right->int_value;
							} else if (left->type == FLOAT_VAR && right->type == FLOAT_VAR) {
								result->int_value = (int)left->float_value > (int)right->float_value;
							} else if (left->type == INT_VAR && right->type == FLOAT_VAR) {
								result->int_value = left->int_value > (int)left->float_value;
							} else if (left->type == INT_VAR && right->type == CHAR_VAR) {
								result->int_value = left->int_value > (int)right->char_value;
							} else if (left->type == FLOAT_VAR && right->type == INT_VAR) {
								result->int_value = (int)left->float_value > right->int_value;
							} else if (left->type == CHAR_VAR && right->type == INT_VAR) {
								result->int_value = (int)left->char_value > right->char_value;
							} else if (left->type == CHAR_VAR && right->type == CHAR_VAR) {
								result->int_value = left->char_value > right->char_value;
							} else if (left->type == STRING_VAR && right->type == STRING_VAR) {
								result->int_value = strcmp(left->string_value, right->string_value) > 0;
							} else {
								fprintf(stderr, "%s: BAD GT\n", __func__);
								exit(1);
							}
						} else if (comp->func_ptr->mtype == GTEQ_FUNC) {
							result->type = INT_VAR;
							if (left->type == INT_VAR && right->type == INT_VAR) {
								result->int_value = left->int_value >= right->int_value;
							} else if (left->type == FLOAT_VAR && right->type == FLOAT_VAR) {
								result->int_value = (int)left->float_value >= (int)right->float_value;
							} else if (left->type == INT_VAR && right->type == FLOAT_VAR) {
								result->int_value = left->int_value >= (int)left->float_value;
							} else if (left->type == INT_VAR && right->type == CHAR_VAR) {
								result->int_value = left->int_value >= (int)right->char_value;
							} else if (left->type == FLOAT_VAR && right->type == INT_VAR) {
								result->int_value = (int)left->float_value >= right->int_value;
							} else if (left->type == CHAR_VAR && right->type == INT_VAR) {
								result->int_value = (int)left->char_value >= right->char_value;
							} else if (left->type == CHAR_VAR && right->type == CHAR_VAR) {
								result->int_value = left->char_value >= right->char_value;
							} else if (left->type == STRING_VAR && right->type == STRING_VAR) {
								result->int_value = strcmp(left->string_value, right->string_value) >= 0;
							} else {
								fprintf(stderr, "%s: BAD GTEQ\n", __func__);
								exit(1);
							}
						} else if (comp->func_ptr->mtype == LT_FUNC) {
							result->type = INT_VAR;
							if (left->type == INT_VAR && right->type == INT_VAR) {
								result->int_value = left->int_value < right->int_value;
							} else if (left->type == FLOAT_VAR && right->type == FLOAT_VAR) {
								result->int_value = (int)left->float_value < (int)right->float_value;
							} else if (left->type == INT_VAR && right->type == FLOAT_VAR) {
								result->int_value = left->int_value < (int)left->float_value;
							} else if (left->type == INT_VAR && right->type == CHAR_VAR) {
								result->int_value = left->int_value < (int)right->char_value;
							} else if (left->type == FLOAT_VAR && right->type == INT_VAR) {
								result->int_value = (int)left->float_value < right->int_value;
							} else if (left->type == CHAR_VAR && right->type == INT_VAR) {
								result->int_value = (int)left->char_value < right->char_value;
							} else if (left->type == CHAR_VAR && right->type == CHAR_VAR) {
								result->int_value = left->char_value < right->char_value;
							} else if (left->type == STRING_VAR && right->type == STRING_VAR) {
								result->int_value = strcmp(left->string_value, right->string_value) < 0;
							} else {
								fprintf(stderr, "%s: BAD LT\n", __func__);
								exit(1);
							}
						} else if (comp->func_ptr->mtype == LTEQ_FUNC) {
							result->type = INT_VAR;
							if (left->type == INT_VAR && right->type == INT_VAR) {
								result->int_value = left->int_value <= right->int_value;
							} else if (left->type == FLOAT_VAR && right->type == FLOAT_VAR) {
								result->int_value = (int)left->float_value <= (int)right->float_value;
							} else if (left->type == INT_VAR && right->type == FLOAT_VAR) {
								result->int_value = left->int_value <= (int)left->float_value;
							} else if (left->type == INT_VAR && right->type == CHAR_VAR) {
								result->int_value = left->int_value <= (int)right->char_value;
							} else if (left->type == FLOAT_VAR && right->type == INT_VAR) {
								result->int_value = (int)left->float_value <= right->int_value;
							} else if (left->type == CHAR_VAR && right->type == INT_VAR) {
								result->int_value = (int)left->char_value <= right->char_value;
							} else if (left->type == CHAR_VAR && right->type == CHAR_VAR) {
								result->int_value = left->char_value <= right->char_value;
							} else if (left->type == STRING_VAR && right->type == STRING_VAR) {
								result->int_value = strcmp(left->string_value, right->string_value) <= 0;
							} else {
								fprintf(stderr, "%s: BAD LTEQ\n", __func__);
								exit(1);
							}
						} else {
							fprintf(stderr, "%s: BAD MATH FUNC\n", __func__);
							exit(1);
						}
					} else if (comp->func_ptr->ftype == USER_DEFINED) {
						int num = comp->func_ptr->num_args;
						struct variable_list ** vars;
						if (num) {
							vars = malloc(sizeof(struct variable_list*)*num);
						}
						int i;
						for (i = num - 1; i >= 0; i--) {
							vars[i] = value_stack[stack_height - 1];
							stack_height--;
							if (comp->func_ptr->args[i]->type == VL_VAR) {
								vars[i]->v->name = comp->func_ptr->args[i]->v->name;
							} else if (comp->func_ptr->args[i]->type == VL_ARRAY) {
								vars[i]->v_array->name = comp->func_ptr->args[i]->v_array->name;
							} else {
								assert(0);
							}
						}
						struct function * copy = malloc(sizeof(struct function));
						copy->num_args = num;
						copy->args = vars;
						copy->ftype = USER_DEFINED;
						copy->type = comp->func_ptr->type;
						copy->name = comp->func_ptr->name;
						copy->start = comp->func_ptr->start;
						copy->scopes_start = NULL;

						struct variable_list * res = execute_function(copy);
						if (abort_flag) {
							return NULL;
						}
						free(copy);
						if (num > 0) {
							free(vars);
						}

						rt->type = res->type;

						if (res->type == VL_VAR) {
							result->type = res->v->type;
							switch (res->v->type) {
								case INT_VAR:
									result->int_value = res->v->int_value;
									break;
								case FLOAT_VAR:
									result->float_value = res->v->float_value;
									break;
								case CHAR_VAR:
									result->char_value = res->v->char_value;
									break;
								case STRING_VAR:
									result->string_value = res->v->string_value;
									break;
								default:
									fprintf(stderr, "%s: BAD RETURN FUNC\n", __func__);
									exit(1);
							}
						} else if (res->type == VL_ARRAY) {
							switch (res->v_array->type) {
								case INT_VAR:
								case FLOAT_VAR:
								case CHAR_VAR:
								case STRING_VAR:
									rt->v_array = res->v_array;
									rt->v_array->type = res->v_array->type;
									break;
								default:
									assert(0);
							}
							free(result);
							rt->v = NULL;
						} else {
							assert(0);
						}
					} else {
						fprintf(stderr, "%s: BAD FTYPE\n", __func__);
						exit(1);
					}
					value_stack[stack_height] = rt;
					stack_height++;
				} else {
					fprintf(stderr, "%s: BAD COMP TYPE\n", __func__);
					exit(1);
				}
			}
			if (stack_height == 1) {
				tmp_mem = value_stack[0];
			} else {
				fprintf(stderr, "%s: WARNING: stack_height != 1 after EXEC_EXPR\n", __func__);
			}
			free(value_stack);
			curr = curr->next;
		} else if (curr->type == IF_STMT) {

			assert(tmp_mem->type == VL_VAR);
			assert(tmp_mem->v->type == INT_VAR);
			if (tmp_mem->v->int_value) {
				curr = curr->next;
			} else {
				curr = curr->alt;
			}
		} else if (curr->type == ARRAY_DEC) {


			assert(curr_scope);
			struct variable_list * curr_l = curr_scope->vars_start, *prev;
			struct array * arr = malloc(sizeof(struct array));
			arr->name = curr->array->name;
			arr->type = curr->array->type;
			arr->num_vars = curr->array->num_vars;
			arr->vars = malloc(sizeof(struct variable*)*arr->num_vars);
			unsigned long idx;
			for (idx = 0; idx < arr->num_vars; idx++) {
				arr->vars[idx] = malloc(sizeof(struct variable));
				arr->vars[idx]->type = curr->array->vars[idx]->type;
				switch (arr->vars[idx]->type) {
					case INT_VAR:
						arr->vars[idx]->int_value = curr->array->vars[idx]->int_value;
						break;
					case FLOAT_VAR:
						arr->vars[idx]->float_value = curr->array->vars[idx]->float_value;
						break;
					case CHAR_VAR:
						arr->vars[idx]->char_value = curr->array->vars[idx]->char_value;
						break;
					case STRING_VAR:
						arr->vars[idx]->string_value = curr->array->vars[idx]->string_value;
						break;
					default:
						assert(0);
				}
			}

			if (!curr_l) {
				curr_scope->vars_start = malloc(sizeof(struct variable_list));
				curr_scope->vars_start->next = NULL;
				curr_scope->vars_start->type = VL_ARRAY;
				curr_scope->vars_start->v_array = arr;
			} else {
				while (curr_l) {
					prev = curr_l;
					curr_l = curr_l->next;
				}
				curr_l = prev;
				curr_l->next = malloc(sizeof(struct variable_list));
				curr_l = curr_l->next;
				curr_l->next = NULL;
				curr_l->type = VL_ARRAY;
				curr_l->v_array = arr;
			}

			curr = curr->next;
		
		} else if (curr->type == ARRAY_PREP) {

			assert(tmp_mem->type == VL_VAR);
			array_prep = tmp_mem->v;
			tmp_mem = NULL;
			curr = curr->next;
		} else if (curr->type == PUSH_STMT) {

			assert(tmp_mem);
			assert(tmp_mem->type == VL_VAR);

			struct variable_list * arr = get_ref(curr_scope, f, curr->assign_name);
			assert(arr->type == VL_ARRAY);

			if (arr->v_array->num_vars > 0) {
				arr->v_array->vars[arr->v_array->num_vars] = tmp_mem->v;
			} else {
				arr->v_array->vars = malloc(sizeof(struct variable*)*10);
				arr->v_array->vars[0] = tmp_mem->v;
			}
			tmp_mem = NULL;

			if (arr->v_array->num_vars == arr->v_array->total_vars) {
				arr->v_array->num_vars++;
				arr->v_array->total_vars++;
				if (arr->v_array->num_vars%10 == 0) {
					arr->v_array->vars = realloc(arr->v_array->vars, sizeof(struct variable*)*arr->v_array->num_vars+10);
				}
			} else {
				arr->v_array->num_vars++;
			}

			curr = curr->next;
		} else if (curr->type == INSERT_STMT) {

			assert(array_prep);
			assert(tmp_mem);
			assert(tmp_mem->type == VL_VAR);

			struct variable_list * arr = get_ref(curr_scope, f, curr->assign_name);
			assert(arr->type == VL_ARRAY);

			if (tmp_mem->v->type == INT_VAR) {

				if (arr->v_array->num_vars == 0) {
					assert(tmp_mem->v->int_value == 0);
					if (!arr->v_array->vars) {
						arr->v_array->vars = malloc(sizeof(struct variable*)*10);
					}
				}
				int idx;

				if (arr->v_array->num_vars == arr->v_array->total_vars) {
					arr->v_array->num_vars++;
					arr->v_array->total_vars++;

					if (arr->v_array->num_vars%10 == 0) {
						arr->v_array->vars = realloc(arr->v_array->vars, sizeof(struct variable*)*(arr->v_array->num_vars+10));
					}
				}
				for (idx = arr->v_array->num_vars-1; idx >= tmp_mem->v->int_value; idx++) {
					arr->v_array->vars[idx] = arr->v_array->vars[idx-1];
				}
				arr->v_array->vars[tmp_mem->v->int_value] = array_prep;
			}



		} else {
			fprintf(stderr, "%s: UNIMPLEMENTED STMT\n", __func__);
			exit(1);
		}
	}
	return NULL;
}

























