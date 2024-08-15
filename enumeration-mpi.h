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

#include "random-mpi.h"
#include "random.h"
#include "config.h"
#include <stdbool.h>
#include <omp.h>
#include "mpi.h"

void mpi_enumeration_sort(RANDOM_NUMBER *, long n, bool parallel_outer, bool parallel_inner);
