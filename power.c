
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "power.h"


int power_int(int l, int r) {

	int ret = 1;
	int i;
	for (i = 0; i < r; i++) {
		ret = ret * l;
	}
	return ret;
}

static int gcfFinder(int a, int b) {
	int gcf = 1;
	for (int i = 1; i <= a && i <= b; i++)
	{
		if (a % i == 0 && b % i == 0)
		{
			gcf = i;
		}
	}
	return gcf;
}

static int shortform(int* a, int* b) {
	for (int i = 2; i <= *a && i <= *b; i++) {
		if (*a % i == 0 && *b % i == 0) {
			*a = *a / i;
			*b = *b / i;
		}
	}
	return 0;
}

struct fraction {

	int top;
	int bottom;
};

static struct fraction float_to_fraction(float a) {

	int c = 10000;
	float b = (a - floor(a)) * c;
	int d = (int)floor(a) * c + (int)(b + .5f);

	while (1)
	{
		if (d % 10 == 0)
		{
			d = d / 10;
			c = c / 10;
		}
		else
			break;
	}
	int* i = &d;
	int* j = &c;
	while (1) {
		int gcf = gcfFinder(d, c);
		if (gcf == 1)
		{
			struct fraction f;
			f.top = d;
			f.bottom = c;
			return f;
		}
		else
		{
			shortform(i, j);
		}
	}
}

float power_il_fr(int l, float r) {

	struct fraction f = float_to_fraction(r);

	int ld, i;
	int td = ld = 1;

	for (i = 0; i < f.top ; i++) {
		td = td * l;
	}

	for (i = 0; i < f.bottom; i++) {
		ld = ld * l;
	}

	return (float)td/(float)ld;
}

float power_fl_fr(float l, float r) {

	struct fraction f = float_to_fraction(r);

	float ld;
	float td = ld = 1;

	int i;
	for (i = 0; i < f.top; i++) {
		td = td * l;
	}

	for (i = 0; i < f.bottom; i++) {
		ld = ld * l;
	}

	return td/ld;
}

float power_fl_ir(float l, int r) {

	float ret = 1.0;
	int i;

	for (i = 0; i < r; i++) {
		ret = ret * l;
	}

	return ret;
}
