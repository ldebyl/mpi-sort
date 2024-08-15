/********************************************************************
 * CITS5007 High Performance Computing
 * University of Western Australia
 * Assignment 2
 *
 * Lee de Byl (10306807@student.uwa.edu.au
 * October 2021
 *
 * Main entry point for Assignment 2.
 * Will be built in both serial and parallel versions by
 * the included Makefile.
 *******************************************************************/

#include "sort.h"

/* Prints usage to stdout */
void
usage()
{
	puts("High Performance Computing - Assignment 2");
    puts("October 2021");
    puts("Lee de Byl (10306807).");
	puts("");
	puts("Usage:");
	puts("   -a <algorithm>");
	puts("     Specifies the sort algorithm to use.");
	puts("       ENUMERATION        Enumeration Sort - No Loop Threading");
    puts("       ENUMERATION_INNER  Enumeration Sort - Inner Loop Threading");
    puts("       ENUMERATION_OUTER  Enumeration Sort - Outer Loop Threading");
	puts("       MERGE              Merge Sort");
	puts("       QUICK              Quick Sort (Default)");
	puts("");
    puts("   -r <method>: Specifies the method used to disseminate the array");
    puts("                to individual processes.");
    puts("      BCAST     The array is generated on the root process, and broadcast");
    puts("                to individual processes.");
    puts("      SHARED_SEED");
    puts("                The random number seed is broadcast to each process,");
    puts("                and each process generates its own version of the array.");
    puts("   -n <number>: Specifies the number of numbers to generate and sort.");
	puts("   -o:          Enables the results of the sort to be output to stdout.");
	puts("   -v:          Validates the sort results (in serial).");
	puts("   -t:          Number of Threads.");
	puts("   -l:          Number of Levels.");
    puts("   -O <path>:   Write the array to a binary file.");
    puts("   -V:          Validate the written array. Requires -O.");
}

/*
 * Given two timespec structs, calcualtes the difference in seconds between
 * them.
 */
double
timespec_diff(const struct timespec time1, const struct timespec time0)
{
	return (time1.tv_sec - time0.tv_sec)
	+ (time1.tv_nsec - time0.tv_nsec) / 1e9;
}

/*
 * Parses command line arguments and returns a struct represneting the parsed
 * options. Uses the defaults in config.h for any unspecified options.
 */
opts
parse_args(int argc, char **argv, bool enable_output)
{
	opts		o;
	char		opt;

    // Set default options
	o.algorithm = DEFAULT_ALGORITHM;
	o.n_numbers = DEFAULT_N;
	o.enable_output = DEFAULT_ENABLE_OUTPUT;
	o.validate_sort = DEFAULT_VALIDATE_SORT;
	o.n_threads = DEFAULT_THREADS;
	o.n_levels = DEFAULT_LEVELS;
    o.validate_array = DEFAULT_VALIDATE_ARRAY;
    o.write_array = DEFAULT_WRITE_ARRAY;
    o.array_file = DEFAULT_ARRAY_FILE;
    o.random_method = DEFAULT_RANDOM_METHOD;;
    o.parameters_invalid=false;

	//Parse command line arguments
		while ((opt = getopt(argc, argv, "t:n:a:l:oO:vVmhr:")) != -1) {
		switch (opt) {
        case 'r':
            //Random Dissemination Method
            o.random_method = optarg;
            break;
		case 'n':
			//Array size
			o.n_numbers = atol(optarg);
			break;
		case 'o':
			//Output to stdout?
			o.enable_output = true;
			break;
        case 'O':
            //Write the array to a binary file
            o.write_array = true;
            o.array_file = optarg;
            break;
        case 'V':
            //Validate the written array
            o.validate_array = true;
            break;
		case 'a':
			o.algorithm = optarg;
			break;
		case 'v':
			o.validate_sort = true;
			break;
		case 'm':
			o.machine_output = true;
			break;
		case 'l':
			o.n_levels = atoi(optarg);
			break;
		case 't':
			o.n_threads = atoi(optarg);
			break;
		case '?':
		default:
			if (enable_output) usage();
            o.parameters_invalid=true;
		}
	}

    // Check combinations
    if (o.validate_array && !o.write_array) {
        if (enable_output) {
            fprintf(stderr, "Validation of array requested without specifying array should be written.\n");
            fprintf(stderr, "Disabling array validation.");
        }
        o.validate_array=false;
        o.parameters_invalid=true;
    }
	return o;
}

/*
 * Validates the results of a sort. Returns true if the numbers are sorted,
 * false otherwise. Note this only validates that the numbers are
 * an increasing order; it does not validate that the set of the
 * numbers is the same. See also serial_validate_sort()
 */
bool
validate_sort(RANDOM_NUMBER array[], unsigned long n)
{
	for (unsigned int i = 0; i < n - 1; i++) {
		if (array[i] > array[i + 1]) {
			return false;
		}
	}
	return true;
}

/*
 * Outputs performance statistics and runtime options for the sort algorithm.
 */
void
print_stats(const opts o, const stats s)
{
	double	wall_time = timespec_diff(s.end_time, s.start_time);
	double	clocks_per_Mn = s.total_cpu_time / o.n_numbers * 10e6;
	double	seconds_per_Mn = wall_time / o.n_numbers * 10e6;

	printf("%s,%s,%d,%d,%lu,%f,%f,%f,%f\n", o.parallel_or_serial, o.algorithm, s.processes, o.n_threads, o.n_numbers, s.total_cpu_time, wall_time, clocks_per_Mn, seconds_per_Mn);
}

/* Outputs an array of floating point numbers to stdout */
void
print_array(RANDOM_NUMBER array[], unsigned long n)
{
	for (unsigned long i = 0; i < n; i++) {
		printf("%.15f\n", array[i]);
	}
}
