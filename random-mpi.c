/****************************************************************************
 * CITS5007 - Assignment 2
 * Lee de Byl (10306807)
 * 10306807@student.uwa.edu.au
 *
 * October 2021
 *
 * Generates, saves and loads random numbers in an MPI Context.
 ***************************************************************************/

#include "random-mpi.h"
#include "debug.h"

/* Generates an array of random numbers using MPI.
 * Rather than generating chunks of random numbers
 * and gathering the results back to the root process,
 * only for them to be scattered again, we make the assumption
 * that generating the random numbers within each process
 * is more efficient. Therefore, we give each process the
 * same seed and generate the set from the same seed in
 * each process.
 *
 * OpenMP will be used in each process where applicable.
 */
RANDOM_NUMBER * mpi_common_random_array(unsigned long n)
{
    int rank;
    long int seed;
    RANDOM_NUMBER *array = malloc(n * sizeof(RANDOM_NUMBER));

    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    if (rank == 0)
        seed = (long int)time(NULL);

    // Sync the seeds of all the processes in the communicator.
    MPI_Bcast(&seed, 1, MPI_LONG_INT, 0, MPI_COMM_WORLD);
    srand48(seed);

    // Generate the random numbers. Note that it is critical
    // that this be run in serial on each process.
    for (unsigned long i = 0; i < n; i++) {
		array[i] = drand48();
	}

    // Wait for each process to finish.
    MPI_Barrier(MPI_COMM_WORLD);
	return (array);
}

/* Generates an array of random numbers using OMP
 * on the root process, and broadcasts it to all
 * processes. Each process will have its own complete
 * copy of the array.
 */
RANDOM_NUMBER * mpi_bcast_random_array(unsigned long n)
{
    int rank;
    RANDOM_NUMBER *array = malloc(n * sizeof(RANDOM_NUMBER));
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    if (rank == 0) {
        // Generate the master array on the root node.
        long int seed = (long int)time(NULL);
        srand48(seed);

        #pragma omp parallel for shared(array)
        for (unsigned long i = 0; i < n; i++) {
            array[i] = drand48();
        }
    }
    // Broadcast the array of random numbers to all processes
    // in the communicator.
    MPI_Bcast(array, n, MPI_RANDOM_NUMBER, 0, MPI_COMM_WORLD);

	return (array);
}

/* Gets a chunk from an array of random numbers for a process to work on.
 * Note the data is not copied; only a pointer and count are returned.
 *
 * array: The array to extract a chunk from
 * n: The number of elements in the array
 * n_processes: The number of processes to split between
 * rank: the rank of the process the chunk is for
 * chunk: the pointer that will be returned for the new chunk
 * chunk_n: the number of items in the new chunk (out)
 */
void mpi_random_chunk(RANDOM_NUMBER *array, long n, int n_processes,
        int rank, RANDOM_NUMBER **chunk, long *chunk_n) {
    long start = n/n_processes * rank;
    long end = start + n/n_processes + (rank == n_processes-1 ? n%n_processes : 0) - 1;
    *chunk_n = end-start+1;
    *chunk = &array[start];
}

/* Calculate the number of random numbers this
 * rank is reponsible for, based on the total
 * number of random numbers required. */
unsigned long mpi_rank_n(unsigned long n) {
    int rank, processes;
    unsigned long rank_n;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &processes);

    rank_n = (unsigned long)n / processes;
    if (rank+1 == processes) rank_n += n % processes;
    return (rank_n);
}

/* Calculate the offset in the output file for this
 * rank to write to based on the total n */
unsigned long mpi_file_offset(unsigned long n) {
    int rank, processes;
    unsigned long offset;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &processes);

    offset = sizeof(unsigned long) + rank * (n / processes) * sizeof(RANDOM_NUMBER);
    return offset;
}

/* Writes an array using MPI
 * Each process will write a different chunk to the file, depending
 * on the total number of processes and the rank of the process.
 * The file will contain a header word denoting the total number
 * of elements in the file.
 */
int mpi_write_array(char *filename, RANDOM_NUMBER *array, unsigned long n) {

    int                 rank_n = mpi_rank_n(n);
    unsigned long       offset = mpi_file_offset(n);
    int                 rank, processes;
    unsigned long       start, filesize = 0;
    MPI_File            file;
    MPI_Status          status;

    // Calculate the total file size in bytes, to truncate the file if
    // necessary.
    filesize = sizeof(unsigned long) + sizeof(RANDOM_NUMBER) * n;

    MPI_Comm_size(MPI_COMM_WORLD, &processes);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_File_open(MPI_COMM_WORLD,
        filename,
        MPI_MODE_CREATE|MPI_MODE_WRONLY,
        MPI_INFO_NULL,
        &file);

    // MPI_File_set_size is collective; all processes must run the same
    MPI_File_set_size(file, filesize);

    // Calculate the point of the array this process should write from.
    start = n/processes * rank;

    if (rank == 0) {
        // Write the header, which only needs to be done
        // once.
        MPI_File_write_at(file, 0, &n, 1, MPI_INT, &status);
    }

    PRINT_DEBUG("Rank %d is writing at offset %ld: n_elements: %d starting at %ld\n", rank, offset, rank_n, start);
    MPI_File_write_at(file, offset, array + start, rank_n, MPI_RANDOM_NUMBER, &status);
    MPI_File_close(&file);

    // Wait until all processes have finished writing, as verification
    // may be the next step. If verification commences before all processes
    // have finished writing, verification may fail.
    MPI_Barrier(MPI_COMM_WORLD);
    return (0);
}

/* Reads a portion of an array from disk using MPI File Operations.
 * Based on the total number of processes, and the rank of the process
 * performing the read, the segment of the array to be processed
 * by the calling process is read from disk. The number of array
 * elements relevant to the calling process is returned.
 *
 * Arguments:
 *   *filename: filename to read the array from.
 *   **array: Buffer to read into.
 */
unsigned long mpi_read_array(char *filename, RANDOM_NUMBER **array) {
    MPI_File file;
    MPI_Status status;
    unsigned long n, n_rank, offset;

    // Open the file using MPI Routines
    MPI_File_open(MPI_COMM_WORLD,
        filename,
        MPI_MODE_CREATE|MPI_MODE_WRONLY,
        MPI_INFO_NULL,
        &file);

    // Get the number of elements from the header
    MPI_File_read_at(file, 0, &n, 1, MPI_LONG, &status);

    // Based on the number of elements, calculate the number
    // of elements this rank should read and the offset
    // to commence at.
    n_rank = mpi_rank_n(n);
    offset = mpi_file_offset(n);

    // Perform the read.
    MPI_File_read_at(file, offset, array, n_rank, MPI_RANDOM_NUMBER, &status);
    MPI_File_close(&file);
    return (n_rank);
}
