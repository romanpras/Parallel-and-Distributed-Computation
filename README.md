# Parallel-and-Distributed-Computation
Parallel and distributed computing with MPI, OpenMP, Cuda, C library

The program must test the highest match between two DNA sequences, by moving an offset and by adding a mutant to the sequence. 
The calculation is done by creating a new sequence according to the test and the weighted amount is calculated using weights given to us.
All this is done using the MPI, OpenMP, Cuda.


Steps to take:
First of all you need to modify the mf file so that the IP addresses match the environments in which you work.
Then open the terminal and run the "make" after that run the "make run".
If you want to run on two computers, run "make" on each of them and then run "make runOn2".
The program will run and after a while an output.txt file will be created with the calculation results.
_______________________________________________________________________________________________________________________________________________
Summary:
The program must test the highest match between two sequences, by moving an offset and by adding a mutant to the sequence.
The calculation is done by creating a new sequence according to the test and the weighted amount is calculated using weights given to us.
More details about the task in another file that details the task.
_______________________________________________________________________________________________________________________________________________
Execution stages of the program:
The program is run by the MPI who is responsible for dividing the work between two processors - Master and Slave.
Each other is powered by a function that works with OpenMP, which is responsible for dividing the total offset into both, the master gets a certain range, and the slave gets a certain range.
The master's job is to read the data from the input.txt file and send it to the processor.
He must then calculate the best result between him and the result of the slave and write the result to the output.txt file.
The job of the slave is to calculate the best result from the data he received from the master and send these results back to him.
The results are calculated using CUDA, which is responsible for comparing 2 sequences with a particular mutant offset, and returns the result in which the offset and the mutant yield a maximum match of two sequences.
