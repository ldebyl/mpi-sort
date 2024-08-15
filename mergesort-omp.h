
/********************************************************************
 * CITS5007 High Performance Computing
 * University of Western Australia
 * Assignment 2
 *
 * Lee de Byl (10306807@student.uwa.edu.au
 * October 2021
 *
 *******************************************************************/

#include <omp.h>
#include "random.h"
#include <stdlib.h>
#include <string.h>

#define MAX(a,b) (a > b) ? a : b

void		swap      (int *p, int *q);
void		pmergesort(RANDOM_NUMBER a[], RANDOM_NUMBER b[], int const low, int const high, int const offset);
int		binary_search(RANDOM_NUMBER key, RANDOM_NUMBER array[], const int p, const int r);
void
pmerge(RANDOM_NUMBER t[], int p1, int r1, int p2, int r2, RANDOM_NUMBER
       a[], int p3);
void omp_mergesort(RANDOM_NUMBER array[], int n);
