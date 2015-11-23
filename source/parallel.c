#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <pthread.h>
#include <stdint.h>

#define ANSI_COLOR_RED     ""
#define ANSI_COLOR_RESET   ""

#define BILLION 1000000000L

pthread_mutex_t precisionLock;
pthread_barrier_t barrier;

double fRand(double, double);
void setValue(int, int);

struct RelaxData {
    int chunkStart, chunkEnd;
    double *values;
    double *newValues;
    int *withinPrecision;
    int dimension;
    double precision;
};

void* relaxArray(void *td) {
	struct RelaxData *data = (struct RelaxData*) td;

	int chunkStart = data->chunkStart;
	int chunkEnd = data->chunkEnd;
	double *values = data->values;
	double *newValues = data->newValues;
	int *withinPrecision = data->withinPrecision;
	int dimension = data->dimension;
	double precision = data->precision;
	int outOfPrecision = 1;

	while (outOfPrecision) {
		outOfPrecision = 0;

		int i;

		for (i = chunkStart; i < chunkEnd + 1; i++) {
			if ((i % dimension) == 0 || (i % dimension) == (dimension-1))
				continue;

			double above, below, left, right;
			// Get the neighbouring values for this index
			above = values[i-dimension];
			below = values[i+dimension];
			left = values[i-1];
			right = values[i+1];
			// Set the new value to the average of the neighbouring values
			newValues[i] = (above + below + left + right) / 4.0;

			if (fabs(values[i] - newValues[i]) > precision) {
				outOfPrecision = 1;
			}
		}
		
		if (outOfPrecision) {
			pthread_mutex_lock(&precisionLock);
			*withinPrecision = 0;
			pthread_mutex_unlock(&precisionLock);
		}

		// Wait until all the newValues are calculated
		pthread_barrier_wait(&barrier);

		if (*withinPrecision == 0) {
			outOfPrecision = 1; // So the while loop continues
			for (i = chunkStart; i < chunkEnd + 1; i++) {
				if ((i % dimension) == 0 || (i % dimension) == (dimension-1))
					continue;

				// Move relaxed numbers back into original arra
				values[i] = newValues[i];
			}

			// Wait until all the newValues have moved in to values
			pthread_barrier_wait(&barrier);

			// Update withinPrecision back to 1 to reset precision check
			if (*withinPrecision == 0) {
				pthread_mutex_lock(&precisionLock);
				*withinPrecision = 1;
				pthread_mutex_unlock(&precisionLock);
			}

			pthread_barrier_wait(&barrier);
		}
					
	}

	return NULL;
}

int main(int argc, char *argv[]) {

	/* Values hard coded - ensure to update
	 * debug - The level of debug output: 0, 1, 2
	 * cores - number of cores to use for the program
	 * dimension - how big the square array is
	 * precision - how precise the relaxation needs to be before the program ends
	 *
	 * generateNumbers - 0 to use values in setValues[][], 1 to generate them randomly
	 * textFile[] - text file to read numbers from. Needs to be set and filled in if generateNumbers == 0
	 * 				needs to contain at least dimension*dimension numbers, can contain more but not less
	 */

	int debug = 0; /* Debug output: 0 no detail - 1 some detail - 2 all detail */

	int cores = 4; 
	int dimension = 10;
	double precision = 0.0000000001;

	int generateNumbers = 1;
	// textFile needs to be set and filled in if generateNumbers == 0
	char textFile[] = "scratch/valuesSmall.txt";
	
	/* End editable values */

	uint64_t diff;
	struct timespec start, end;
	clock_gettime(CLOCK_MONOTONIC, &start);

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
		} else if (strcmp(argv[a], "-f") == 0 || strcmp(argv[a], "-filepath") == 0) {
			if (a + 1 <= argc - 1) { /* Make sure we have more arguments */
				a++;
				strncpy(textFile, argv[a], sizeof(textFile));
			}
		} else {
			/* Non optional arguments here, but we have none of those */
		}
	}

	FILE *valueFile;

	if (!generateNumbers) {
		valueFile = fopen(textFile, "r");
		if (valueFile == NULL) {
			fprintf(stdout, "LOG ERROR - Failed to open file: %s. Exiting program", textFile);
			return 1;
		}
	}

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
	if (cores < 1) cores = 1;
	if (precision < 0.0000000001) precision = 0.0000000001;

	if (debug >= 1) {
		fprintf(stdout, "LOG FINE - Using %d cores.\n", cores);
		fprintf(stdout, "LOG FINE - Using array of dimension %d.\n", dimension);
		fprintf(stdout, "LOG FINE - Working to precision of %.10lf.\n", precision);
	}

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

	// init precision lock
	if (pthread_mutex_init(&precisionLock, NULL) != 0) {
        printf("\n mutex init failed\n");
        return 1;
    }

	/* Make threads & distribute workload */

    pthread_barrier_init(&barrier, NULL, cores);

	pthread_t thread[cores];
	struct RelaxData data[cores];

	int chunks = (dimension-2)*(dimension-2);
	int chunksPerCore = chunks/cores;
	int remain = chunks % cores;
	
	if (debug >= 2) {
		fprintf(stdout, "\nLOG FINEST - Calculations required: %d\n", chunks);
		fprintf(stdout, "LOG FINEST - Chunking to size: %d\n", chunksPerCore);
	}

	

	/* End thread making */

	int curIndex = 0;
	int chunkToGive;

	int withinPrecision = 0;

	for (i = 0; i < cores; i++) {
		chunkToGive = chunksPerCore;
		if (remain > 0) { 
			chunkToGive++;
			remain--;
		}

		/* curIndex only refers to the actionable array */
		/* Perform arithmetic to convert position in actionable array to position in full array */
		data[i].chunkStart = curIndex + ((curIndex / (dimension-2)) * 2) + (dimension+1);
		data[i].chunkEnd = curIndex + (chunkToGive - 1) + 
							(((curIndex + (chunkToGive - 1)) / (dimension-2)) * 2) + (dimension+1);
		
		curIndex = curIndex + chunkToGive;

		data[i].values = values;
		data[i].newValues = newValues;
		data[i].withinPrecision = &withinPrecision;
		data[i].dimension = dimension;
		data[i].precision = precision;

		pthread_create(&thread[i], NULL, relaxArray, &data[i]);
	}


	// Wait for all threads to finish
	for (i = 0; i < cores; i++)
		pthread_join(thread[i], NULL);


	// Swap pointers, so we have the final array in values
	double *tempValues = values;
	values = newValues;
	newValues = tempValues;

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
	free(newValues);


	clock_gettime(CLOCK_MONOTONIC, &end);	/* mark the end time */

	diff = BILLION * (end.tv_sec - start.tv_sec) + end.tv_nsec - start.tv_nsec;
	if (debug >= 1) printf("LOG FINE - Completed in %llu Nanoseconds\n",  (long long unsigned int) diff);
	
}

double fRand(double fMin, double fMax) {
    double f = (double)rand() / RAND_MAX;
    return fMin + f * (fMax - fMin);
}
