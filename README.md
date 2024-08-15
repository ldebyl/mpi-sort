# mpi-sort
Parallel and serial versions of three different sorting algorithms are implemented using MPI and OpenMP: enumeration sort, quicksort and merge sort. Experiments are then run to determine the relative performance of each of the algorithms in their parallel and serial forms as a function of various run-time parameters, including the size of the numeric array being sorted and the number of processes and threads.
Both the OpenMPI and OpenMP libraries are used to implement these algorithms.


## Compiling
The program includes a Makefile to compile and link the program. Simply executing `make` within the src directory will commence compiling and linking the program. Note that the `OpenMP` and `MPI` libraries are required to build the parallel versions but are not required to build the serial versions.
It may be necessary to specify the C compiler to be used. For example, on macOS using the GCC compiler provided as part of a homebrew installation:
export CC=/opt/homebrew/bin/gcc-11;make
The build process will compile both the serial (sort-serial) and parallel (sort-mpi) versions. It is possible to build each version separately, if so desired:
```
make serial
make mpi
```
Default runtime parameters can be set in config.h.
Manuals are available for the two produced executables:

```
man ./sort-mp.1
man ./sort-serial.1
```

## Sorting Algorithms
In order to parallelise the sorting algorithms it is necessary to divide the workload. We define a function, get_random_chunk() that returns a pointer and number of elements contained within that chunk for each process to consume as a function of its rank number, the total number of processes and the number of elements to be sorted:


Serial Implementation
```
let a=[array to be sorted] let b=[output array]
let n=length(a)
for i in 1..n:
     let rank=0
     for j in 1..n:
        if a[j]<a[i]:
          increment rank
        if a[i]=a[j] and j<i:
          increment rank
        b[rank]=a[i]
```

The algorithm above handles duplicates by incrementing the count (or rank) based on the order they are encountered.
Enumeration sort is easily implemented and parallelizes well. However, due to the algorithm's complexity of O(n2), it does not scale well with large values of n.
B. Parallel
The parallel implementation pushes the parallelism to the outer loop across processes. Each process is given a complete copy of the array to be sorted, which is necessary to directly determine the index in the output array each input should map to. Because each process is independently assessing the rank of item i to be sorted, the results of each process' independent work are reduced by summation to give the final output array.
Two additional versions of the Enumeration sort are included here: 

```
function get_random_chunks(array):
    let n = length of array
    let r = rank of requesting process
    let p - total number of processes

    let i = n/p * r
    let j = i + n/p - 1
    if r = p:
        j = j + n%p
    return array[i:j]
```

The algorithm above handles duplicates by incrementing the count (or rank) based on the order they are encountered.
Enumeration sort is easily implemented and parallelizes well. However, due to the algorithm's complexity of O(n2), it does not scale well with large values of n. 

### Parallel
The parallel implementation pushes the parallelism to the outer loop across processes. Each process is given a complete copy of the array to be sorted, which is necessary to directly determine the index in the output array each input should map to. Because each process is independently assessing the rank of item i to be sorted, the results of each process' independent work are reduced by summation to give the final output array.
Two additional versions of the Enumeration sort are included here: ENUMERATION_INNER and ENUMERATION_OUTER. These apply additional parallelism through threads to either the inner or outer loops.

```
within each process:
  let n=number of elements  let p=process rank  let t=total num processes
  let a=[array to be sorted]
  let b=[output array]
  let n=length(a)
  for i in 1..n:
    in OMP parallel workshare:      let rank=0      for j in 1..n:
       if a[j]<a[i]:
         increment rank
       or if a[i]=a[j] and j<i:         increment rank    reduce rank by sum
    b[rank]=a[i]
```

### Quicksort
Quicksort is a divide-and-conquer sorting algorithm that sorts in-place. Because of its design, it is suitable for parallel execution using nested tasks. It has worst-case performance of O(n2), and average case performance of O(n log n), making it generally significantly more performant that enumeration sort.

```
Serial Implementation
proc quicksort(array, a, b):	if a>0 and b>0 and a<b:		pivot=partition(array,a,b)		quicksort(array,a,pivot-1)		quicksort(array,pivot+1,b)func pivot(array,a,b):	pvt_value=array[b]	pvt_index=a-1	for i in (a, b]:		if array[i]<=pvt_value:			increment pvt_index			swap array[pvt_index], array[i]	return pvt_index<
```

###Parallel Implementation
This implementation is loosely based on the Hyperquicksort, which selects the global pivot point based on the mean of the individual process' mean pivot point in attempt to select a globally optimal pivot point.
Each process is given an approximately equal sized initial chunk of random numbers to sort, which it performs locally using an OpenMP parallelised sort using shared memory as implemented in Assignment 1. Local and global pivot points are determined using a combination of MPI Broadcasts and Reductions. Each process then exchanges partitions (referred to as halves in the source code) with its neighbour. 
Crucially, before each recursive descent, the communicator is split into new communicators, based on each process being assigned a "colour" from the expression rank/(nprocesses/2) and its rank. The recursion continues until each communicator contains a single process.
At the conclusion of the recursion, each process will contain an uneven number of elements to be collated into the final output; these will be combined in order of rank. MPI Gather and Gatherv can be used to determine the final number of elements in each process, and form displacement and count vectors to gather these results. These implementation details have been omitted from the pseudocode for brevity.