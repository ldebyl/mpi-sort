/********************************************************************
 * CITS5007 High Performance Computing
 * University of Western Australia
 * Assignment 2
 *
 * Lee de Byl (10306807@student.uwa.edu.au
 * October 2021
 *
 * MPI Version of the Mergesort algorithm
 *******************************************************************/

#include "mergesort-mpi.h"
#include "sort.h"
#include "debug.h"

void merge(RANDOM_NUMBER *a, unsigned long n_a, RANDOM_NUMBER *b, unsigned long n_b, RANDOM_NUMBER *c) {
    long i=0, j=0, index=0;
    // Perform the merge on the equal lengths of the vectors
    while(i < n_a && j < n_b)
        c[index++] = a[i]<b[j] ? a[i++] : b[j++];

    // Deal with the leftovers
    if (i >= n_a) {
        while (j < n_b) {
            c[index++] = b[j++];
        }
    }
    if (j >= n_b) {
        while (i < n_a) {
            c[index++] = a[i++];
        }
    }
}

void mpi_mergesort(RANDOM_NUMBER *array, long n) {
    int rank;           // rank of this process
    int n_processes;    // total number of processes
    int max_height;     // total height of the process tree
    int parent_rank;    // rank of the parent that these results will be sent to.
    int right_child_rank; // the rank of the right child process
    int height=0;      // The current height in the tree of this process.
    int received_count; // Number of elements to be received from child node.
    MPI_Status status;  // General MPI Status buffer
    RANDOM_NUMBER *half1;       // The portion of the array being worked on by this process.
    RANDOM_NUMBER *half2;       // Portion of the array that will be received from the right child
    RANDOM_NUMBER *mergeResult; // Where the two halves will be merged into.
    long n_chunk;        // The region of the array this mergesort is operating on

    // Get basic MPI Variables
    MPI_Comm_size(MPI_COMM_WORLD, &n_processes);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    // Find the maximum number of levels that can be merged
    max_height = log2(n_processes);

    // Get the portion of the total array this process should
    // start with.
    mpi_random_chunk(array, n, n_processes, rank, &half1, &n_chunk);

    // Sort this half in serial
    // The line below uses the stdlib implementation of mergesort
    //mergesort(half1, n_chunk, sizeof(RANDOM_NUMBER), comp); // sort local array

    // Use our OMP implementation of Mergesort
    omp_mergesort(half1, n_chunk);

    while (height < max_height) { // not yet at top
        parent_rank = (rank & (~(1 << height)));

        PRINT_DEBUG("Parent of rank %d is: %d", rank, parent_rank);

        if (rank == parent_rank) {
            // This is a "left" child. This will receive
            // the sorted sub-array from the "right" peer,
            // and perform the merge.
		    right_child_rank = (rank | (1 << height));

  		    // Prepare to receive the results from the right-hand child
            MPI_Probe(right_child_rank, 0, MPI_COMM_WORLD, &status);

            // Get the count of numbers to be received from the status
            // so that memory can be allocated.
            MPI_Get_count(&status, MPI_RANDOM_NUMBER, &received_count);
            PRINT_DEBUG("Rank %d Preparing to receive %d items from rank %d", rank, received_count, right_child_rank);
            // Allocate memory and receive the sorted data from the right-hand
            // peer.

            half2 = malloc(received_count * sizeof(RANDOM_NUMBER));
  		    MPI_Recv(half2, received_count, MPI_RANDOM_NUMBER, right_child_rank, 0,
				MPI_COMM_WORLD, &status);

  		    // Allocate memory for the result of the merge
  		    mergeResult = malloc ((n_chunk + received_count) * sizeof(RANDOM_NUMBER));
  		    // merge half1 and half2 into mergeResult
  		    merge(half1, n_chunk, half2, received_count, mergeResult);
  		    // reassign half1 to merge result
            half1 = mergeResult;
            // Calculate the size of the newly merged chunk
			n_chunk = n_chunk + received_count;
            height++;
			free(half2);
        } else {
            // Send the result to the parent process.
            MPI_Send(half1, n_chunk, MPI_RANDOM_NUMBER, parent_rank, 0, MPI_COMM_WORLD);
            if(height != 0) free(half1);
            height = max_height;
        }
    }

    // Sort is complete, rank0 (the root process) contains the
    // sorted array. Copy it back.
    if (rank == 0) {
        PRINT_DEBUG("Final size: %ld\n", n_chunk);
        for (long i=0; i<n_chunk; i++) {
            array[i] = half1[i];
        }
    }
}
