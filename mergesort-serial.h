
/********************************************************************
 * CITS5007 High Performance Computing
 * University of Western Australia
 * Assignment 2
 *
 * Lee de Byl (10306807@student.uwa.edu.au
 * October 2021
 *******************************************************************/

#include "random.h"
#include <stdlib.h>
#include <string.h>

#define MAX(a,b) (a > b) ? a : b

void merge_sort(RANDOM_NUMBER array[], unsigned long n);
