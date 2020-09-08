/*
 ============================================================================
 Name        : FinalProject.c
 Author      : Roman Prasolov id- 313091746
 ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#include <mpi.h>
#include <math.h>

#include "mutantSequenceOpenMP.h"
#include "cudaFunctions.h" 
#include "cFunction.h"

// calculate the best score by using OpenMP and cuda.
// returns double array with the best score 
double* mutantSequences(char str1[], char str2[], int startOffset, int endOffset, double* weight) { 
	int size = strlen(str2);
	double *best, *temp;
	// allocate double array for best score
	best = (double*) malloc(sizeof(double)*3);
	if (best == NULL) {
		fprintf(stderr, "Couldn't allocate array");
		MPI_Abort(MPI_COMM_WORLD, 1);
	}
	
	best[2] = -INFINITY;
	omp_lock_t lock;
	omp_init_lock(&lock);

#pragma omp parallel for private(temp)
	for (int i = 0; i <= size; i++) {
		// divide the offsets to two parts for each procces
		if (!i) {
			temp = computeOnGPU(str1, str2, size, 0, startOffset, endOffset, weight);
		} else {
			temp = computeOnGPU(str1, addMutant(str2, i), size+1, i, startOffset, endOffset-1, weight);
		}
		omp_set_lock(&lock);
		// sets of the best score
		if (temp[2] > best[2]) {
			best[0] = temp[0];
			best[1] = temp[1];
			best[2] = temp[2];
		}
		omp_unset_lock(&lock);
	}
	omp_destroy_lock(&lock);

return best;
}
