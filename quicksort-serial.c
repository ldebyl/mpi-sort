/********************************************************************
 * CITS5007 High Performance Computing
 * University of Western Australia
 * Assignment 2
 *
 * Lee de Byl (10306807@student.uwa.edu.au
 * October 2021
 *
 * Serial implementation of the Quicksort algorithm
 *******************************************************************/

#include "quicksort-serial.h"

/* Partition an array between elements a and b */
long partition(RANDOM_NUMBER * array, long a, long b)
{
	RANDOM_NUMBER	pivot_value = array[b];
	RANDOM_NUMBER	temp;
	long		    pivot_index = a - 1;

	for (long i = a; i <= b; i++) {
		if (array[i] <= pivot_value) {
			pivot_index++;

            /* Swap array[pivot_index] with array[i] */
			temp = array[pivot_index];
			array[pivot_index] = array[i];
			array[i] = temp;
		}
	}
	return pivot_index;
}

/* Recursively perform a Quicksort on an array
 * between two elements *in-place*
 */
static void qs(RANDOM_NUMBER * array, long a, long b)
{
	long		pivot_index;

	// Only recurse if a and b are reasonable indexes,
	// and if a is less than b.
    if (a >= 0 && b >= 0 && a < b) {
        pivot_index = partition(array, a, b);
        qs(array, a, pivot_index - 1);
        qs(array, pivot_index + 1, b);
    }
}

/* Perform a Quicksort on an array *in-place* with the
 * specified number of elements */
void quick_sort(RANDOM_NUMBER * array, unsigned long n)
{
	qs(array, 0, n - 1);
}
