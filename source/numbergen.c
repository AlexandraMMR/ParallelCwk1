#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

double fRand(double, double);

// THAT NUMBER GENERATION THO
int main() {
	int i;
	int dimension = 100;
	double val;

	FILE *valueFile;
	valueFile = fopen("valuesSmall.txt", "w");
	if (valueFile == NULL) {
		fprintf(stdout, "LOG ERROR - Failed to open file. Exiting program");
		return 1;
	}

	srand((unsigned)time(NULL));

	for (i = 0; i < dimension * dimension; i++) {
		val = fRand(1, 2);
		fprintf(valueFile, "%.5lf ", val);
	}
}

double fRand(double fMin, double fMax) {
    double f = (double)rand() / RAND_MAX;
    return fMin + f * (fMax - fMin);
}