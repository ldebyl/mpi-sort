/********************************************************************
 * CITS5007 High Performance Computing
 * University of Western Australia
 * Assignment 2
 *
 * Lee de Byl (10306807@student.uwa.edu.au)
 * October 2021
 *
 *
 *******************************************************************/

#include "sort.h"
#include "enumeration-mpi.h"
#include "quicksort-mpi.h"
#include "mergesort-mpi.h"
#include "mpi.h"
#include "random-mpi.h"
#include "random-serial.h"
#include <omp.h>
#include <string.h>

// Serial sort algorithms for validating the
// parallel algorithms
#include "quicksort-serial.h"
#include "enumeration-serial.h"
#include "mergesort-serial.h"
