#####################################################################
# Makefile for CITS5007 Assignment r21
# Lee de Byl
# October, 2021
# 10306807@student.uwa.edu.au
#####################################################################

# Use the current directory for includes, optimise, and produce warnings.
OPTS = -I. -o3 -Wall
DEPS = $(wildcard *.h)
# CC=/opt/homebrew/bin/gcc-11

default: clean all

all: sort-serial sort-mpi

# Builds the serial version of the sort
serial: sort-serial

# Builds the MPI/OpenMP version
mpi: sort-mpi

%-mpi.o: %-mpi.c $(DEPS)
	mpicc $(OPTS) -c -o $@ $< -fopenmp

%.o: %.c $(DEPS)
	$(CC) $(OPTS) -Wno-unknown-pragmas -c -o $@ $<

# Builds a multiprocessing (MPI/OpenMP) version of the sort program
sort-mpi: sort.o random-serial.o random-mpi.o sort-mpi.o enumeration-mpi.o quicksort-mpi.o quicksort-serial.o mergesort-mpi.o mergesort-omp.o enumeration-serial.o quicksort-serial.o mergesort-serial.o
	mpicc $(OPTS) -o $@ $^ -fopenmp

# Builds a serial version of the sort program.
# Stop GCC complaining about unknown pragmas when OpenMP missing.
sort-serial: sort-serial.o sort.o random-serial.o enumeration-serial.o quicksort-serial.o mergesort-serial.o
	$(CC) $(OPTS) -Wno-unknown-pragmas -o $@ $^

# Formats C source files
pretty:
	find . -iname '*.[ch]' -prune -exec indent {} \;

clean:
	rm -f sort-serial sort-mpi *.o *.BAK

submission:
	cd .. && tar -cvzf assignment2-10306807.tgz src/run_experiments src/*.c src/*.h src/Makefile src/*.1 results assignment2-10306807.pdf
