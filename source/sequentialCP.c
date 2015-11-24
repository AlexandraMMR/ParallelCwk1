#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <stdint.h>

#define ANSI_COLOR_RED     "\x1b[31m"
#define ANSI_COLOR_RESET   "\x1b[0m"

#define BILLION 1000000000L

double fRand(double, double);
void setValue(int, int);

int main(int argc, char *argv[]) {

	/* Values hard coded - ensure to update
	 * debug - The level of debug output: 0, 1, 2
	 * cores - number of cores to use for the program
	 * dimension - how big the square array is
	 * precision - how precise the relaxation needs to be before the program ends
	 *
	 * generateNumbers - 0 to use values in setValues[][], 1 to generate them randomly
	 * textFile[] - text file to read numbers from. Needs to be set and filled in if generateNumbers == 0
	 */

	int debug = 0; /* Debug output: 0 no detail - 1 some detail - 2 all detail */

	int cores = 1; 
	int dimension = 500;
	double precision = 0.1;

	int generateNumbers = 0;
	// textFile needs to be set and filled in if generateNumbers == 0
	char textFile[] = "values.txt";
	
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
		} else if (strcmp(argv[a], "-g") == 0 || strcmp(argv[a], "-generate") == 0) {
			if (a + 1 <= argc - 1) { /* Make sure we have more arguments */
				if (atoi(argv[a+1]) >= 0) {
					a++;
					generateNumbers = atoi(argv[a]);
				} else {
					fprintf(stderr, "LOG WARNING - Invalid argument for -g. Integer >= 0 required. Using %d dimension as default.\n", dimension);
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

	FILE *valueFile;

	if (!generateNumbers) 
		valueFile = fopen(textFile, "r");

	/* Set up two arrays, one to store current results & one to store changes */
	double *values = malloc(dimension * dimension * sizeof(double));
	double *newValues = malloc(dimension * dimension * sizeof(double));
	
	srand((unsigned)time(NULL));

	/* Put numbers into the value arrays */
	int i, j;
	for (i = 0; i < dimension; i++) {
		for (j = 0; j < dimension; j++) {
			// If we're going to generate the numbers, or use predetermined ones
			if (generateNumbers)
				values[i*dimension+j] = fRand(1, 2);
			else
				fscanf(valueFile, "%lf", &values[i*dimension+j] );
			// And copy them to the new array as well
			newValues[i*dimension+j] = values[i*dimension+j];
		}
	}
	if (precision < 0.0000000001) precision = 0.0000000001;
	if (dimension < 3) dimension = 3;
	
	if (debug >= 1) {
		fprintf(stdout, "LOG FINE - Using %d cores.\n", cores);
		fprintf(stdout, "LOG FINE - Using array of dimension %d.\n", dimension);
		fprintf(stdout, "LOG FINE - Working to precision of %.10lf.\n", precision);
	}

	int count = 0; // Count how many times we try to relax the square array
	int withinPrecision = 0; // 1 when pass entirely completed within precision, i.e. finished

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

	while (!withinPrecision) {
		count++;
		withinPrecision = 1;
		// Outside line of square array will remain static so skip it
		for (i = 1; i < dimension - 1; i++) { // Skip top and bottom
			for (j = 1; j < dimension - 1; j++) { // Skip left and right
				// Store relaxed number into new array
				newValues[i*dimension+j] = (values[(i-1)*dimension+j] + values[(i+1)*dimension+j] 
							  + values[i*dimension+(j-1)] + values[i*dimension+(j+1)]) / 4.0;
				/* If the numbers changed more than precision, we need to do it again */
				if (fabs(values[i*dimension+j] - newValues[i*dimension+j]) > precision) {
					withinPrecision = 0;
				}
			}
		}
		// Swap pointers, so we can continue working on the new array
		double *tempValues = values;
		values = newValues;
		newValues = tempValues;
	}
	/* Switch the pointers around to move the new list to the currently active list */

	if (debug >= 1) 
		fprintf(stdout, "\nLOG FINE - Program complete. Relaxation count: %d.\n", count);
	if (debug >= 2) {
		fprintf(stdout, "LOG FINEST - Final array:\n");
		for (i = 0; i < dimension; i++) {
			for (j = 0; j < dimension; j++) {
				if (i == 0 || i == dimension-1 || j == 0 || j == dimension -1)
					fprintf(stdout, ANSI_COLOR_RED);
				fprintf(stdout, "%.10lf " ANSI_COLOR_RESET, values[i*dimension+j]);
			}
			fprintf(stdout, "\n");
		}
	}

	free(values);
	free(newValues);

	fprintf(stdout, "Program complete.\n");
}

double fRand(double fMin, double fMax) {
    double f = (double)rand() / RAND_MAX;
    return fMin + f * (fMax - fMin);
}

/* Image 2 threads
 * Give a's to 1, o's to 2
 * Go through and compute, updating new array with computed result (new array each, or new array both write to)
 * 
 *
 *
 */




/* Checkboard for parallelising 
 * a's and o's never interact
 *
 *	x x x x x x x x x x
 *	x o o o o o o o o x
 *	x o o o o o o o o x
 *	x o o o o o o o o x
 *	x o o o o o o o o x
 *	x o o o o o o o o x
 *	x o o o o o o o o x
 *	x o o o o o o o o x
 *	x o o o o o o o o x
 *	x x x x x x x x x x
 *
 * - Do all a's then all o's
 * - Do diagonals
 * - Do radial?
 * - Others?
 *
 * n dimensions? Talk about, rather than implement
 */

