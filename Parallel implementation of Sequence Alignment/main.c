/*
 ============================================================================
 Name        : FinalProject.c
 Author      : Roman Prasolov id- 313091746
 ============================================================================
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <mpi.h>

#include "cFunction.h"
#define MAX_Processes 2

int main(int argc, char *argv[]) {

	// rank of process
	int rankNumber; 
	
	// number of processes 
	int numOfProcesses; 

	// start up MPI 
	MPI_Init(&argc, &argv);

	// find out process rank 
	MPI_Comm_rank(MPI_COMM_WORLD, &rankNumber);

	// find out number of processes 
	MPI_Comm_size(MPI_COMM_WORLD, &numOfProcesses);

	// check if number of processes is allowed 
	if (numOfProcesses != MAX_Processes)
	{
		printf("Only %d processes are allowed", MAX_Processes);
		MPI_Finalize();
		MPI_Abort(MPI_COMM_WORLD, 1);
	}
	
	// 0- is the master process 1- is the slave process
	if (rankNumber == 0) 
		// start master work
		master(argc, argv); 
	else 
		// start slave work
		slave(); 

	// shutdown MPI 
	MPI_Finalize();

	return 0;
}
