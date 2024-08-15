/****************************************************************************
 * CITS5007 - Assignment 2
 * Lee de Byl (10306807)
 * 10306807@student.uwa.edu.au
 *
 * October 2021
 *
 * Generates, saves and loads random numbers in an MPI Context.
 ***************************************************************************/
#include "sort-serial.h"

int main(int argc, char **argv)
{
	stats		    stats;
	opts		    o;
	RANDOM_NUMBER  *array;

    // Parse command line arguments
    // If invalid parameters, exit (this structure is for
    // compatiblity with the MPI version, to allow sharing
    // of the command line argument parser).
	o = parse_args(argc, argv, true);
    o.parallel_or_serial = "SERIAL";
    o.n_threads = 0;
    stats.processes = 1;

    if (o.parameters_invalid) exit(1);

    fprintf(stderr, "Generating and Sorting %lu random numbers.\n", o.n_numbers);

    // Generate the array of random numbers.
	array = random_array(o.n_numbers);

    // Write the array to a binary file, if requested.
    if (o.write_array) {
        fprintf(stderr, "Writing the array to %s.\n", o.array_file);
        write_array(o.array_file, array, o.n_numbers);
    }

    // If the array was written above, and array validation has been requested,
    // perform validation and write to stderr if validation fails.
    if (o.write_array && o.validate_array) {
        fprintf(stderr, "Validating the written array.\n");
        int error_code = validate_array(o.array_file, array, o.n_numbers);
        switch (error_code) {
        case BAD_COUNT:
            fprintf(stderr, "Stated count in binary array file does not match count of written array.\n");
            break;
        case BAD_COMPARE:
            fprintf(stderr, "Element-wise comparison of array in memory and array on disk does not match.\n");
            break;
        }
    }

	// Start timing
	stats.start_clock = clock();
	clock_gettime(CLOCK_MONOTONIC, &stats.start_time);

    // Perform the sort
    // Note that the three versions of enumeration sort
    // are identical in the serial version. They are
    // listed here for compatibility with the MPI version only.
    fprintf(stderr, "Commencing sort ...\n");

    if (strcmp(o.algorithm, MERGE_SORT) == 0) {
		merge_sort(array, o.n_numbers);
	} else if (strcmp(o.algorithm, QUICK_SORT) == 0) {
		quick_sort(array, o.n_numbers);
	} else if (strcmp(o.algorithm, ENUMERATION_SORT) == 0) {
		array = enumeration_sort(array, o.n_numbers);
	} else if (strcmp(o.algorithm, ENUMERATION_O_SORT) == 0) {
		array = enumeration_sort(array, o.n_numbers);
	} else if (strcmp(o.algorithm, ENUMERATION_I_SORT) == 0) {
		array = enumeration_sort(array, o.n_numbers);
	} else {
		fprintf(stderr, "Invalid algorithm specified: %s\n", o.algorithm);
		exit(-1);
	}

    // Stop timing and calculate CPU Usages
	stats.end_clock = clock();
	clock_gettime(CLOCK_MONOTONIC, &stats.end_time);
    stats.cpu_time = (double)(stats.end_clock - stats.start_clock) / CLOCKS_PER_SEC;
    stats.total_cpu_time = stats.cpu_time;

    // Output the results
	print_stats(o, stats);

    // Output the sorted array, if requested
	if (o.enable_output) {
		print_array(array, o.n_numbers);
	}

    // Validate the sort by checking the numbers are in
    // order. This differs to the MPI version, which checks
    // against the serial version of the sort.
	bool valid = true;
	if (o.validate_sort) {
		valid = validate_sort(array, o.n_numbers);
		if (!valid) {
			fprintf(stderr, "Sort validation failed!\n");
		} else {
			fprintf(stderr, "Sort validation passed.\n");
		}
	}

    // Return the status of the validation.
	return (valid);
}
