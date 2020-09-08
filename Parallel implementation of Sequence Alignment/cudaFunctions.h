/*
 ============================================================================
 Name        : FinalProject.c
 Author      : Roman Prasolov id- 313091746
 ============================================================================
 */
 
#ifndef CUDAFUNCTIONS_H
#define CUDAFUNCTIONS_H

#pragma once 


char* mallocForStr(size_t sizeT);
void memCopyForStr(size_t sizeT,char *str, char *data,int direction);
double* computeOnGPU(char str1[], char str2[], int size,int mutant, int startOffset, int endOffset, double* weight);
  
#endif
