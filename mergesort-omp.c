/********************************************************************
 * CITS5007 High Performance Computing
 * University of Western Australia
 * Assignment 2
 *
 * Lee de Byl (10306807@student.uwa.edu.au
 * October 2021
 *
 * OpenMP and Serial version of the Mergesort algorithm.
 * If compiled without OpenMP support, this essentially
 * becomes a serial version.
 *******************************************************************/

#include "mergesort-omp.h"
#include "debug.h"

/*
 * Swaps two integers in place. *a: pointer to integer 1 *b: pointer to
 * integer 2
 */
void
swap(int *a, int *b)
{
	int	tmp;
	tmp = *a;
	*a = *b;
	*b = tmp;
}

/*
 * mergesort Performs a mergesort of a[] into b[]. If cimpiled with openMP,
 * the sort will be performed in parallel.
 *
 * a[]: The array of integers to be sorted. b[]: The array to sort into. Must be
 * allocated prior to calling. low: The lower bound of a[] upon which sorting
 * shall be performed. high: The upper bound in a[] upon which sorting shall
 * be performed. offset: Index within b that result shall be written.
 */
void
pmergesort(RANDOM_NUMBER a[], RANDOM_NUMBER b[], int low, int high, int offset)
{
    PRINT_DEBUG("Merge Sort Thread ID: %d\n", omp_get_thread_num());

	// Determine the number of elements(n) that will be sorted
	int		n = high - low + 1;

	// If there is only one element, no sorting is required.The single
    // element can be written to the destination array.
		if (n == 1) {
		b[offset] = a[low];
	} else if (n > 1) {
	    //If n is non - zero, and not 1, there is sorting to be done.
		// Allocate a temporary array.
		RANDOM_NUMBER * t = malloc(n * sizeof(RANDOM_NUMBER));
		int		mid1 = (low + high) / 2;
		int		mid2 = mid1 - low + 1;

#pragma omp task if (mid1 - low > 500)
		{
			pmergesort(a, t, low, mid1, 0);
		}
#pragma omp task if (high - mid1 + 1 > 500)
		{
			pmergesort(a, t, mid1 + 1, high, mid2);
		}
#pragma omp taskwait

		//Merge the items in t in to b.
		pmerge(t, 0, mid2 - 1, mid2, n - 1, b, offset);
		free(t);
	}
}

/*
 * binary_search Performs a binary search of a given key within an array of
 * random numbers. Requires that the array is sorted. p and r specify the
 * lower and upper bounds of array to be searchd in, respectively.
 */
int
binary_search(RANDOM_NUMBER key, RANDOM_NUMBER array[], const int p, const int r)
{
	int		low = p;
	int		high = MAX(p, r + 1);
	int		mid;
	while (low < high) {
		mid = (low + high) / 2;
		if (key <= array[mid]) {
			high = mid;
		} else {
			low = mid + 1;
		}
	}
	return high;
}

void
pmerge(RANDOM_NUMBER t[], int p1, int r1, int p2, int r2, RANDOM_NUMBER a[], int p3)
{
	int		n1 = r1 - p1 + 1;
	int		n2 = r2 - p2 + 1;

	if (n1 < n2) {
		swap(&p1, &p2);
		swap(&r1, &r2);
		swap(&n1, &n2);
	}
	if (n1 == 0)
		return;

	int		q1 = (p1 + r1) / 2;
	int		q2 = binary_search(t[q1], t, p2, r2);
	int		q3 = p3 + (q1 - p1) + (q2 - p2);
	a[q3] = t[q1];

#pragma omp task if (n1 > 500)
	{
		pmerge(t, p1, q1 - 1, p2, q2 - 1, a, p3);
	}
#pragma omp task if (n1 > 500)
	{
		pmerge(t, q1 + 1, r1, q2, r2, a, q3 + 1);
	}
#pragma omp taskwait

}

// Driver - establishes OpenMP parallel construct
void omp_mergesort(RANDOM_NUMBER array[], int n)
{
#pragma omp parallel
	{
    #pragma omp single nowait
		{
		//a, b, low, high, offset
        PRINT_DEBUG("Number of threads available to Mergesort: %d\n", omp_get_max_threads());
        pmergesort(array, array, 0, n - 1, 0);
		}
	}
}
