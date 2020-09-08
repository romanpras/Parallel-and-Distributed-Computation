/*
 ============================================================================
 Name        : FinalProject.c
 Author      : Roman Prasolov id- 313091746
 ============================================================================
 */
 
#ifndef CFUNCTION_H
#define CFUNCTION_H

#pragma once  
#define MAIN_MAX_LENGTH 3000
#define SEQ_MAX_LENGTH 2000

void master(int argc, char *argv[]);
void slave();
double calculateScore(double* weight, char* result);
void initDataFromFile(const char *filename, double *weight, char** mainSequence,int* numberOfSeq, char*** arrOfSeq);
char* readSequenceFromFile(FILE* file, int maxSize);
void writeBestScore(double* best, int numberOfBest);
char* addMutant(char str[], int index);

#endif
