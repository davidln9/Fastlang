#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "token.h"


#define BUF_SIZE 512

static int is_num(char c);
static struct token * first_token = NULL;



static int add_token(struct token * token) {

	struct token * curr = first_token, * prev = NULL;
        while (curr) {
                prev = curr;
                curr = curr->next;
        }

        if (prev) {
                prev->next = token;
        } else {
                first_token = token;
        }
	return 0;
}

	
static int get_token(char * line, int last_space, int index) {

	char * text = malloc(index - last_space + 1);
	text[0] = '\0';
	strncpy(text, line + last_space, index - last_space);
	struct token * token = malloc(sizeof(struct token));

	token->next = NULL;
	token->value = text;
	if (!strcmp(text, "continue")) {
		token->type = CONTINUE;
	} else if (!strcmp(text, "break")) {
		token->type = BREAK;
	} else if (!strcmp(text, "for")) {
		token->type = FOR;
	} else if (!strcmp(text, "while")) {
		token->type = WHILE;
	} else if (!strcmp(text, "print")) {
		token->type = PRINT;
	} else if (!strcmp(text, "println")) {
		token->type = PRINTLN;
	} else if (!strcmp(text, "eprint")) {
		token->type = EPRINT;
	} else if (!strcmp(text, "eprintln")) {
		token->type = EPRINTLN;
	} else if (!strcmp(text, "len")) {
		token->type = LEN;
	} else if (!strcmp(text, "if")) {
		token->type = IF;
	} else if (!strcmp(text, "array_push")) {
		token->type = PUSH;
	} else if (!strcmp(text, "array_pop")) {
		token->type = POP;
	} else if (!strcmp(text, "array_insert")) {
		token->type = INSERT;
	} else if (!strcmp(text, "array_remove")) {
		token->type = REMOVE;
	} else if (!strcmp(text, "int")) {
		token->type = KEYWORD_INT;
	} else if (!strcmp(text, "float")) {
		token->type = KEYWORD_FLOAT;
	} else if (!strcmp(text, "string")) {
		token->type = KEYWORD_STRING;
	} else if (!strcmp(text, "else")) {
		token->type = ELSE;
	} else if (!strcmp(text, "run")) {
		token->type = RUN;
	} else if (!strcmp(text, "stop")) {
		token->type = STOP;
	} else if (!strcmp(text, "loop")) {
		token->type = LOOP;
	} else if (!strcmp(text, "+")) {
		token->type = PLUS;
	} else if (!strcmp(text, "-")) {
		token->type = MINUS;
	} else if (!strcmp(text, "*")) {
		token->type = MULT;
	} else if (!strcmp(text, "/")) {
		token->type = DIV;
	} else if (!strcmp(text, "%")) {
		token->type = MOD;
	} else if (!strcmp(text, "|")) {
		token->type = BIN_OR;
	} else if (!strcmp(text, "&")) {
		token->type = BIN_AND;
	} else if (!strcmp(text, "#")) {
		token->type = POUND;
	} else if (!strcmp(text, "(")) {
		token->type = LPAREN;
	} else if (!strcmp(text, ")")) {
		token->type = RPAREN;
	} else if (!strcmp(text, "[")) {
		token->type = LBRACKET;
	} else if (!strcmp(text, "]")) {
		token->type = RBRACKET;
	} else if (!strcmp(text, "{")) {
		token->type = LBRACE;
	} else if (!strcmp(text, "}")) {
		token->type = RBRACE;
	} else if (!strcmp(text, "goto")) {
		token->type = GOTO;
	} else if (!strcmp(text, "!")) {
		token->type = NOT;
	} else if (!strcmp(text, "@")) {
		token->type = AMPERSAND;
	} else if (!strcmp(text, "=")) {
		token->type = EQUALS;
	} else if (!strcmp(text, "$")) {
		token->type = DOLLAR;
	} else if (!strcmp(text, ",")) {
		token->type = COMMA;
	} else if (!strcmp(text, "char")) {
		token->type = KEYWORD_CHAR;
	} else if (!strcmp(text, ";")) {
		token->type = SEMICOLON;
	} else if (!strcmp(text, ":")) {
		token->type = COLON;
	} else if (!strcmp(text, "<")) {
		token->type = LT;
	} else if (!strcmp(text, ">")) {
		token->type = GT;
	} else if (!strcmp(text, "`")) {
		token->type = MYSQL_QUOTE;
	} else if (!strcmp(text, "~")) {
		token->type = TILDE;
	} else if (!strcmp(text, "return")) {
		token->type = RETURN;
	} else if (!strcmp(text, "include")) {
		token->type = INCLUDE;
	} else if (!strcmp(text, "define")) {
		token->type = DEFINE;
	} else if (!strcmp(text, "ifdef")) {
		token->type = IFDEF;
	} else if (!strcmp(text, "ifndef")) {
		token->type = IFNDEF;
	} else if (!strcmp(text, "void")) {
		token->type = KEYWORD_VOID;
	} else {
		int is_int = 1, is_float = 0;
		if (token->value[0] == '-') {
			for (int i = 1; i < strlen(token->value); i++) {
				if (!is_num(token->value[i])) {	
					is_int = 0;
					is_float = 1;
					break;
				}
			}
			if (is_int) {
				token->type = INT;
				add_token(token);
				return 0;
			}
			int found_dot = 0;
			for (int i = 1; i < strlen(token->value); i++) {
				if (!is_num(token->value[i])) {
					if (found_dot) {
						is_float = 0;
						fprintf(stderr, "%s:%d: invalid token: %s\n", __FILE__, __LINE__, token->value);
						exit(1);
					}
					if (token->value[i] == '.') {
						found_dot = 1;
					} else {
						fprintf(stderr, "%s:%d: invalid token: %s\n", __FILE__, __LINE__, token->value);
						exit(1);
					}
				}
			} 
			if (is_float) {
				token->type = FLOAT;
				add_token(token);
				return 0;
			}
			fprintf(stderr, "%s:%d: invalid token: %s\n", __FILE__, __LINE__, token->value);
			exit(1);
		} else if (is_num(token->value[0])) {
			int is_int = 1, is_float = 0;
			for (int i = 0; i < strlen(token->value); i++) {
				if (!is_num(token->value[i])) {
					is_int = 0;
					is_float = 1;
					break;
				}
			}
			if (is_int) {
				token->type = INT;
				add_token(token);
				return 0;
			}
			int found_dot = 0;
			for (int i = 0; i < strlen(token->value); i++) {
				if (!is_num(token->value[i])) {
					if (found_dot) {
						fprintf(stderr, "%s:%d: invalid token: %s\n", __FILE__, __LINE__, token->value);
						exit(1);
					}
					if (token->value[i] == '.') {
						found_dot = 1;
					} else {
						fprintf(stderr, "%s:%d: invalid token: %s\n", __FILE__, __LINE__, token->value);
						exit(1);
					}
				}
			}
			if (is_float) {
				if (token->value[strlen(token->value)-1] != '.') {
					token->type = FLOAT;
					add_token(token);
					return 0;
				} else {
					fprintf(stderr, "%s:%d: invalid token: %s\n", __FILE__, __LINE__, token->value);
					exit(1);
				}
			}
			fprintf(stderr, "%s:%d: invalid token: %s\n", __FILE__, __LINE__, token->value);
			exit(1);
		} else {
			token->type = IDENTIFIER;
			add_token(token);
			return 0;
		}


	}

	add_token(token);
	return 0;
}

static int is_num(char c) {
	return c == '0' || c == '1' || c == '2' ||
		c == '3' || c == '4' || c == '5' ||
		c == '6' || c == '7' || c == '8' ||
		c == '9';
}

	



static int get_tokens_from_line(char * line) {

#ifdef DEBUG
	printf("enter get_tokens_from_line: %s\n", line);
#endif
	int i, last_space = 0;
	char word[BUF_SIZE];
	memset(word, 0, sizeof(word));

	int token_flag = 0, parse_double_quote_string = 0, parse_single_quote_string = 0;
	for (i = 0; i < strlen(line); i++) {
		if (!parse_double_quote_string && !parse_single_quote_string) {
			if (line[i] == ' ' || line[i] == '\t') {
				if (token_flag) {
					get_token(line, last_space, i);
					token_flag = 0;
				}
				last_space = i + 1;
			} else if (line[i] == '(' || line[i] == ')' || line[i] == '{' || 
					line[i] == '}' || line[i] == '[' || line[i] == ']' ||
					line[i] == '|' || line[i] == '&' || line[i] == '+' ||
					line[i] == '-' || line[i] == '!' || line[i] == '@' || 
					line[i] == '#' || line[i] == '%' || line[i] == '^' ||
					line[i] == '=' || line[i] == ',' || line[i] == '*' ||
					line[i] == '`' || line[i] == '~' || line[i] == ';' ||
					line[i] == '>' || line[i] == '<' || line[i] == '/') {

				if (token_flag) {
					get_token(line, last_space, i);
					token_flag = 0;
				}
				if (line[i] == '-' && line[i+1] == '>') {
					struct token * t = malloc(sizeof(struct token));
					t->type = ARROW;
					t->value = malloc(3);
					t->value[0] = '\0';
					strcpy(t->value, "->");
					add_token(t);
					last_space = i+2;
					i++;
				} else if (line[i] == '>' && line[i] == '=') {
					struct token * t = malloc(sizeof(struct token));
					t->type = GTEQ;
					t->value = malloc(3);
					t->value[0] = '\0';
					strcpy(t->value, ">=");
					add_token(t);
					last_space = i+2;
					i++;
				} else if (line[i] == '<' && line[i+1] == '=') {
					struct token * t = malloc(sizeof(struct token));
					t->type = LTEQ;
					t->value = malloc(3);
					t->value[0] = '\0';
					strcpy(t->value, "<=");
					add_token(t);
					last_space = i+2;
					i++;
				} else if (line[i] == '=' && line[i+1] == '=') {
					struct token * t = malloc(sizeof(struct token));
					t->type = EQEQ;
					t->value = malloc(3);
					t->value[0] = '\0';
					strcpy(t->value, "==");
					add_token(t);
					last_space = i+2;
					i++;
				} else if (line[i] == '!' && line[i+1] == '=') {
					struct token * t = malloc(sizeof(struct token));
					t->type = NEQ;
					t->value = malloc(3);
					t->value[0] = '\0';
					strcpy(t->value, "!=");
					add_token(t);
					last_space = i+1;
					i++;
				} else {
					get_token(line, i, i+1);
					last_space = i + 1;
				}
			} else if (line[i] == '\"') {
				parse_double_quote_string = 1;
				last_space = i+1;
			} else if (line[i] == '\'') {
				parse_single_quote_string = 1;
				last_space = i+1;
			} else {
				token_flag = 1;
			}
		} else if (parse_double_quote_string) {
			if (line[i] == '\"') {
				parse_double_quote_string = 0;
				struct token * t = malloc(sizeof(struct token));
				t->value = malloc(i-last_space+1);

				int j, idx = 0;
				for (j = last_space; j < i; j++) {
					if (line[j] == '\\') {
						switch (line[j+1]) {
							case 'n':
								t->value[idx] = '\n';
								break;
							case 't':
								t->value[idx] = '\t';
								break;
							case 'r':
								t->value[idx] = '\r';
								break;
							case '0':
								t->value[idx] = '\0';
								break;
							default:
								fprintf(stderr, "%s: unknown escape esquence '\\%c'\n", __func__, line[j+1]);
								exit(1);
						}
						j++;
					} else {
						t->value[idx] = line[j];
					}
					idx++;
				}
				t->value[idx] = '\0';

				t->type = DOUBLE_QUOTE_STRING;
				add_token(t);
				last_space = i+1;
			}
		} else if (parse_single_quote_string) {
			if (line[i] == '\'') {
				parse_single_quote_string = 0;
				struct token * t = malloc(sizeof(struct token));
				t->value = malloc(i-last_space+1);
				t->value[0] = '\0';
				int j;
				int idx = 0;;
				for (j = last_space; j < i; j++) {
					if (line[j] == '\\') {
						switch (line[j+1]) {
							case 'n':
								t->value[idx] = '\n';
								break;
							case 't':
								t->value[idx] = '\t';
								break;
							case 'r':
								t->value[idx] = '\r';
								break;
							case '0':
								t->value[idx] = '\0';
								break;
							default:
								fprintf(stderr, "%s: unknown escape sequence: '\\%c'\n", __func__, line[j+1]);
								exit(1);
						}
						j++;
					} else {
						t->value[idx] = line[j];
					}
					idx++;
				}
				t->value[idx] = '\0';
				t->type = SINGLE_QUOTE_STRING;
				add_token(t);
				last_space = i+1;
			}
		}
	}

	if (token_flag) {
		get_token(line, last_space, i);
	}
	return 0;
}

struct token *get_token_list(const char * filename) {

	FILE * fp = fopen(filename, "r");

	char line_buf[BUF_SIZE];
	int index = 0;
	memset(line_buf, 0, sizeof(line_buf));
	char * line = NULL;
	while (!feof(fp)) {
		char c = fgetc(fp);
		if (c == '\n') {
			if (index > BUF_SIZE) {
				if (line) {
					line = realloc(line, index);
					strcpy(line, line_buf);
				} else {
					line = malloc(index);
					line[0] = '\0';
					strcpy(line, line_buf);
				}
				get_tokens_from_line(line);
				memset(line_buf, 0, sizeof(line_buf));
				free(line);
				line = NULL;
			} else if (index > 0) {
				get_tokens_from_line(line_buf);
				memset(line_buf, 0, sizeof(line_buf));
			}
			index = 0;


		} else {
			line_buf[index%BUF_SIZE] = c;
			index++;
			if (index%BUF_SIZE == 0) {
				if (line) {
					line = realloc(line, index);
					strcpy(line, line_buf);
				} else {
					line = malloc(index);
					line[0] = '\0';
					strcpy(line, line_buf);
				}
			}
		}
	}

	fclose(fp);

	return first_token;
}



