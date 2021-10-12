#ifndef PARSER_H
#define PARSER_H

#include "token.h"
#include "program.h"

int parse_program(struct token_a ** tokens, struct program * p, const int, const char **);
struct program * preparse_program(struct token_a ** tokens);

#endif
