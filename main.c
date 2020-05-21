/*
 * main.c
 *
 *  Created on: May 17, 2020
 *      Author: Muhammad & Shadi
 */

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <time.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include "spmat.h"
#include "SPBufferset.h"

#define EPSILON 0.00001

typedef enum {LIST, ARRAY} IMP_TYPE;
typedef enum {false, true} boolean;

/* Prototypes */
void create_random_vector(double *vector, int n);
IMP_TYPE find_implementation(char* flag);
int calculateNNZ(FILE* input_matrix, unsigned int n);
boolean calculate_next(double *b_next, double *b_prev, spmat *matrix);

int main(int argc, char **argv) {

	FILE *input_matrix, *initial_vector, *eigen_vector;
	unsigned int res, n, nnz, vals[2];
	register unsigned int i;
	double *b_prev, *row, *b_next, *temp;
	clock_t start, end;
	spmat *matrix;

	IMP_TYPE imp_type;

	start = clock();
	SP_BUFF_SET();
	srand(time(NULL));
	assert(argc == 4 || argc == 5);

	input_matrix = fopen(argv[1], "r");
	assert(input_matrix != NULL);

	res = fread(&n, sizeof(int), 1, input_matrix);
	assert(res == 1);

	b_prev = (double*) malloc(n * sizeof(double));
	assert(b_prev != NULL);

	if(argc == 4){
		imp_type = find_implementation(argv[3]);
		create_random_vector(b_prev, n);

		eigen_vector = fopen(argv[2], "w");
		assert(eigen_vector != NULL);
	}


	else /* argc == 5 */ {
		imp_type = find_implementation(argv[4]);
		initial_vector = fopen(argv[2], "r");
		assert(initial_vector != NULL);

		res = fseek(initial_vector, 2 * sizeof(int), SEEK_SET);
		assert(res == 0);

		res = fread(b_prev, sizeof(double), n, initial_vector);
		assert(res == n);

		eigen_vector = fopen(argv[3], "w");
		assert(eigen_vector != NULL);
	}

	/* converting input matrix into sparse matrix */

	if(imp_type == LIST)
		matrix = spmat_allocate_list(n);

	else /* imp_type == ARRAY */ {
		nnz = calculateNNZ(input_matrix, n);
		matrix = spmat_allocate_array(n, nnz);
	}

	res = fseek(input_matrix, 2 * sizeof(int), SEEK_SET);
	assert(res == 0);

	row = (double*)malloc(n * sizeof(double));
	assert(row != NULL);

	for(i = 0; i < n; i++){

		res = fread(row, sizeof(double), n, input_matrix);
		assert(res == n);

		matrix->add_row(matrix, row, i);

	}

	b_next = row;
	assert(b_next != NULL);

	vals[0] = 1;
	vals[1] = n;

	res = fwrite(vals, sizeof(int), 2, eigen_vector);
	assert(res == 2);

	while(1){

		if(!calculate_next(b_next, b_prev, matrix)){
			temp = b_prev;
			b_prev = b_next;
			b_next = temp;
		}

		else {
			res = fwrite(b_next, sizeof(double), n, eigen_vector);
			assert(res == n);
			break;
		}

	}

	/* free memory */

	free(b_prev);
	free(b_next);

	matrix->free(matrix);
	free(matrix);

	if(argc == 5){
		fclose(initial_vector);
	}
	fclose(input_matrix);
	fclose(eigen_vector);

	end = clock();
	printf("Prog  took: %f seconds\n", (double)(end - start) / CLOCKS_PER_SEC);

	return 0;
}

void create_random_vector(double *vector, int n){

	register unsigned int i;

	for(i = n; i != 0; i--){
		*vector = rand();
		vector++;
	}

}

IMP_TYPE find_implementation(char* flag){

	if(strcmp(flag, "-list") == 0)
		return LIST;

	assert(strcmp(flag, "-array") == 0);

	return ARRAY;

}

int calculateNNZ(FILE* input_matrix, unsigned int n){

	unsigned int res;
	register unsigned int i, j, cnt = 0;
	double *row;
	register double *ptr;

	res = fseek(input_matrix, 2 * sizeof(int), SEEK_SET);
	assert(res == 0);

	row = (double*)malloc(n * sizeof(double));
	assert(row != NULL);

	for(i = n; i != 0; i--){

		res = fread(row, sizeof(double), n, input_matrix);
		assert(res == n);

		ptr = row;

		for(j = n; j != 0; j--){
			cnt += (*ptr != 0);
			ptr++;
		}

	}

	free(row);

	return cnt;
}


boolean calculate_next(double *b_next, double *b_prev, spmat *matrix){

	unsigned int n = matrix->n;
	register unsigned int i;
	register double *ptr_1 = b_next, *ptr_2 = b_prev;
	register double val, norm = 0, norm_inverse;
	boolean res = true;

	matrix->mult(matrix, b_prev, b_next);

	for(i = 0; i < n; i++){
		val = *ptr_1;
		norm += val * val;
		ptr_1++;
	}

	norm = sqrt(norm);
	norm_inverse = 1 / norm;

	ptr_1 = b_next;
	ptr_2 = b_prev;

	for(i = n; i != 0; i--){

		(*ptr_1) *= norm_inverse;

		if(res && fabs((*ptr_1)-(*ptr_2)) >= EPSILON)
			res = false;

		ptr_1++;
		ptr_2++;
	}

	return res;
}

