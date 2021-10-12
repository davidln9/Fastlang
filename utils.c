
#include <stdio.h>
#include <stdlib.h>


#include "function.h"
#include "stmt.h"

void free_function(struct function * f) {

	int i;
	for (i = 0; i < f->num_args; i++) {
		free(f->args[i]);
	}
	if (f->args) {
		free(f->args);
	}
	struct stmt * curr = f->start, *prev;

	while (curr) {
		prev = curr;
		curr = curr->next;
		free(prev);
	}
}
