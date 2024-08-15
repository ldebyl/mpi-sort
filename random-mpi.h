#include "random.h"
#include "mpi.h"
#include <omp.h>
#include <time.h>

RANDOM_NUMBER * mpi_common_random_array(unsigned long n);
RANDOM_NUMBER * mpi_bcast_random_array(unsigned long n);
int mpi_write_array(char *filename, RANDOM_NUMBER *array, unsigned long n);
void mpi_random_chunk(RANDOM_NUMBER *array, long n, int n_processes,
        int rank, RANDOM_NUMBER **chunk, long *chunk_n);
