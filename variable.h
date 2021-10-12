
#ifndef VARIABLE_H
#define VARIABLE_H

enum var_type {
	FLOAT_VAR, INT_VAR, CHAR_VAR, STRING_VAR
};

struct variable {
	enum var_type type;
	char * name;

	int int_value;
	float float_value;
	char char_value;
	char * string_value;
};

#endif
