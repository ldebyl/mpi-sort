/********************************************************************
 * CITS5007 High Performance Computing
 * University of Western Australia
 * Assignment 2
 *
 * Lee de Byl (10306807@student.uwa.edu.au
 * October 2021
 *
 * Performs merge sort on an array of random numbers in serial.
 *******************************************************************/

#include "mergesort-serial.h"

// Function Prototypes
static void pmerge(RANDOM_NUMBER t[], unsigned long p1,
    unsigned long r1, unsigned long  p2, unsigned long r2,
    RANDOM_NUMBER a[], unsigned long p3);

static void
pmergesort(RANDOM_NUMBER a[], RANDOM_NUMBER b[], unsigned long low,
        unsigned long high, unsigned long offset);

static unsigned long
binary_search(RANDOM_NUMBER key, RANDOM_NUMBER array[], const unsigned long p, const unsigned long r);

// Function Definitions
// This one can be deleted
//static int
//comp(const void *elem1, const void *elem2)
//{
//	RANDOM_NUMBER	f = *((RANDOM_NUMBER *) elem1);
//	RANDOM_NUMBER	s = *((RANDOM_NUMBER *) elem2);
//	if (f > s)
//		return 1;
//	if (f < s)
//		return -1;
//	return 0;
//}

/*
 * Swaps two integers in place. *a: pointer to integer 1 *b: pointer to
 * integer 2
 */
static void
swap(unsigned long *a, unsigned long *b)
{
	unsigned long	tmp;
	tmp = *a;
	*a = *b;
	*b = tmp;
}

/*
 * mergesort Performs a mergesort of a[] into b[].
 *
 * a[]: The array of integers to be sorted.
 * b[]: The array to sort into. Must be
 * allocated prior to calling. low: The lower bound of a[] upon which sorting
 * shall be performed. high: The upper bound in a[] upon which sorting shall
 * be performed. offset: Index within b that result shall be written.
 */
static void
pmergesort(RANDOM_NUMBER a[], RANDOM_NUMBER b[], unsigned long low, unsigned long high, unsigned long offset)
{
	// Determine the number of elements(n) that will be sorted
	unsigned long		n = high - low + 1;

	// If there is only one element, no sorting is required.The single
    // element can be written to the destination array.
		if (n == 1) {
		b[offset] = a[low];
	} else if (n > 1) {
	    //If n is non - zero, and not 1, there is sorting to be done.
		// Allocate a temporary array.
		RANDOM_NUMBER * t = malloc(n * sizeof(RANDOM_NUMBER));
		unsigned long		mid1 = (low + high) / 2;
		unsigned long		mid2 = mid1 - low + 1;
		pmergesort(a, t, low, mid1, 0);
		pmergesort(a, t, mid1 + 1, high, mid2);
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
unsigned long
binary_search(RANDOM_NUMBER key, RANDOM_NUMBER array[], const unsigned long p, const unsigned long r)
{
	unsigned long		low = p;
	unsigned long		high = MAX(p, r + 1);
	unsigned long   	mid;
	while (low < high) {
		mid = (low + high) / 2;
		// factor out, used elsewhere
		if (key <= array[mid]) {
			high = mid;
		} else {
			low = mid + 1;
		}
	}
	return high;
}

static void
pmerge(RANDOM_NUMBER t[], unsigned long p1, unsigned long r1, unsigned long  p2, unsigned long r2, RANDOM_NUMBER a[], unsigned long p3)
{
	unsigned long		n1 = r1 - p1 + 1;
	unsigned long		n2 = r2 - p2 + 1;

	if (n1 < n2) {
		swap(&p1, &p2);
		swap(&r1, &r2);
		swap(&n1, &n2);
	}
	if (n1 == 0)
		return;

	unsigned long		q1 = (p1 + r1) / 2;
	unsigned long		q2 = binary_search(t[q1], t, p2, r2);
	unsigned long		q3 = p3 + (q1 - p1) + (q2 - p2);
	a[q3] = t[q1];
	pmerge(t, p1, q1 - 1, p2, q2 - 1, a, p3);
	pmerge(t, q1 + 1, r1, q2, r2, a, q3 + 1);

}

/* Performs a merge sort on an array of random numbers
 * in serial and in-place. */
void merge_sort(RANDOM_NUMBER array[], unsigned long n)
{
	pmergesort(array, array, 0, n - 1, 0);
}
