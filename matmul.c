#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <stdlib.h>
#include <stdbool.h>

#define SIZE 1024

volatile __uint64_t A[SIZE][SIZE];
volatile __uint64_t B[SIZE][SIZE];
volatile __uint64_t BT[SIZE][SIZE];
volatile __uint64_t C[SIZE][SIZE];
volatile __uint64_t ANSWER[SIZE][SIZE];
void init(volatile __uint64_t A[][SIZE], volatile __uint64_t B[][SIZE])
{
	int r, c;

	for (c = 0; c < SIZE; c++) {
		for (r = 0; r < SIZE; r++) {
			A[r][c] = rand();
			float bfloat = rand();
			BT[c][r] = bfloat;
			B[r][c] = bfloat;
		}
	}
}

int verify(volatile __uint64_t C[][SIZE], volatile __uint64_t D[][SIZE])
{
	int r, c;

	for (c = 0; c < SIZE; c++) {
		for (r = 0; r < SIZE; r++) {
			if (C[r][c] != D [r][c]) {
				printf("error!\n");
				return 0;
			}
			
		}
	}
	return 1;
}

void matmul(volatile __uint64_t A[][SIZE], volatile __uint64_t B[][SIZE], volatile __uint64_t BT[][SIZE], bool transpose, volatile __uint64_t answer[][SIZE])
{
	int rowA, colB, idx;

	for (rowA = 0; rowA < SIZE; rowA++) {
		for (colB = 0; colB < SIZE; colB++) {
			for (idx = 0; idx < SIZE; idx++) {
				if (transpose) {
					C[rowA][colB] += A[rowA][idx] * BT[colB][idx];
				} else {
					answer[rowA][colB] += A[rowA][idx] * B[idx][colB];
				}
			}
		}
	}
}

void matmulTiling(volatile __uint64_t A[][SIZE], volatile __uint64_t B[][SIZE], int noOfTiles)
{
	int rowA, colB, idx;
	int aFirst = 0;
	int aLast = noOfTiles;
	int bFirst = 0;
	int bLast = noOfTiles;
	int idxFirst = 0;
	int idxLast = noOfTiles;
	
	while (aLast <= SIZE) {
		bFirst = 0;
		bLast = noOfTiles;
		while (bLast <= SIZE) {
			idxFirst = 0;
			idxLast = noOfTiles;
			while (idxLast <= SIZE) {
				for (rowA = aFirst; rowA < aLast; rowA++) {
					for (colB = bFirst; colB < bLast; colB++) {
						for (idx = idxFirst; idx < idxLast; idx++) {
							C[rowA][colB] += A[rowA][idx] + B[colB][idx];
						}
					}
				}
				idxFirst += noOfTiles;
				idxLast += noOfTiles;
			}
			bFirst += noOfTiles;
			bLast += noOfTiles;
		}
		aFirst += noOfTiles;
		aLast += noOfTiles;
	}	
}


int main(int argc, char **argv)
{
	clock_t t;
	double time_taken;

	init(A, B);
	memset((__uint64_t**)C, 0, sizeof(__uint64_t) * SIZE * SIZE);
	t = clock();
        matmul(A, B, BT, false, ANSWER);
        t = clock() - t;
        time_taken = ((double)t)/CLOCKS_PER_SEC; // in seconds
	printf("Matmul without transpose took %f seconds to execute \n", time_taken);

	t = clock();
        matmul(A, B, BT, true, ANSWER);
        t = clock() - t;
        time_taken = ((double)t)/CLOCKS_PER_SEC; // in seconds
        printf("Matmul with transpose took %f seconds to execute \n", time_taken);
	printf("Is verified? ->");
        printf(verify(ANSWER,C) ? "true" : "false");
        printf("\n");


	for (int i = 1; i<=1024; i*=2){	
		t = clock();
		//matmul(A, B, true);
		matmulTiling(A,B,i);
		t = clock() - t;
		time_taken = ((double)t)/CLOCKS_PER_SEC; // in seconds
		printf("Matmul blocksize:%d took %f seconds to execute \n",i, time_taken);
		printf("Is verified? ->");
        	printf(verify(ANSWER,C) ? "true" : "false");
        	printf("\n");
	}
	
}
