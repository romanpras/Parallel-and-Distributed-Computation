/*
 ============================================================================
 Name        : FinalProject.c
 Author      : Roman Prasolov id- 313091746
 Version     :
 Copyright   : Your copyright notice
 Description : Cuda Functions
 ============================================================================
 */
 
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>

#include "cudaFunctions.h"
#include "cFunction.h"

// checks if str1 and str2 belong to the same conservative group
__device__ void groupCheck(const char *conservative[], int size, char result[], int index, char str1, char str2, char sign) {
	int group1, group2;
	for (int j = 0; j < size; j++) { //size of groups array
		group1 = -1;
		group2 = -1;
		for (int k = 0; conservative[j][k] != '\0'; k++) {
			if (group1 == -1 && str1 == conservative[j][k])
				group1 = j;
			if (group2 == -1 && str2 == conservative[j][k])
				group2 = j;
			if (group1 != -1 && group1 == group2) {
				result[index] = sign;
				return;
			}
		}
	}
}

// compare pair in semi-conservative groups
__device__ void semiConservativeGroupAction(char str1, char str2, int index, char result[]) {
	const char *semiConservative[11] = { "SAG", "ATV", "CSA", "SGND", "STPA", "STNK", "NEQHRK", "NDEQHK", "SNDEQK", "HFY", "FVLIM" };
	groupCheck(semiConservative, 11, result, index, str1,str2, '.');
}

// compare pair in conservative groups
__device__ void conservativeGroupAction(char str1, char str2, int index, char result[]) {
	const char *conservative[9] = { "NDEQ", "NEQK", "STA", "MILV", "QHRK", "NHQK", "FYW", "HY", "MILF" };
	groupCheck(conservative, 9, result, index, str1, str2,':');
}

// compare 2 sequences with offset
__global__ void compareStringsWithOffset(char *str1, char *str2, char *result, int size, int offset) {
	
	int index = blockIdx.x * blockDim.x + threadIdx.x;
	if (index < size) {
		result[index] = ' ';
		if (str2[index] == '-')
			return;
		if (str1[index + offset] == str2[index])
			result[index] = '*';
		else {
			conservativeGroupAction(str1[index + offset], str2[index], index, result);
			if (result[index] != ':')
				semiConservativeGroupAction(str1[index + offset], str2[index], index,result);
		}
	}
}

// allocate device memory
char* mallocForStr(size_t sizeT){
	char *str;
	cudaError_t err = cudaMalloc((void **)&str, sizeT);
	if (err != cudaSuccess) {
        	fprintf(stderr, "Failed to allocate device memory - %s\n", cudaGetErrorString(err));
        	exit(EXIT_FAILURE);
    }
	return str;
}

// copy str to memory according to direction 0
void memCopyForStr(size_t sizeT,char *str, char *data, int direction){
	cudaError_t err;
	if (direction==1)
		err = cudaMemcpy(str, data, sizeT, cudaMemcpyDeviceToHost);
	else 
		err = cudaMemcpy(str, data, sizeT, cudaMemcpyHostToDevice);
    if (err != cudaSuccess) {
        fprintf(stderr, "Failed to copy data from %s - %s\n", (direction==0 ? "host to device" : "device to host"), cudaGetErrorString(err));
        exit(EXIT_FAILURE);
    }
}


// return best result as double* of all opportunities of offsets.
double* computeOnGPU(char str1[], char str2[], int size,int mutant, int startOffset, int endOffset, double* weight) {

	size_t sizeT = size; 
	char *d_str1,*d_str2,*d_result, *result;
	
	// 0 - cudaMemcpyHostToDevice
	int direction=0; 

	double *best = (double*) malloc(sizeof(double)*3);
	if (best==NULL) {
		printf("Failed to allocate the best array memory\n");
		exit(1);
	}

	// best offset
	best[0] = 0;
	
	// best mutant
	best[1] = mutant;
	
	// best score
	best[2] = -INFINITY;
	
	// allocate memory on GPU to copy the data from the host
	d_str1=mallocForStr(strlen(str1));
	d_str2=mallocForStr(sizeT);
	d_result=mallocForStr(sizeT);
	
	// allocate memory for the result 
	result = (char*)malloc(sizeof(char)*sizeT);
	if (!result) {
		fprintf(stderr, "Failed to allocate host memory\n");
        	exit(EXIT_FAILURE);
	}
	
	// Copy data from host to the GPU memory 
	memCopyForStr(strlen(str1), d_str1, str1, direction);
	memCopyForStr(sizeT, d_str2, str2, direction);
	
	// Launch the Kernel: calculate number of threads and blocks 
	int threadsPerBlock;
	int blocksPerGrid;
	if (size <= 1024){
		threadsPerBlock = size;
	}
	else {
		threadsPerBlock = 1024;
	}
	blocksPerGrid=size/threadsPerBlock;
	if (size%threadsPerBlock!=0)
		blocksPerGrid++;
		
	// 1 - cudaMemcpyDeviceToHost 
	direction=1; 
	double currentBest[2];
	
	// calculate thes best score of the all offsets (startOffset to endOffset) 
	for (int i = startOffset; i < endOffset; i++) {
		currentBest[0] = i;
		compareStringsWithOffset<<<blocksPerGrid,threadsPerBlock>>>(d_str1, d_str2, d_result, size, i);
		memCopyForStr(sizeT, result, d_result, direction);
		result[sizeT] = '\0';
		
		// calculate the score
		currentBest[1] = calculateScore(weight, result); 
		
		// sets the best score
		if (currentBest[1] > best[2]) {
			best[2] = currentBest[1];
			best[0] = currentBest[0];
		}
	}
	
	// free the memory
	cudaFree(d_str1);
	cudaFree(d_str2);
	cudaFree(d_result);
	free(result);
    return best;
}

