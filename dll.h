//Copyright Caruntu Dana-Maria 311CA
#ifndef DLL_H
#define DLL_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "command.h"

#define DIE(assertion, call_description)    \
	do {								    \
		if (assertion) {					\
			fprintf(stderr, "(%s, %d): ",	\
					__FILE__, __LINE__);	\
			perror(call_description);		\
			exit(errno);				    \
		}									\
	} while (0)

dll_list * dll_create(unsigned int data_size, int bytes_per_list);
dll_node_t *dll_get_nth_node(dll_list *list, unsigned int n);
void dll_add_nth_node(dll_list *list, int n, const void *data);
void add_nth_block(allocated_block_info *blocks, unsigned int n,
				   unsigned long address, int n_bytes);
dll_node_t *dll_remove_nth_node(dll_list *list, unsigned int n);
unsigned int dll_get_size(dll_list *list);
void dll_free(dll_list **pp_list);
void dll_free_block(allocated_block_info **block);
block_info *dll_remove_nth_block(allocated_block_info *list, unsigned int n);
#endif // DLL_H
