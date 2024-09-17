//Copyright Caruntu Dana-Maria 311CA
#ifndef COMMAND_H
#define COMMAND_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdint.h>
#include "struct.h"

heap_struct * INIT_HEAP(unsigned int address, int n_lists,
					   int bytes_per_list, int tip_r);
void MALLOC(heap_struct *heap, int n_bytes, allocated_block_info *blocks);
			allocated_block_info *CREATE_BLOCK(void);
void DESTROY_HEAP(heap_struct *heap, allocated_block_info *block);
unsigned int parsehex(char *s);
int is_allocated(allocated_block_info *block, uint64_t address);
void add_for_block(dll_node_t *allocd_node, allocated_block_info *blocks,
				   int n_bytes, heap_struct *heap);
unsigned int find_my_address(allocated_block_info *blocks,
							 unsigned int address);
block_info *find_index(allocated_block_info *list, unsigned int n);
void fragmentation(heap_struct *heap, int i, int n_bytes,
				   dll_node_t *allocd_node);
void verify_if_possible(allocated_block_info *blocks,
						unsigned long data_length, int write_bytes,
						unsigned int *final_poz, unsigned int poz,
						unsigned int *bytes, heap_struct *heap);
void create_missed_list(heap_struct *heap, int wanted_bytes,
						block_info *freed_block, int *index);
void put_in_list(heap_struct *heap, int index,
				 block_info *freed_block, unsigned int address);
void WRITE(allocated_block_info *block, heap_struct *heap,
		   unsigned int address, char *data, int write_bytes);
void READ(allocated_block_info *blocks, heap_struct *heap,
		  unsigned int address, int nr_bytes);
void make_ans(heap_struct *heap, allocated_block_info *block);
void FREE(heap_struct *heap, unsigned int address,
		  allocated_block_info *blocks);
void create_string(char *data, block_info **current);
void create_string_final(int write_bytes, block_info **current, char *data);
int check_address(allocated_block_info *blocks, unsigned int address);
void final_print(int nr_bytes, block_info *curr);
void DUMP_MEMORY(heap_struct *heap, allocated_block_info *block);
#endif // COMMAND_H
