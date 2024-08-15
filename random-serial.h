#include "random.h"

RANDOM_NUMBER  *random_array(unsigned long n);
int write_array(char *filename, RANDOM_NUMBER *array, unsigned long n);
unsigned long read_array(char *filename, RANDOM_NUMBER **array);
int validate_array(char *filename, RANDOM_NUMBER *array, unsigned long n);
