

#include <stdio.h>
#include <stdlib.h>
#include <math.h>


int add_int(int a, int b) {
	return a + b;
}

float add_float(float a, float b) {
	return a + b;
}

float add_fl_ir(float l, int r) {
	return l + r;
}

float add_il_fr(int l, float r) {
	return l + r;
}

int sub_int(int l, int r) {
	return l - r;
}

float sub_float(float l, float r) {
	return l - r;
}

float sub_fl_ir(float l, int r) {
	return l - r;
}

float sub_il_fl(int l, float r) {
	return l - r;
}

int mult_int(int l, int r) {
	return l*r;
}

float mult_float(float l, float r) {
	return l*r;
}

float mult_fl_ir(float l, int r) {
	return l*r;
}

float mult_il_fr(int l, float r) {
	return l*r;
}


int div_int(int l, int r) {
	if (r == 0) {
		fprintf(stderr, "ERR: DIV BY 0\n");
		exit(1);
	}
	return l/r;
}

float div_float(float l, float r) {
	return l/r;
}

float div_fl_ir(float l, int r) {
	return l/(float)r;
}

float div_il_fr(int l, float r) {
	return (float)l/r;
}


