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
#include <omp.h>

#include "cFunction.h"
#include "cudaFunctions.h"
#include "mutantSequenceOpenMP.h"

// master func
void master(int argc, char *argv[])
{
	double *weight; 
	char *mainSequence;
	int numberOfSeq;
	char **arrOfSeq;
	int sizeOfStr;
	
	MPI_Status status;
	
	// allocate char array for creating mainSequence
	weight = (double*) malloc(sizeof(double)*4); 
	if (weight == NULL){
		fprintf(stderr, "Couldn't allocate array");
		MPI_Abort(MPI_COMM_WORLD, 1);
	}
	
	// read from file
	initDataFromFile(argv[1], weight, &mainSequence, &numberOfSeq, &arrOfSeq);
	
	// send num of sequences to the slave
	MPI_Send(&numberOfSeq, 1, MPI_INT, 1, 0, MPI_COMM_WORLD);
	
	// send mainSequence to slave
	sizeOfStr = strlen(mainSequence);
	MPI_Send(&sizeOfStr, 1, MPI_INT, 1, 0, MPI_COMM_WORLD);
	MPI_Send(mainSequence, sizeOfStr, MPI_CHAR, 1, 0, MPI_COMM_WORLD);

	// send arrOfSeq to slave
	for (int i = 0; i < numberOfSeq; i++){
		sizeOfStr = strlen(arrOfSeq[i]);
		MPI_Send(&sizeOfStr, 1, MPI_INT, 1, 0, MPI_COMM_WORLD);
		MPI_Send(arrOfSeq[i], sizeOfStr, MPI_CHAR, 1, 0, MPI_COMM_WORLD);
	}

	// send array of weights to slave
	MPI_Send(weight, 4, MPI_DOUBLE, 1, 0, MPI_COMM_WORLD);

	// allocate double array for creating best score from Master
	double* bestMaster = (double*) malloc(sizeof(double) * 3*numberOfSeq);
	
	// perform all master's calculations with openMP
	for (int i = 0; i < numberOfSeq; i++)
	{
		int mainSequenceLength = strlen(mainSequence);
		int otherSequenceLength = strlen(arrOfSeq[i]);
		int middleOffset = ((mainSequenceLength-otherSequenceLength)/2)+1;
		double *tempBest = mutantSequences(mainSequence, arrOfSeq[i], 0, middleOffset, weight); 
		bestMaster[i*3] = tempBest[0];
		bestMaster[i*3+1] = tempBest[1];
		bestMaster[i*3+2] = tempBest[2];
		free(tempBest);
		
	}

	// receive all results from slave and compare to master results of each sequence
	double tempBestSlave[3];
	for (int i = 0; i < numberOfSeq; i++)
	{
		MPI_Recv(tempBestSlave, 3, MPI_DOUBLE, 1, 0, MPI_COMM_WORLD,&status);
		if (tempBestSlave[2] > bestMaster[i*3+2]) {
			bestMaster[i*3] = tempBestSlave[0];
			bestMaster[i*3+1] = tempBestSlave[1];
			bestMaster[i*3+2] = tempBestSlave[2];
		}
	}
	// write to file the best score
	writeBestScore(bestMaster, numberOfSeq);
	
	// free the memory
	for (int i = 0; i < numberOfSeq; i++) {
		free(arrOfSeq[i]);
	}
	free(arrOfSeq);
	free(bestMaster);
}

// slave program
void slave()
{
	double weight[4];
	char *mainSequence;
	int numberOfSeq;
	char **arrOfSeq;
	int sizeOfStr;
	
	MPI_Status status;

	// recive num of sequences from master
	MPI_Recv(&numberOfSeq, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, &status);
	
	// allocate char array for creating mainSequence
	arrOfSeq = (char**) malloc(sizeof(char*)*numberOfSeq); 
	
	if (arrOfSeq == NULL){
		fprintf(stderr, "Couldn't allocate array");
		MPI_Abort(MPI_COMM_WORLD, 1);
	}
	
	// recive mainSequence size from master
	MPI_Recv(&sizeOfStr, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, &status);
	
	// allocate for creating mainSequence
	mainSequence = (char*) malloc(sizeof(char) * sizeOfStr);
	if (!mainSequence) {
		fprintf(stderr, "Couldn't allocate array");
		MPI_Abort(MPI_COMM_WORLD, 1);
	}
	// recive mainSequence from master
	MPI_Recv(mainSequence, sizeOfStr, MPI_CHAR, 0, 0, MPI_COMM_WORLD, &status);

	// recive arrOfSeq from master
	for (int i = 0; i < numberOfSeq; i++){
		MPI_Recv(&sizeOfStr, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, &status);
		arrOfSeq[i] = (char*) malloc(sizeof(char) * sizeOfStr);
		if (!arrOfSeq[i]) {
			fprintf(stderr, "Couldn't allocate array");
			MPI_Abort(MPI_COMM_WORLD, 1);
		}
		MPI_Recv(arrOfSeq[i], sizeOfStr, MPI_CHAR, 0, 0, MPI_COMM_WORLD, &status);
	}
	
	// recive array of weights from master
	MPI_Recv(&weight, 4, MPI_DOUBLE, 0, 0, MPI_COMM_WORLD, &status);

	// allocate double array for creating best score from slave
	double* bestSlave = (double*) malloc(sizeof(double) * 3*numberOfSeq);
	
	// perform all slave's calculations with openmp
	for (int i = 0; i < numberOfSeq; i++)
	{
		int mainSequenceLength = strlen(mainSequence);
		int otherSequenceLength = strlen(arrOfSeq[i]);
		int middleOffset = ((mainSequenceLength-otherSequenceLength)/2)+1;
		double *tempBest = mutantSequences(mainSequence, arrOfSeq[i], middleOffset, mainSequenceLength-otherSequenceLength+1, weight); 
		bestSlave[i*3] = tempBest[0];
		bestSlave[i*3+1] = tempBest[1];
		bestSlave[i*3+2] = tempBest[2];
		free(tempBest);
	}

	// send all results to master
	for (int i = 0; i < numberOfSeq; i++)
	{
		MPI_Send(&bestSlave[i*3], 3, MPI_DOUBLE, 0, 0, MPI_COMM_WORLD);
	}

	// free the memory
	for (int i = 0; i < numberOfSeq; i++) {
		free(arrOfSeq[i]);
	}
	free(arrOfSeq);
	free(bestSlave);
}

	// calculate the score according to weights
double calculateScore(double* weight, char* result) {
	int numOfStars = 0, numOfColons = 0, numOfPoints = 0, numOfSpaces = 0;
	for (int i = 0; i < strlen(result); i++) {
		if (result[i] == '*')
			numOfStars++;
		else if (result[i] == ':')
			numOfColons++;
		else if (result[i] == '.')
			numOfPoints++;
		else
			numOfSpaces++;
	}
	double score = (weight[0] * numOfStars) - (weight[1]  * numOfColons) - (weight[2]  * numOfPoints) - (weight[3]  * numOfSpaces);
	return score;
}

	// init data from file
void initDataFromFile(const char *filename, double *weight, char** mainSequence,int* numberOfSeq, char*** arrOfSeq){
	FILE* file = fopen(filename, "r");
	char c;
	// check if file exist
	if (file==NULL) {
		fprintf(stderr, "Couldn't open file");
		MPI_Abort(MPI_COMM_WORLD, 1);
	}
	// reads weights from file
	if (fscanf(file, "%lf%lf%lf%lf", &weight[0], &weight[1], &weight[2], &weight[3]) != 4) {
		fprintf(stderr, "Couldn't read weights");
		MPI_Abort(MPI_COMM_WORLD, 1);
	}
	// reads the rest weights line from file
	while ((c = fgetc(file)) != '\n');
	
	// reads sequnce from file
	*mainSequence = readSequenceFromFile(file, MAIN_MAX_LENGTH);
	if (*mainSequence == NULL) {
		fprintf(stderr, "Couldn't read sequence");
		MPI_Abort(MPI_COMM_WORLD, 1);
	}
	// reads number of sequences from file
	if (fscanf(file, "%d", numberOfSeq) != 1) {
		fprintf(stderr, "Couldn't read weights");
		MPI_Abort(MPI_COMM_WORLD, 1);
	}
	// reads the rest number of sequences line from file
	while ((c = fgetc(file)) != '\n');
	
	// allocate char array for creating array of Sequences
	*arrOfSeq = (char**) malloc(sizeof(char*)*(*numberOfSeq)); 
	if (*arrOfSeq == NULL){
		fprintf(stderr, "Couldn't allocate array");
		MPI_Abort(MPI_COMM_WORLD, 1);
	}
	
	// reads Sequences from file
	for (int i = 0; i < *numberOfSeq; i++) {
		(*arrOfSeq)[i] = readSequenceFromFile(file, SEQ_MAX_LENGTH);
		if ((*arrOfSeq)[i] == NULL) {
			fprintf(stderr, "Couldn't read sequence");
			MPI_Abort(MPI_COMM_WORLD, 1);
		}
	}
	// close file
	fclose(file);
}

	// reads sequence from file
char* readSequenceFromFile(FILE* file, int maxSize) {
	char sequence[maxSize];
	if (fgets(sequence, maxSize, file) != NULL) {
		sequence[strcspn(sequence, "\r\n")] = '\0';
		return strdup(sequence);
	}
	return NULL;
}

// write the best score to output file
void writeBestScore(double* best, int numberOfBestScore) {
	FILE* file = fopen("output.txt", "w");
	if (!file) {
		fprintf(stderr, "Couldn't open file");
		MPI_Abort(MPI_COMM_WORLD, 1);
	}
	for (int i = 0; i < numberOfBestScore; i++) {
		fprintf(file, "The Best offset: %d, The Best mutant: %d, The Best Score: %f\n", (int)best[i*3], (int)best[i*3+1],(double)best[i*3+2]);
	}
	fclose(file);
}
	
// adds mutant into the sequence in the index place
char* addMutant(char str[], int index) {
	int i, len;
	len = strlen(str) + 1;
	char temp[len];

	for (i = 0; i < index; i++) {
		temp[i] = str[i];
	}
	temp[index] = '-';
	for (i = index + 1; i < len; i++) {
		temp[i] = str[i - 1];
	}
	return strdup(temp);
}


