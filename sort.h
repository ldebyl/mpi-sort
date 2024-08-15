/********************************************************************
 * CITS5007 High Performance Computing
 * University of Western Australia
 * Assignment 1
 *
 * Lee de Byl (10306807@student.uwa.edu.au)
 * September 2021
 *
 * Main entry point for Assignment 1.
 *
 *******************************************************************/
#include "config.h"
#include "random.h"
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <time.h>

#define ENUMERATION_SORT        "ENUMERATION"
#define ENUMERATION_O_SORT      "ENUMERATION_OUTER"
#define ENUMERATION_I_SORT      "ENUMERATION_INNER"
#define QUICK_SORT              "QUICK"
#define MERGE_SORT              "MERGE"

// Random number generation options
// These options control how random numbers
// are generated and distributed across processes.
#define RND_SHARED_SEED          "SHARED_SEED"
#define RND_BCAST                "BCAST"

// Struct for command line arguments
typedef struct Options {
    int		n_threads;
    unsigned long    n_numbers;
    int		n_levels;
    char    *algorithm;
    bool	enable_output;
    bool	validate_sort;
    bool	machine_output;
    bool    write_array;
    bool    validate_array;
    char    *array_file;
    bool    parameters_invalid;
    char    *parallel_or_serial;
    char    *random_method;
}		opts;

//Struct for run time stats.
typedef struct Stats {
    int      processes;
    struct   timespec	start_time, end_time;
    clock_t	 start_clock, end_clock;
    bool     valid_sort;
    double   cpu_time;  // CPU Time for individual process
    double   total_cpu_time; // Total CPU time across all processes
} stats;

opts
parse_args(int argc, char **argv, bool enable_output);

bool
validate_sort(RANDOM_NUMBER array[], unsigned long n);

void
print_stats(const opts o, const stats s);

void
print_array(RANDOM_NUMBER array[], unsigned long n);
