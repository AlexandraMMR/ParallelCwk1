# Parallel Computing - Coursework 1
The first coursework for Final Year Bath Comp Sci, Parallel Computing unit

This zip contains 2 folders:
Source, Testing

Source - the actual compilable .c code
Testing - the testing document and the values utilised by it (code run times etc)


Compile sequential.c or parallel.c using gcc -Wall -pthread filename.c -lrt

Run the program using ./filename, and possible flags:
-debug : The level of debug output: 0, 1, 2
-c : number of threads to use for the program
-d : length of the square array
-p : how precise the relaxation needs to be before the program ends
-g : (1 or 0) 0 to use values in from file specified in program, 1 to generate them randomly
-f : string, path of the textfile to use

For example: ./parallel -debug 2 -c 16 -d 500 -p 0.01 -g 0 -f values.txt

The values must first be computed initially by editing numbergen.c to specify dimension size and output file, then compiling and running. Computing a file with dimension 1000 can be used by any program for any dimension <= 1000, but not for those with > 1000.
