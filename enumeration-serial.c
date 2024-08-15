/********************************************************************
 * CITS5007 High Performance Computing
 * University of Western Australia
 * Assignment 2
 *
 * Lee de Byl (10306807@student.uwa.edu.au
 * October 2021
 *
 * Implements an enumeration sort in serial.
 * *******************************************************************/

#include "enumeration-serial.h"

/*
 * Performs an enumeration sort on the given array. n specifies the number of
 * elements.
 * This algorithm does not sort in place; a pointer to the new array will be
 * returned.
 */
RANDOM_NUMBER  *
enumeration_sort(RANDOM_NUMBER * array, unsigned long n)
{
	RANDOM_NUMBER  *outdata = malloc(sizeof(RANDOM_NUMBER) * n);
	int		       rank;

	for (int j = 0; j < n; j++) {
		rank = 0;
		for (int i = 0; i < n; i++) {
			if (array[i] < array[j])
				rank++;
			else if ((array[i] == array[j]) & (j < i))
				rank++;
		}

		outdata[rank] = array[j];

	}

	return outdata;
}
