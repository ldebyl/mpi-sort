/********************************************************************
 * CITS5007 High Performance Computing
 * University of Western Australia
 * Assignment 2
 *
 * Lee de Byl (10306807@student.uwa.edu.au
 * October 2021
 *
 *******************************************************************/
#include "quicksort-mpi.h"
#include "quicksort-serial.h"
#include "debug.h"

#define ROOT 0
#define MSG_RIGHT_TO_LEFT 1
#define MSG_LEFT_TO_RIGHT 2

long search(RANDOM_NUMBER array[], long n, RANDOM_NUMBER value) {
    long i=0;
    for (i=0; i<n; i++) {
        if (array[i] > value) return i-1;
    }
    return i-1;
}

void copy_elements (
        RANDOM_NUMBER src[],
        RANDOM_NUMBER dest[],
        unsigned long src_index,
        unsigned long dest_index,
        unsigned long n) {
    for (unsigned long i = 0; i < n; i++) {
        dest[i+dest_index] = src[i+src_index];
    }
}

/* mpi_quicksort2
 * The internal, recursive function of the MPI Quicksort, based on hyper
 * quicksort.
 *
 * Parameters:
 *    *array: The array to be sorted.
 *    *n:     The number of elements that should be handled by this process
 *    *last_n The total number of elements handled by this branch. This value
 *    *       is incremented by each recursion.
 *    *commuinator: The MPI communicator that should be used. These are
 *            progressively split.
 */
static void mpi_quicksort_recursive(RANDOM_NUMBER *array, long n, int *last_n, MPI_Comm communicator) {
    int rank, nprocesses, number_received;
    long n_keep, pivot_index;
    RANDOM_NUMBER pivot_value;
    RANDOM_NUMBER mean_local[2] = {0.0, n};
    RANDOM_NUMBER mean_global[2] = {0.0, 0.0};
    RANDOM_NUMBER *chunk_received, *chunk_keep;
    MPI_Status status;
    MPI_Comm split_communicator;

    // Get details of the current communicator
    MPI_Comm_size(communicator, &nprocesses);
    MPI_Comm_rank(communicator, &rank);

    PRINT_DEBUG("Rank %d has %ld elements to deal with.\n", rank, n);

    if (nprocesses==1) {
        *last_n = n;
        return;
    }

    /* Find the average split point across all the processes */
    for (long i=0; i<n; i++) {
        mean_local[0] = mean_local[0] + array[i];
    }
   PRINT_DEBUG("******** Rank %d: Local mean: %f", rank, mean_local[0]);
   MPI_Reduce(&mean_local, &mean_global, 2, MPI_RANDOM_NUMBER, MPI_SUM, ROOT, communicator);

   // If this is the root in the communicator
   if (rank==ROOT) {
       pivot_value = mean_global[0] / mean_global[1];
       PRINT_DEBUG("Root: Global Pivot Value: %f", pivot_value);
   }
   MPI_Bcast(&pivot_value, 1, MPI_RANDOM_NUMBER, ROOT, communicator);

   PRINT_DEBUG("Rank: %d - Searching for number: %f", rank, pivot_value);
   pivot_index = search(array, n, pivot_value);
   PRINT_DEBUG("Rank: %d: Pivot index of %ld", rank, pivot_index);

   /* Exchange halves */
   if (rank < nprocesses/2) {
       // Send elements higher than pivot to our neighbour
       MPI_Send(array + (pivot_index+1), (n-1)-pivot_index, MPI_RANDOM_NUMBER, rank + nprocesses/2, MSG_LEFT_TO_RIGHT, communicator);
       // Receive elements lower than pivot from our neighbour
       MPI_Probe(rank + nprocesses/2, MSG_RIGHT_TO_LEFT, communicator, &status);
       MPI_Get_count(&status, MPI_RANDOM_NUMBER, &number_received);
       PRINT_DEBUG("Rank %d Number received: %d", rank, number_received);
       // Allocate a buffer to receive the juicy data
       chunk_received = malloc(number_received * sizeof(RANDOM_NUMBER));
       MPI_Recv(chunk_received, number_received, MPI_RANDOM_NUMBER, rank+nprocesses/2,
               MSG_RIGHT_TO_LEFT, communicator, &status);
   } else {
       MPI_Probe(rank - nprocesses/2, MSG_LEFT_TO_RIGHT, communicator, &status);
       MPI_Get_count(&status, MPI_RANDOM_NUMBER, &number_received);
       PRINT_DEBUG("Rank %d Number received: %d", rank, number_received);
       chunk_received = malloc(number_received * sizeof(RANDOM_NUMBER));
       MPI_Recv(chunk_received, number_received, MPI_RANDOM_NUMBER, rank-nprocesses/2, MSG_LEFT_TO_RIGHT, communicator,
               &status);
       MPI_Send(array, pivot_index + 1, MPI_RANDOM_NUMBER, rank - nprocesses/2, MSG_RIGHT_TO_LEFT, communicator);
   }
   PRINT_DEBUG("Rank: %d: Completed exchange.", rank);

   // Copy halves
   if (rank < nprocesses/2) {
        n_keep = pivot_index + 1;
        PRINT_DEBUG("Rank %d: n_keep: %ld, pivot index: %ld", rank, n_keep, pivot_index);
        chunk_keep = malloc(n_keep * sizeof(RANDOM_NUMBER));
        copy_elements(array, chunk_keep, 0, 0, n_keep);
   } else {
        n_keep = (n - pivot_index) - 1;
        PRINT_DEBUG("Rank %d: n_keep: %ld, pivot index: %ld", rank, n_keep, pivot_index);
        chunk_keep = malloc(n_keep * sizeof(RANDOM_NUMBER));
        copy_elements(array, chunk_keep, pivot_index + 1, 0, n_keep);
    }
    PRINT_DEBUG("Rank %d: Number Received: %d Number keep: %ld: Elements per Proc: %ld", rank, number_received, n_keep, n);
    merge(chunk_received, number_received, chunk_keep, n_keep, array);
    int side = rank/(nprocesses/2);
    PRINT_DEBUG("Rank %d forking to side %d", rank, side);
    MPI_Comm_split(communicator, side, rank, &split_communicator);
    mpi_quicksort_recursive(array, n_keep + number_received, last_n, split_communicator);
}

void mpi_quicksort(RANDOM_NUMBER * array, unsigned long n) {
    int rank, n_processes;
    RANDOM_NUMBER *chunk;
    long n_chunk;
    int last_n;
    int *displacements, *counts;

    MPI_Comm_size(MPI_COMM_WORLD, &n_processes);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    mpi_random_chunk(array, n, n_processes, rank, &chunk, &n_chunk);

    // Perform a serial sort of this chunk.
    quick_sort(chunk, n_chunk);
    // Perform the parallel quicksort with the other
    // chunks across the other processes.
    mpi_quicksort_recursive(chunk, n_chunk, &last_n, MPI_COMM_WORLD);

    // Allocate memory for count and displacement vectors for gather operations.
    counts = malloc(n_processes * sizeof(int));
    displacements = malloc(n_processes * sizeof(int));

    // last_n is the final number of values in this chunk
    MPI_Gather(&last_n, 1, MPI_INT, counts, 1, MPI_INT, ROOT, MPI_COMM_WORLD);

    // Each process will have a different number of results that need
    // to be assimilated.
    // Create a displacement vector

    MPI_Barrier(MPI_COMM_WORLD);
    if (rank==ROOT) {
        // Create the displacement vector by taking the
        // cumulative sum of received counts
        for (int i=0; i<n_processes; i++) {
            if (i==0) displacements[0] = 0;
            else displacements[i] = displacements[i-1] + counts[i-1];
        }
    }

    // New array to receive into
    RANDOM_NUMBER *collected = malloc(n * sizeof(RANDOM_NUMBER));
    MPI_Gatherv(chunk, last_n, MPI_RANDOM_NUMBER, collected, counts, displacements, MPI_RANDOM_NUMBER, ROOT, MPI_COMM_WORLD);
    if (rank==ROOT) {
        for (int i=0; i<n; i++) {
            array[i]=collected[i];
        }
    }
}
