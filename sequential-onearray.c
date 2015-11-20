#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

#define ANSI_COLOR_RED     "\x1b[31m"
#define ANSI_COLOR_RESET   "\x1b[0m"

double fRand(double, double);
void setValue(int, int);

int main(int argc, char *argv[]) {

	/* Values hard coded - ensure to update
	 * cores - number of cores to use for the program
	 * dimension - how big the square array is
	 * values[][] - change numbers in brackets to match dimension value
	 * precision - how precise the relaxation needs to be before the program ends
	 */

	int cores = 1; 
	int dimension = 6;
	double precision = 0.0000000000000000000000000000000000000001;

	int debug = 0; /* Debug output: 0 no detail - 1 some detail - 2 all detail */
	
	/* End editable values */

	/* Parse command line input */
	int a;
	for (a = 1; a < argc; a++) { /* argv[0] is program name */
		if (strcmp(argv[a], "-c") == 0 || strcmp(argv[a], "-cores") == 0) { // Value of 0 means strings are identical
			if (a + 1 <= argc - 1) { /* Make sure we have more arguments */
				if (atoi(argv[a+1]) > 0) {
					a++;
					cores = atoi(argv[a]);
				} else {
					fprintf(stderr, "LOG WARNING - Invalid argument for -c. Positive integer required. Using %d cores as default.\n", cores);
				}
			}
		} else if (strcmp(argv[a], "-d") == 0 || strcmp(argv[a], "-dimension") == 0) {
			if (a + 1 <= argc - 1) { /* Make sure we have more arguments */
				if (atoi(argv[a+1]) > 0) {
					a++;
					dimension = atoi(argv[a]);
				} else {
					fprintf(stderr, "LOG WARNING - Invalid argument for -d. Positive integer required. Using %d dimension as default.\n", dimension);
				}
			}
		} else if (strcmp(argv[a], "-p") == 0 || strcmp(argv[a], "-precision") == 0) {
			if (a + 1 <= argc - 1) { /* Make sure we have more arguments */
				if (atof(argv[a+1]) > 0.0) {
					a++;
					precision = atof(argv[a]);
				} else {
					fprintf(stderr, "LOG WARNING - Invalid argument for -p. Positive double required. Using %f precision as default.\n", precision);
				}
			}
		} else if (strcmp(argv[a], "-debug") == 0) {
			if (a + 1 <= argc - 1) { /* Make sure we have more arguments */
				if (atoi(argv[a+1]) >= 0) {
					a++;
					debug = atoi(argv[a]);
				} else {
					fprintf(stderr, "LOG WARNING - Invalid argument for -debug. Integer >= 0 required. Using %d debug as default.\n", debug);
				}
			}
		} else {
			/* Non optional arguments here, but we have none of those */
		}
	}


	double *values = malloc(dimension * dimension * sizeof(double));
	
	srand((unsigned)time(NULL));

	/* Generate random numbers and put them into values */
	int i, j;
	for (i = 0; i < dimension; i++) {
		for (j = 0; j < dimension; j++) {
			values[i*dimension+j] = fRand(1, 2);
		}
	}


	if (debug >= 1) {
		fprintf(stdout, "LOG FINE - Using %d cores.\n", cores);
		fprintf(stdout, "LOG FINE - Using array of dimension %d.\n", dimension);
		fprintf(stdout, "LOG FINE - Working to precision of %.40lf.\n", precision);
	}

	int count = 0; // Count how many times we try to relax the square array
	int withinPrecision = 0; // 1 when pass entirely completed within precision, i.e. finished
	int guard = 0;

	if (debug >= 2) {
	/* Display initial array for debugging */
		fprintf(stdout, "LOG FINEST - Working with array:\n");
		for (i = 0; i < dimension; i++) {
			for (j = 0; j < dimension; j++) {
				if (i == 0 || i == dimension-1 || j == 0 || j == dimension -1)
					fprintf(stdout, ANSI_COLOR_RED);
				fprintf(stdout, "%f " ANSI_COLOR_RESET, values[i*dimension+j]);
			}
			fprintf(stdout, "\n");
		}
	}

	while (!withinPrecision && guard < 200000) {
		count++;
		withinPrecision = 1;
		// Outside line of square array will remain static so skip it
		for (i = 1; i < dimension - 1; i++) { // Skip top and bottom
			for (j = 1; j < dimension - 1; j++) { // Skip left and right
				double curVal = values[i*dimension+j];
				values[i*dimension+j] = (values[(i-1)*dimension+j] + values[(i+1)*dimension+j] 
							  + values[i*dimension+(j-1)] + values[i*dimension+(j+1)]) / 4;
				/* If the numbers changed more than precision, we need to do it again */
				if (fabs(curVal - values[i*dimension+j]) > precision) {
					withinPrecision = 0;
				}
			}
		}
		guard++;
	}

	if (debug >= 1) 
		fprintf(stdout, "\nLOG FINE - Program complete. Relaxation count: %d.\n", count);
	if (debug >= 2) {
		fprintf(stdout, "LOG FINEST - Final array:\n");
		for (i = 0; i < dimension; i++) {
			for (j = 0; j < dimension; j++) {
				if (i == 0 || i == dimension-1 || j == 0 || j == dimension -1)
					fprintf(stdout, ANSI_COLOR_RED);
				fprintf(stdout, "%f " ANSI_COLOR_RESET, values[i*dimension+j]);
			}
			fprintf(stdout, "\n");
		}
	}

	free(values);

	fprintf(stdout, "Program complete.\n");
}

double fRand(double fMin, double fMax) {
    double f = (double)rand() / RAND_MAX;
    return fMin + f * (fMax - fMin);
}

/* Imagine 2 threads
 * Give a's to 1, o's to 2
 * Go through and compute, updating new array with computed result (new array each, or new array both write to)
 *
 *
 *
 */




/* Checkboard for parallelising 
 * a's and o's never interact
 *
 *	x x x x x x
 *	x a o a o x
 *	x o a o a x
 *	x a o a o x
 *	x o a o a x
 *	x x x x x x
 *
 * - Do all a's then all o's
 * - Do diagonals
 * - Do radial?
 * - Others?
 *
 * n dimensions? Talk about, rather than implement
 */

