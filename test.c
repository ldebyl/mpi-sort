#include<stdio.h>
#include "mpi.h"
#define N 16
int main(int argc, char **argv){
    int rank, num_of_process;
    printf("here");
    MPI_File fhw;
    MPI_Status status;
    MPI_Init(&argc, &argv);
    printf("blah %s\n", argv[1]);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &num_of_process);
    int buf[N];
    for (int i=0;i<N;i++) buf[i] = i;
    int ave_num_of_ints = N / num_of_process;
    int offset = rank * ave_num_of_ints;
    MPI_File_open(MPI_COMM_WORLD, "dfile",
    MPI_MODE_CREATE|MPI_MODE_WRONLY, MPI_INFO_NULL, &fhw);
    printf("\nRank: %d, Offset: %d\n", rank, offset);
    MPI_File_write_at(fhw, offset * sizeof(int), buf+offset, ave_num_of_ints, MPI_INT, &status);
    MPI_File_close(&fhw);
    MPI_Finalize();
    return 0;
}
