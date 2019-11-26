#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <stdlib.h>
#include <stdbool.h>

#define SIZE 1024

volatile __uint64_t A[SIZE][SIZE];
volatile __uint64_t B[SIZE][SIZE];
volatile __uint64_t MATMULT_WITHOUT_TRANSPOSE[SIZE][SIZE];
volatile __uint64_t MATMULT_WITH_TRANSPOSE[SIZE][SIZE];

volatile __uint64_t BT[SIZE][SIZE];
volatile __uint64_t ANSWER[SIZE][SIZE];

void init(volatile __uint64_t A[][SIZE], volatile __uint64_t B[][SIZE], volatile __uint64_t C[][SIZE])
{
	int r, c;
	volatile __uint64_t temp = 0; 

	for (c = 0; c < SIZE; c++) {
		for (r = 0; r < SIZE; r++) {
			A[r][c] = rand();
			temp = rand();
			C[c][r] = temp;
			B[r][c] = temp;
		}
	}
}

void transpose(volatile __uint64_t Transpose[][SIZE]){
	int i, j; 
	volatile __uint64_t temp; 

	for(i = 0; i < SIZE; i++){
		for(j = i + 1; j < SIZE; j++){
			temp = Transpose[i][j];
			Transpose[i][j] = Transpose[j][i];
			Transpose[j][i] = temp; 
		}
	}
}

int verify(volatile __uint64_t C[][SIZE], volatile __uint64_t D[][SIZE])
{
	int r, c;

	for (c = 0; c < SIZE; c++) {
		for (r = 0; r < SIZE; r++) {
			if (C[r][c] != D[r][c]) {
				// printf("error!\n");
				return 0;
			}
			
		}
	}
	return 1;
}

void matmul(volatile __uint64_t A[][SIZE], volatile __uint64_t B[][SIZE], volatile __uint64_t BT[][SIZE], bool transpose)
{
	int rowA, colB, idx;

	for (rowA = 0; rowA < SIZE; rowA++) {
		for (colB = 0; colB < SIZE; colB++) {
			for (idx = 0; idx < SIZE; idx++) {
				if (transpose) {
					MATMULT_WITH_TRANSPOSE[rowA][colB] += A[rowA][idx] * BT[colB][idx];
				} else {
					MATMULT_WITHOUT_TRANSPOSE[rowA][colB] += A[rowA][idx] * B[idx][colB];
				}
			}
		}
	}
}

void matmulTiling(volatile __uint64_t A[][SIZE], volatile __uint64_t X[][SIZE], int noOfTiles)
{
	int rowA, colB, idx;
	int aFirst = 0;
	int aLast = noOfTiles;
	int bFirst = 0;
	int bLast = noOfTiles;
	int idxFirst = 0;
	int idxLast = noOfTiles;
	
	while (aLast <= SIZE) {
		// bFirst = 0;
		// bLast = noOfTiles;
		while (bLast <= SIZE) {
			// idxFirst = 0;
			// idxLast = noOfTiles;
			while (idxLast <= SIZE) {
				for (rowA = aFirst; rowA < aLast; rowA++) {
					for (colB = bFirst; colB < bLast; colB++) {
						for (idx = idxFirst; idx < idxLast; idx++) {
							ANSWER[rowA][colB] += A[rowA][idx] * X[idx][colB];
						}
					}
				}
				idxFirst += noOfTiles;
				idxLast += noOfTiles;
			}
			idxFirst = 0;
			idxLast = noOfTiles;

			bFirst += noOfTiles;
			bLast += noOfTiles;
		}
		idxFirst = 0;
		idxLast = noOfTiles;

		bFirst = 0;
		bLast = noOfTiles;

		aFirst += noOfTiles;
		aLast += noOfTiles;
	}	
}


int main(int argc, char **argv)
{
	clock_t t;
	double time_taken;

	init(A, B, BT);
	memset((__uint64_t**)MATMULT_WITHOUT_TRANSPOSE, 0, sizeof(__uint64_t) * SIZE * SIZE);
	memset((__uint64_t**)MATMULT_WITH_TRANSPOSE, 0, sizeof(__uint64_t) * SIZE * SIZE);
	memset((__uint64_t**)ANSWER, 0, sizeof(__uint64_t) * SIZE * SIZE);

	transpose(BT); 
	printf(verify(B, BT) ? "true\n":"false\n");
	transpose(BT); 

	t = clock();
    matmul(A, B, BT, false);
    t = clock() - t;
    time_taken = ((double)t)/CLOCKS_PER_SEC; // in seconds
	printf("Matmul without transpose took %f seconds to execute \n\n", time_taken);

	t = clock();
    matmul(A, B, BT, true);
    t = clock() - t;
    time_taken = ((double)t)/CLOCKS_PER_SEC; // in seconds
    printf("Matmul with transpose took %f seconds to execute \n", time_taken);
	printf("Is verified? ->");
    printf(verify(MATMULT_WITHOUT_TRANSPOSE,MATMULT_WITH_TRANSPOSE) ? "true" : "false");
    printf("\n\n");


	for (int i = 1; i<=1024; i*=2){	
		t = clock();
		matmulTiling(A,B,i);
		t = clock() - t;
		time_taken = ((double)t)/CLOCKS_PER_SEC; // in seconds
		printf("Matmul blocksize:%d took %f seconds to execute \n",i, time_taken);
		printf("Is verified? ->");
        printf(verify( MATMULT_WITHOUT_TRANSPOSE, ANSWER) ? "true" : "false");
      	printf("\n\n");
	}
	
}