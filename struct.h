//Copyright Caruntu Dana-Maria 311CA
#ifndef STRUCT_H
#define STRUCT_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdint.h>

typedef struct block_info {
	int n_bytes;//sizeul unui block
	unsigned int address;//adresa de start
	char *input;//mesajul pentru functia de WRITE
	struct block_info *prev, *next;//privesc un bloc ca pe un nod dintr-o lista
} block_info;

//structura alease pentru retinerea blocurilor alocate
typedef struct allocated_block_info {
	struct block_info *head, *tail;
	unsigned int size;//sizeul zonei de blocuri alocate
	unsigned int data_size;//dimensiunnea datelor pe care le stochez intr-un
	//bloc de memorie
} allocated_block_info;

typedef struct dll_node_t {
	void *data; /* Pentru ca datele stocate sa poata avea orice tip, folosim un
				pointer la void. */
	struct dll_node_t *prev, *next;
} dll_node_t;

typedef struct dll_list {
	dll_node_t *head;
	unsigned int data_size;
	unsigned int size;
	int tip_r;
} dll_list;

typedef struct heap_struct {
	unsigned int heap_base;
	int n_lists;
	int malloc_calls;
	int free_calls;
	int bytes_per_list;
	int total_memory;
	int total_allocated_memory;
	int total_free_memory;
	int num_blocks;
	int num_allocated_blocks;
	int fragmentations;
	dll_list **sfl;//vectorul de liste
} heap_struct;

#endif
