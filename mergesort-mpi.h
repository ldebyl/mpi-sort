
/********************************************************************
 * CITS5007 High Performance Computing
 * University of Western Australia
 * Assignment 1
 *
 * Lee de Byl (10306807@student.uwa.edu.au
 * September 2021
 *
 * Specifies default and configurable parameters for the sort
 * experiment.
 *******************************************************************/

#include "mergesort-omp.h"
#include "random-mpi.h"
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "mpi.h"

void mpi_mergesort(RANDOM_NUMBER *array, long n);
void merge(RANDOM_NUMBER *a, unsigned long n_a, RANDOM_NUMBER *b, unsigned long n_b, RANDOM_NUMBER *c);
