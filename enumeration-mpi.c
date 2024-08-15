/********************************************************************
 * CITS5007 High Performance Computing
 * University of Western Australia
 * Assignment 2
 *
 * Lee de Byl (10306807@student.uwa.edu.au
 * October 2021
 *
 * Implements an enumeration sort using MPI.
 *******************************************************************/

#include "enumeration-mpi.h"
#include "debug.h"
#include "sort.h"

/*
 * Performs an enumeration sort on the given array. n specifies the number of
 * elements. If parallel_outer is true, the outer loop is run in parallel
 * when using openMP. If parallel_inner is true, the inner loop is run in
 * parallel.
 */

void
mpi_enumeration_sort(RANDOM_NUMBER * array, long n, bool parallel_outer, bool parallel_inner)
{
    int rank;
    int n_processes;
    long start, end, index;

    // Get the number of processes and rank for this communicator and process.
    MPI_Comm_size(MPI_COMM_WORLD, &n_processes);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    // Obtain the chunk of the array this process should work on.
    //mpi_random_chunk(array, n, n_processes, rank, &chunk, &end);

    start = n/n_processes * rank;
    end = start + n/n_processes + (rank == n_processes-1 ? n%n_processes : 0);

    PRINT_DEBUG("Rank %d Start: %ld End %ld <END>\n", rank,  start, end);

    // Allocate a buffer for each process to store the sorted
    // data in.
    RANDOM_NUMBER *outdata = calloc(n, sizeof(RANDOM_NUMBER));

    #pragma omp parallel for private(index) if (parallel_outer == true)
	for (long j = start; j < end; j++) {
		index = 0;

        #pragma omp parallel for reduction(+:index) if (parallel_inner == true)
		for (long i = 0; i < n; i++) {
			if (array[i] < array[j])
				index++;
			else if ((array[i] == array[j]) && (j < i))
				index++;
		}
		outdata[index] = array[j];
	}
    // Gather all the results from the various processes, and return them to
    // root.
    MPI_Reduce(outdata, array, n, MPI_RANDOM_NUMBER, MPI_SUM, 0, MPI_COMM_WORLD);
}
