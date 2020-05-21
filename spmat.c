/*
 * spmat.c
 *
 *  Created on: May 17, 2020
 *      Author: Muhammad & Shadi
 */


#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <math.h>
#include "spmat.h"
#include "SPBufferset.h"

typedef enum {false, true} boolean;

struct linked_list_t {
	double value;
	unsigned int column;
	struct linked_list_t *next;
};

typedef struct linked_list_t Element;

/* List - Functions' Prototypes */
void list_imp_add_row(struct _spmat *A, const double *row, int i);
void list_imp_free(struct _spmat *A);
void list_imp_mult(const struct _spmat *A, const double *v, double *result);

/* List - Implementation */

spmat* spmat_allocate_list(int n){

	Element** linked_lists;

	spmat* matrix = (spmat*)malloc(sizeof(spmat));
	assert(matrix != NULL);

	matrix->n = n;

	matrix->add_row = list_imp_add_row;
	matrix->free = list_imp_free;
	matrix->mult = list_imp_mult;

	linked_lists = (Element**)malloc(n * sizeof(Element*));
	assert(linked_lists != NULL);

	matrix->private = linked_lists;

	return matrix;

}


void list_imp_add_row(struct _spmat *A, const double *row, int i){

	register Element *head = NULL, *tail;
	unsigned int n;
	register unsigned int col;
	register const double *ptr = row;
	double val;
	boolean not_initialized = true;
	size_t sizeof_element = sizeof(Element);

	n = A->n;

	for(col = 0; col < n; col++){

		val = *ptr;

		if(val == 0){
			ptr++;
			continue;
		}

		if(not_initialized && val != 0){
			head = (Element*)malloc(sizeof_element);
			assert(head != NULL);
			not_initialized = false;
			head->value = val;
			head->column = col;
			tail = head;
			ptr++;
			continue;
		}

		tail->next = (Element *)malloc(sizeof_element);
		assert(tail->next != NULL);
		tail = tail->next;
		tail->value = val;
		tail->column = col;
		ptr++;

	}

	tail->next = NULL;

	((Element**)(A->private))[i] = head;
}


void list_imp_free(struct _spmat *A){

	unsigned int n = A->n;
	register unsigned int i;
	register Element **linked_lists = (Element**)(A->private);
	register Element *current, *next;

	for(i = n; i != 0; i--){

		current = *linked_lists;

		while(current != NULL){
			next = current->next;
			free(current);
			current = next;
		}


		linked_lists++;
	}

	free(A->private);

}


void list_imp_mult(const struct _spmat *A, const double *v, double *result){

	unsigned int n = A->n;
	register unsigned int i;
	register Element **ptr_element = (Element**)(A->private);
	register Element *head;
	register double * ptr_res = result, res;

	for(i = 0; i < n; i++){

		head = *ptr_element;
		res = 0;

		while(head != NULL){

			res += (head->value) * v[head->column];
			head = head->next;

		}

		*ptr_res = res;

		ptr_element++;
		ptr_res++;

	}

}


/* ------------------------------------------------------------------------- */


/* Array - Functions' Prototypes */

typedef struct {

	double *vaules;
	int *colind;
	int *rowptr;
	unsigned int first_empty;
	unsigned int nnz;

} Array_Rep;

void array_imp_add_row(struct _spmat *A, const double *row, int i);
void array_imp_free(struct _spmat *A);
void array_imp_mult(const struct _spmat *A, const double *v, double *result);


/* Array - Implementation */

spmat* spmat_allocate_array(int n, int nnz){

	Array_Rep *array_rep;

	spmat* matrix = (spmat*)malloc(sizeof(spmat));
	assert(matrix != NULL);

	matrix->n = n;

	matrix->add_row = array_imp_add_row;
	matrix->free = array_imp_free;
	matrix->mult = array_imp_mult;

	array_rep = (Array_Rep*)malloc(sizeof(Array_Rep));
	assert(array_rep != NULL);

	array_rep->vaules = (double*)malloc(nnz * sizeof(double));
	assert(array_rep->vaules != NULL);

	array_rep->colind = (int*)malloc(nnz * sizeof(int));
	assert(array_rep->colind != NULL);

	array_rep->rowptr = (int*)malloc((n + 1) * sizeof(int));
	assert(array_rep->rowptr != NULL);
	(array_rep->rowptr)[n] = nnz;

	array_rep->first_empty = 0;
	array_rep->nnz = nnz;

	matrix->private = array_rep;

	return matrix;

}


void array_imp_add_row(struct _spmat *A, const double *row, int i){

	Array_Rep *array_rep = (Array_Rep*)(A->private);
	int n = A->n;
	unsigned int j;
	register int col;
	register const double *ptr = row;
	register double *ptr_1;
	register int *ptr_2;
	double val;

	j = array_rep->first_empty;
	(array_rep->rowptr)[i] = j;
	ptr_1 = array_rep->vaules + j;
	ptr_2 = array_rep->colind + j;

	for(col = 0; col < n; col++){

		val = *ptr;

		if(val != 0){
			*ptr_1++ = val;
			*ptr_2++ = col;
			array_rep->first_empty++;
		}

		ptr++;

	}

}

void array_imp_free(struct _spmat *A){

	Array_Rep *array_rep = (Array_Rep*)(A->private);

	free(array_rep->vaules);
	free(array_rep->colind);
	free(array_rep->rowptr);
	free(array_rep);

}

void array_imp_mult(const struct _spmat *A, const double *v, double *result){

	unsigned int n, start, end;
	register unsigned int i, j;
	register double *ptr = result, *ptr_1, res;
	register int *ptr_2, *ptr_3;
	Array_Rep *array_rep = (Array_Rep*)(A->private);

	n = A->n;
	ptr_3 = (array_rep->rowptr);

	for(i = 0; i < n; i++){

		res = 0;

		start = *ptr_3;
		end = *(ptr_3 + 1);

		ptr_1 = array_rep->vaules + start;
		ptr_2 = array_rep->colind + start;

		for(j = end - start; j != 0; j--){

			res += (*ptr_1) * v[*ptr_2];
			ptr_1++;
			ptr_2++;

		}

		*ptr = res;
		ptr++;
		ptr_3++;

	}

}
