
#ifndef CONVERTER_H
#define CONVERTER_H

#include "token.h"
#include "variable.h"
#include "function.h"
#include "array.h"

struct variable ** convert_vl_to_array(struct variable_list * start, unsigned long * num);
struct token_a ** convert_token_list_to_array(struct token * start);
struct variable_list ** convert_variable_list_to_array(struct variable_list * start, unsigned long * num);
struct function ** convert_function_list_to_array(struct function_list * start, unsigned long * num);

#endif /* FUNCTION_H */
