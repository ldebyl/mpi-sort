/********************************************************************
 * CITS5007 High Performance Computing
 * University of Western Australia
 * Assignment 2
 *
 * Lee de Byl (10306807@student.uwa.edu.au
 * October 2021
 *
 * Main entry point for Assignment 2.
 * This is the MPI version - unlike assignment 1, a seperate
 * serial and a seperate parallel version exist.
 *******************************************************************/

#include "debug.h"
#include "sort-mpi.h"
#

/* Function to validate the parallel sort against its
 * serial implementation.
 */
static bool validate_sort_serial(
        RANDOM_NUMBER *array_sorted,
        RANDOM_NUMBER *array_unsorted,
        long n,
        char* algo
);

/*
 * Returns true if the provided integer is a power of two,
 */
static int check_power_of_two(int i) {
    return ((i != 0) && ((i &(i - 1)) == 0));
}

/* Main entry point for the program */
int main(int argc, char **argv) {
	int             rank, n_processes;
    bool            valid = true;
    stats		    stats;
	opts		    o;
	RANDOM_NUMBER   *array;
    RANDOM_NUMBER   *unsorted_array;
    int             thread_level_provided;

    // Perform MPI Initialisation
    MPI_Init_thread(&argc, &argv, MPI_THREAD_FUNNELED, &thread_level_provided);
    MPI_Comm_size(MPI_COMM_WORLD, &n_processes);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    // Check that the MPI Thread Level provided is what was requested and
    // warn if this was not the case.
    if (thread_level_provided != MPI_THREAD_FUNNELED && rank==0) {
        fprintf(stderr, "Requested MPI_THREAD_FUNNELED, but it was not provided.\n");
    }

    // Parse command line arguments
    // Only enable output (usage etc) on rank 0.
	o = parse_args(argc, argv, rank==0);

    // Set whether this is the serial or parallel version.
    o.parallel_or_serial = "PARALLEL";
    stats.processes = n_processes;

    // Check if a valid number of processes are available
    // for the specified algorithm.
    if (strcmp(o.algorithm,QUICK_SORT)==0 || strcmp(o.algorithm,MERGE_SORT) == 0) {
        if (!check_power_of_two(n_processes)) {
            o.parameters_invalid = true;
            if (rank == 0) {
                fprintf(stderr, "Number of processes must be power of two when using quick or merge sort.\n");
            }
        }
    }

    // If the specified parameters are valid, continue with
    // execution of main program logic.
    if (!o.parameters_invalid) {

        // Initialise OpenMP
        #ifdef _OPENMP

            // Set the default number of threads for a parallel construct
            omp_set_num_threads(o.n_threads);

            //Enable nested threads, which are required for parallel recursion.
            omp_set_max_active_levels(o.n_levels);
            if (rank==0) fprintf(stderr, "Number of threads: %d\n", omp_get_max_threads());

        #else
            // No OpenMP - Running in serial, set threads to zero for correct
            // logging output.
            o.n_threads = 0;
        #endif

        if (rank==0)
            fprintf(stderr, "Generating and Sorting %lu random numbers.\n", o.n_numbers);

        /*************************************************
         * Random array generation
         * **********************************************/

        if (strcmp(o.random_method, RND_SHARED_SEED)==0) {
            // Generate a random array using both MPI and OpenMP
            // Note that Process Rank 0 distributes the seed
            // to all other processes, which is based on the time
            // on the root process.
            if (rank == 0)
                fprintf(stderr, "Using Shared Seed RN Array Generation\n");
            array = mpi_common_random_array(o.n_numbers);
        } else {
            if (rank == 0)
                fprintf(stderr, "Using RN Array Broadcast\n");
            array = mpi_bcast_random_array(o.n_numbers);
        }

        // Make a copy of the unsorted array to validate
        // the sort later, as these algorithms all sort
        // in-place.
        if (rank == 0) {
            unsorted_array = malloc(o.n_numbers * sizeof(RANDOM_NUMBER));
            memcpy(unsorted_array, array, o.n_numbers * sizeof(RANDOM_NUMBER));
        }

        // If requested, write the array to the specified file.
        // This will be performed by all processes in parallel,
        if (o.write_array) {
            mpi_write_array(o.array_file, array, o.n_numbers);
        }

        // If the array was written above, and array validation has been requested,
        // perform validation and write to stderr if validation fails.
        if (o.write_array && o.validate_array && rank==0) {
            fprintf(stderr, "Validating the written array.\n");
            int error_code = validate_array(o.array_file, array, o.n_numbers);
            switch (error_code) {
            case BAD_COUNT:
                fprintf(stderr, "Stated count in binary array file does not match count of written array.\n");
                break;
            case BAD_COMPARE:
                fprintf(stderr, "Element-wise comparison of array in memory and array on disk does not match.\n");
                break;
            default:
                fprintf(stderr, "Written Array Validation Passed.\n");
            }
        }

        // Start timing of the sort operation
        stats.start_clock = clock();
        clock_gettime(CLOCK_MONOTONIC, &stats.start_time);

        // Execute the requested sort
        if (strcmp(o.algorithm, MERGE_SORT) == 0) {
            mpi_mergesort(array, o.n_numbers);
        } else if (strcmp(o.algorithm, QUICK_SORT) == 0) {
            mpi_quicksort(array, o.n_numbers);
        } else if (strcmp(o.algorithm, ENUMERATION_SORT) == 0) {
            mpi_enumeration_sort(array, o.n_numbers, false, false);
        } else if (strcmp(o.algorithm, ENUMERATION_O_SORT) == 0) {
            mpi_enumeration_sort(array, o.n_numbers, true, false);
        } else if (strcmp(o.algorithm, ENUMERATION_I_SORT) == 0) {
            mpi_enumeration_sort(array, o.n_numbers, false, true);
        } else {
            fprintf(stderr, "Invalid algorithm specified: %s\n", o.algorithm);
        }

        // Stop timing and calculate the amount of CPU time consumed by each
        // process. Collect usage information from each process to compute
        // the total number of CPU cycles consumed.
        stats.end_clock = clock();
        clock_gettime(CLOCK_MONOTONIC, &stats.end_time);
        stats.cpu_time = (double)(stats.end_clock - stats.start_clock) / CLOCKS_PER_SEC;
        MPI_Reduce(&stats.cpu_time, &stats.total_cpu_time, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);

        // Output collected statistics from the root process.
        // Also validate the sort results (in serial) and output
        // the sorted array to stdout (if requested).
        if (rank==0) {
            print_stats(o, stats);

            // Output the results, if requested.
            if (o.enable_output) {
               print_array(array, o.n_numbers);
            }

            // Valiate the results, if requested.
            // Unlike the serial version, this validates
            // results against the serial version of
            if (o.validate_sort) {
                fprintf(stderr, "Validating parallel against serial implementations.\n");
                valid = validate_sort_serial(array, unsorted_array, o.n_numbers, o.algorithm);
                if (!valid) {
                    fprintf(stderr, "Validation of Parallel vs Serial Implementation failed!\n");
                } else {
                    fprintf(stderr, "Validation of Parallel vs Serial Implementation Passed.\n");
                }
            }
        }
    } // end of invalid parameter guard

    // Clean-up, ready for the application's demise.
    MPI_Finalize();

    // Only return an error code from the main process
    return(0);
}

/* Function to validate a parallel sort against its serial counterpart */
static bool validate_sort_serial(
        RANDOM_NUMBER *array_sorted,
        RANDOM_NUMBER *array_unsorted,
        long n,
        char* algo) {

        // Execute the requested sort
        if (strcmp(algo, MERGE_SORT) == 0) {
            merge_sort(array_unsorted, n);
        } else if (strcmp(algo, QUICK_SORT) == 0) {
            quick_sort(array_unsorted, n);
        } else if (strcmp(algo, ENUMERATION_SORT) == 0) {
            array_unsorted = enumeration_sort(array_unsorted, n);
        } else if (strcmp(algo, ENUMERATION_O_SORT) == 0) {
            array_unsorted = enumeration_sort(array_unsorted, n);
        } else if (strcmp(algo, ENUMERATION_I_SORT) == 0) {
            array_unsorted = enumeration_sort(array_unsorted, n);
        } else {
            fprintf(stderr, "Invalid algorithm specified: %s\n", algo);
        }

    /* Perform an element-by-element comparison (rather than use strcmp)
     * so any mistmatches can be reported informatively */
    for (long i=0; i<n; i++) {
        if (array_sorted[i] != array_unsorted[i]) {
            fprintf(stderr, "Mismatch between parallel and serial sorted arrays.\n");
            fprintf(stderr, "Mismatch occured at element %ld\n", i);
            return false;
        }
    }
    return true;
}
