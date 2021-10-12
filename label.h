
#ifndef LABEL_H
#define LABEL_H

#include "stmt.h"

struct label {
	char * name;
	struct stmt * landing;
	struct label * next;
};

#endif
