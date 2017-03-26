#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ucontext.h>
#include <signal.h>
#include <sys/time.h>
#include <unistd.h>
#include "mymalloc.h"
#include "my_pthread_t.h"
#include <time.h>
#include <stdint.h>

#define BILLION 1000000000L


#define malloc(x) myallocate(x,__FILE__,__LINE__, 999)
#define free(x)  mydeallocate(x,__FILE__,__LINE__, 999)



double grindA(){
	int i, count;
	char *pointers[3000];
	double diff; 
	struct timespec start, end;

	//printf("Beginning grind process A \n");

	clock_gettime(CLOCK_MONOTONIC, &start); /* mark start time */
	//Perfoming 3000 1 byte mallocs
	for (i = 0; i < 3000; i++){
		pointers[i] = malloc(1);
		if (pointers[i] != NULL)
			count++;
	}
	//freeing all pointers
	for (i = 0; i < 3000; i++){
		free(pointers[i]);
	}
	clock_gettime(CLOCK_MONOTONIC, &end); /* mark the end time */
	

	diff = (double) BILLION * (end.tv_sec - start.tv_sec) + end.tv_nsec - start.tv_nsec; 
	diff = (double) diff / BILLION;
	
	printf("\n");
	printf("\n");
	printf("\n");
	printf("NUMBER OF SUCCESSFUL mallocS: %d \n", count);
	printf("\n");
	printf("\n");
	printf("\n");
	printf("elapsed time grindA= %.5f seconds\n",  diff);	
	
	return diff * 10;
}

double grindB(){
	int i, count;
	char *pointers[3000];
	double diff; 
	struct timespec start, end;

	clock_gettime(CLOCK_MONOTONIC, &start);
	//A single byte malloc followed by 3000 frees
	char * test  = malloc(1);
	for (i = 0; i < 3000; i++)
		free(test);

	clock_gettime(CLOCK_MONOTONIC, &end); /* mark the end time */
	

	diff = (double) BILLION * (end.tv_sec - start.tv_sec) + end.tv_nsec - start.tv_nsec; 
	diff = (double) diff / BILLION;
	/*
	printf("\n");
	printf("\n");
	printf("\n");
	//printf("NUMBER OF SUCCESSFUL mallocS: \n");
	printf("\n");
	printf("\n");
	printf("\n");
	//printf("elapsed time grindB= %.5f second\n",  diff);	
	*/
	return diff * 10;
}

double grindC(){
	int MAX = 3000;
	char * pointers[3000];
	int mallocCount, freeCount, randNum;
	double diff; 
	struct timespec start, end;



	mallocCount = freeCount = 0;

	clock_gettime(CLOCK_MONOTONIC, &start);
	while(mallocCount < MAX){	//until we hit 3000 mallocs
		randNum = rand();
		randNum = randNum % 2;
		if (randNum == 0){		//if remainder is zero, mallloc and increas count
			pointers[mallocCount] = malloc(1);
			mallocCount++;
		}
		else{	//check if the freeCountis before malloc. then it is ok to free
			if (freeCount < mallocCount){
				free(pointers[freeCount]);
				freeCount++;
			}
		}

	}


	while(freeCount < MAX){
		free(pointers[freeCount]);
		freeCount++;
	}
	clock_gettime(CLOCK_MONOTONIC, &end);
	diff = (double) BILLION * (end.tv_sec - start.tv_sec) + end.tv_nsec - start.tv_nsec; 
	diff = (double) (diff / BILLION) * 10;

	
	printf("\n");
	printf("\n");
	printf("\n");
	printf("elapsed time grindC= %.5f second\n",  diff);
	
	return diff * 10;
}
double grindD(){
	int MAX = 3000;
	char * pointers[3000];
	int mallocCount, freeCount, randNum;
	double diff; 
	struct timespec start, end;



	mallocCount = freeCount = 0;

	clock_gettime(CLOCK_MONOTONIC, &start);
	while(mallocCount < MAX){	//until we hit 3000 mallocs
		randNum = rand();
		randNum = randNum % 2;
		if (randNum == 0){		//if remainder is zero, mallloc and increas count
			pointers[mallocCount] = malloc(rand() % 3000);
			mallocCount++;
		}
		else{	//check if the freeCountis before malloc. then it is ok to free
			if (freeCount < mallocCount){
				free(pointers[freeCount]);
				freeCount++;
			}
		}

	}


	while(freeCount < MAX){
		free(pointers[freeCount]);
		freeCount++;
	}
	clock_gettime(CLOCK_MONOTONIC, &end);
	diff = (double) BILLION * (end.tv_sec - start.tv_sec) + end.tv_nsec - start.tv_nsec; 
	diff = (double) diff / BILLION;

	/*
	printf("\n");
	printf("\n");
	printf("\n");
	printf("elapsed time grindD= %.5f second\n",  diff);
	*/
	return diff * 10;
}

double grindE(){

	double diff; 
	struct timespec start, end;

	clock_gettime(CLOCK_MONOTONIC, &start);
	int max = 3000;
	void * pointers[max];
	int i;
	for (i = 0; i < max; i++){
		int randNum = rand() % 5;
		int randSize = rand() % 20;
		switch (randNum) {
			case 0:
				pointers[i] = (int*)malloc(randSize*sizeof(int));
				break;
			case 1:
				pointers[i] = (float*)malloc(randSize*sizeof(float));
				break;
			case 3:
				pointers[i] = (char*)malloc(randSize*sizeof(char));
				break;
			case 4:
				pointers[i] = (double*)malloc(randSize*sizeof(double));
				break;
			default:
				break;
				// do nothing
		}
	}
	for (i = 0; i < max; i++){
		// printf("%d\n", i);
		free(pointers[i]);
	}
	


	clock_gettime(CLOCK_MONOTONIC, &end);
	diff = (double) BILLION * (end.tv_sec - start.tv_sec) + end.tv_nsec - start.tv_nsec; 
	diff = (double) diff / BILLION;
	return diff * 10;

}

double grindF(){

	double diff; 
	struct timespec start, end;

	clock_gettime(CLOCK_MONOTONIC, &start);

	int max = 3000;
	char * pointers[max];
	int size = 10;
	int i;
	for (i = 0; i < max/2; i++){
		pointers[i] = (char*)malloc(size);
		size += 5;
	}
	for (i = 0; i < max/2; i++){
		free(pointers[i]);
	}
	for (i = max/2; i < max; i++){
		pointers[i] = (char*)malloc(size);
		size -= 5;
	}
	for (i = max/2; i < max; i++){
		free(pointers[i]);
	}

	clock_gettime(CLOCK_MONOTONIC, &end);
	diff = (double) BILLION * (end.tv_sec - start.tv_sec) + end.tv_nsec - start.tv_nsec; 
	diff = (double) diff / BILLION;
	return diff * 10;

}


void callGrindA(){
	double timeA, sum, i;
	//One-hundred grindA
	sum = 0;
	for (i = 0; i < 100; i++){
		timeA = grindA();
		sum += timeA;

	}

	sum = sum / 100;
	printf("The average time for 100 grindA is: %.5f \n", sum);
	printf("\n");
	printf("\n");
}

void callGrindB(){

	double timeB, sum, i;
	//One Hundred grindB
	sum = 0;
	for (i = 0; i < 100; i++){
		timeB = grindB();
		sum += timeB;

	}

	sum = sum / 100;
	printf("The average time for 100 grindB is: %.5f \n", sum);
	printf("\n");
	printf("\n");
}

void callGrindC(){

	double timeC, sum, i;
	//One hundred grindC
	sum = 0;
	for (i = 0; i < 100; i++){
		timeC = grindC();
		sum += timeC;

	}

	sum = sum / 100;
	printf("The average time for 100 grindC is: %.5f \n", sum);
	printf("\n");
	printf("\n");	
}

void callGrindD(){

	double timeD, sum, i;
	//One hundred grindD
	sum = 0;
	for (i = 0; i < 100; i++){
		timeD = grindD();
		sum += timeD;

	}
	
	sum = sum / 100;
	printf("The average time for 100 grindD is: %.5f \n", sum);
	printf("\n");
	printf("\n");	
}

void callGrindE(){

	double timeE, sum, i;
	//One hundred grindD
	sum = 0;
	for (i = 0; i < 100; i++){
		timeE = grindE();
		sum += timeE;

	}
	
	sum = sum / 100;
	printf("The average time for 100 grindE is: %.5f \n", sum);
	printf("\n");
	printf("\n");	

}

void callGrindF(){

	double timeF, sum, i;
	// One hundred grindE
	sum = 0;
	for (i = 0; i < 100; i++){
		timeF = grindF();
		//printf("Time for process is %.5f seconds\n", timeE);
		sum += timeF;

	}
	
	sum = sum / 100;
	printf("The average time for 100 grindF is: %.5f \n", sum);
	printf("\n");
	printf("\n");	
}


int main(){

	
	srand(time(NULL));
	printf("\n");
	
	
	callGrindC();

	return 0;
}








