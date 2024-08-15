# mpi-sort
Parallel and serial versions of three different sorting algorithms are implemented using MPI and OpenMP: enumeration sort, quicksort and merge sort. Experiments are then run to determine the relative performance of each of the algorithms in their parallel and serial forms as a function of various run-time parameters, including the size of the numeric array being sorted and the number of processes and threads.
Both the OpenMPI and OpenMP libraries are used to implement these algorithms.


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
