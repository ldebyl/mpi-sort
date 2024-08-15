/********************************************************************
 * CITS5007 High Performance Computing
 * University of Western Australia
 * Assignment 2
 *
 * Lee de Byl (10306807@student.uwa.edu.au
 * October 2021
 *
 * Refer to the followin for information on thread-safe
 * random number generation.
 * https://www.evanjones.ca / random - thread - safe.html
 *
 * General purpose and serial random number routines.
 *******************************************************************/

#include "random-serial.h"

/* Genererates an array of n random numbers. Memory will
 * be allocated by the routine.
 */
RANDOM_NUMBER *
random_array(unsigned long n)
{
	RANDOM_NUMBER  *array = malloc(n * sizeof(RANDOM_NUMBER));
	for (unsigned long i = 0; i < n; i++) {
		array[i] = drand48();
	}

	return (array);
}

/* Writes an array of random numbers to disk as a binary file.
 * A header will be written at the beginning of the file
 * indicating the length of the file.
 *
 * Note that the file written by this routine may not be
 * portable across system architectures.
 */
int write_array(char *filename, RANDOM_NUMBER *array, unsigned long n) {
    FILE *file = fopen(filename, "wb");
    if (!file) return(-1);
    fwrite(&n, 1, sizeof(unsigned long), file);
    fwrite(array, n, sizeof(RANDOM_NUMBER), file);
    fclose(file);
    return (0);
}

/* Reads an array from disk as written by write_array()
 * Returns the number of elements read from disk.
 * The filename, as well as a pointer, shall be passed
 * as arguments. The routine will allocate sufficient memory
 * based on the number of elements in the file, which may
 * not be known ahead of time.
 */
unsigned long read_array(char *filename, RANDOM_NUMBER **array) {
    unsigned long n = -1;
    FILE *file = fopen(filename, "rb");
    if (!file) return(-1);

    fread(&n, 1, sizeof(unsigned long), file);
    // Allocate memory to read the array in to.

    *array = malloc(n * sizeof(RANDOM_NUMBER));
    fread(*array, n, sizeof(RANDOM_NUMBER), file);
    fclose(file);
    return (n);
}

/* Validates an array written to disk. It compares the in-memory
 * array against the written file.
 *
 * Returns either BAD_COUNT or BAD_COMPARE if any issues are identified.
 * The number of expected elements shall be provided as an argument.
 */
int validate_array(char *filename, RANDOM_NUMBER *array, unsigned long n) {
    RANDOM_NUMBER *array2 = 0;
    unsigned long n2 = read_array(filename, &array2);
    int status = 0;

    // Perform the validation
    if (n != n2) {
        status = BAD_COUNT;
    } else {
        // Sizes match - perform element-wise comparison.
        for (unsigned long i=0; i<n; i++) {
            //printf("Index: %ld  Memory: %f   Disk: %f\n", i, array[i], array2[i]);
            if (array[i] != array2[i]) {
                fprintf(stderr, "Validation of written array failed at element %ld.\n", i);
                status = BAD_COMPARE;
                break;
            }
        }
    }

    free(array2);
    return(status);
}
