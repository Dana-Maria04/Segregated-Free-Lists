//Copyright Caruntu Dana-Maria 311CA
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <math.h>
#include <ctype.h>
#include "dll.h"
#include "command.h"
#include "struct.h"

heap_struct *INIT_HEAP(unsigned int address, int n_lists,
					   int bytes_per_list, int tip_r)
{
	heap_struct *heap = malloc(sizeof(heap_struct));
	DIE(!heap, "malloc failed");
	heap->n_lists = n_lists;
	heap->bytes_per_list = bytes_per_list;
	heap->total_memory =  heap->bytes_per_list * heap->n_lists;
	heap->heap_base = address;
	heap->malloc_calls = 0;
	heap->free_calls = 0;
	heap->fragmentations = 0;
	heap->total_allocated_memory = 0;
	heap->total_free_memory = 0;
	heap->num_blocks = 0;
	heap->num_allocated_blocks = 0;
	heap->sfl = malloc(n_lists * sizeof(dll_list *));
	DIE(!heap->sfl, "malloc failed");
	unsigned int base = address;
	for (int i = 0; i < n_lists; i++) {
		long value = (unsigned int)(1 << i) * 8;
		heap->sfl[i] = dll_create(value, bytes_per_list);
		DIE(!heap->sfl[i], "malloc failed");
		heap->sfl[i]->tip_r = tip_r;
		heap->num_blocks += bytes_per_list / value; //numarul de blocuri este
		//cati bytes avem in fiecare lista supra data_sizeul fiecarui bloc
		long size = bytes_per_list / value;
		for (unsigned int j = 0; j < size; j++) {
			dll_add_nth_node(heap->sfl[i], heap->sfl[i]->size,  &base);
			base += value; // Schimb adresa pentru a ma deplasa
			//din value in value
		}
	}
	return heap;
}

allocated_block_info *CREATE_BLOCK(void)
{
	allocated_block_info *new_block = malloc(sizeof(*new_block));
	DIE(!new_block, "malloc failed");
	//stabilesc ca fiind gol , pentru viitoarele MALLOCURI
	new_block->head = NULL;
	new_block->tail = NULL;
	new_block->size = 0;
	new_block->data_size = 0;
	return new_block;
}

void add_for_block(dll_node_t *allocd_node, allocated_block_info *blocks,
				   int n_bytes, heap_struct *heap)
{
	int poz;
	unsigned int address = *((int *)allocd_node->data);
	block_info *current = blocks->head;
	if (!current) {
		add_nth_block(blocks, 0, address, n_bytes);
		heap->num_allocated_blocks++;
	} else {
		poz = 0;
		int ok_break = 0;
		while (current) {
			if (address < current->address) {
				ok_break = 1;
				break;
			}
			poz++;
			current = current->next;
		}
		if (ok_break)
			add_nth_block(blocks, poz, address, n_bytes);
		else
			add_nth_block(blocks, blocks->size, address, n_bytes);
		heap->num_allocated_blocks++;
	}
	free(allocd_node->data);
	free(allocd_node);
}

void fragmentation(heap_struct *heap, int i, int n_bytes,
				   dll_node_t *allocd_node)
{
	int poz = 0;
	heap->fragmentations++;
	heap->num_blocks++;
	int bytes_left = heap->sfl[i]->data_size - n_bytes;
	for (int j = 0; j < heap->n_lists; j++) {
		if (heap->sfl[j]->data_size < (unsigned int)bytes_left)
			continue;
		if (heap->sfl[j]->data_size == (unsigned int)bytes_left) {
			dll_node_t *current = heap->sfl[j]->head;
			poz = 0;
			int address = *((int *)allocd_node->data) + n_bytes;
			int ok_break = 0;
			while (current && current->next) {
				if (address < *(int *)current->data) {
					ok_break = 1;
					break;
				}
				poz++;
				current = current->next;
			}
			if (current && *(int *)current->data < address && ok_break == 0)
				dll_add_nth_node(heap->sfl[j], heap->sfl[j]->size, &address);
			else
				dll_add_nth_node(heap->sfl[j], poz, &address);
			heap->sfl[j]->size++;
		} else { //daca data_size ul este mai mare, trebuie sa fac alta lista
			heap->n_lists++;
			heap->sfl = (dll_list **)realloc(heap->sfl, heap->n_lists *
											 sizeof(dll_list *));
			for (int ii = heap->n_lists - 1; ii > j; ii--)
				heap->sfl[ii] = heap->sfl[ii - 1];
			dll_list *list = (dll_list *)malloc(sizeof(dll_list));
			DIE(!list, "malloc failed");
			list->size = 1; list->data_size = bytes_left;
			list->head = malloc(sizeof(block_info));
			DIE(!list->head, "malloc failed");
			list->head->prev = NULL;
			list->head->data = malloc(sizeof(int));
			DIE(!list->head->data, "malloc failed");
			int actual_address = n_bytes +
								*((int *)allocd_node->data);
			memcpy(list->head->data, &actual_address, sizeof(int));
			list->head->next = NULL;
			heap->sfl[j] = list;
		}
		break;
	}
}

void MALLOC(heap_struct *heap, int n_bytes, allocated_block_info *blocks)
{
	int verif_dim = 0;
	for (int i = 0; i < heap->n_lists; i++) {
		if (n_bytes <= (int)heap->sfl[i]->data_size && heap->sfl[i]->head)
			verif_dim = 1;
	}
	if (!verif_dim) {
		printf("Out of memory\n");//verific daca nu exista blocuri
		//libere cu dimensiune mai mare sau egala cu n_bytes
		return;
	}
	heap->malloc_calls++;//am apelat mallocul
	for (int i = 0; i < heap->n_lists; i++) {
		if (heap->sfl[i]->data_size >=
			(unsigned int)n_bytes && heap->sfl[i]->head) {
			dll_node_t *allocd_node = dll_remove_nth_node(heap->sfl[i], 0);
			--(heap->sfl[i]->size);
			if (heap->sfl[i]->data_size == (unsigned int)n_bytes) {
				dll_list *list = heap->sfl[i];
				if (heap->sfl[i]->size == 0) {
					//daca size-ul este 0, trebuie sa elimin lista
					for (int j = i; j < heap->n_lists - 1; j++)
						heap->sfl[j] = heap->sfl[j + 1];
					dll_free(&list);
					heap->n_lists--;
					heap->sfl = (dll_list **)realloc(heap->sfl, heap->n_lists *
													 sizeof(dll_list *));
				}
			} else {
				fragmentation(heap, i, n_bytes, allocd_node);
			}
			add_for_block(allocd_node, blocks, n_bytes, heap);
			break;
		}
	}
}

block_info *find_index(allocated_block_info *list, unsigned int n)
{
	if (n == 0) {
		block_info *ans = list->head;
		if (!list->head->next)
			list->head = NULL;
		else
			list->head = list->head->next;
		return ans;
	}
	block_info *node = list->head;
	for (unsigned int i = 0; i < n; ++i) {
		if (!node->next) {
			node->prev->next = NULL;
			return node;
		}
		node = node->next;
	}
	if (!node->next) {
		if (node->prev)
			node->prev->next = NULL;
		return node;
	}
	if (node->prev)
		node->prev->next = node->next;
	if (node->next)
		node->next->prev = node->prev;
	return node;
}

unsigned int parsehex(char *str)
{
	//transforma din hex in decimal
	unsigned long ans = 0;
	unsigned int n = strlen(str);
	for (unsigned int i = 0; i < n; i++) {
		unsigned long digit = str[i] - '0';
		if ('a' <= tolower(str[i]) && tolower(str[i]) <= 'f')
			digit = 10 + tolower(str[i]) - 'a';
		ans = ans * 16 + digit;
	}
	return ans;
}

void create_missed_list(heap_struct *heap, int wanted_bytes,
						block_info *freed_block, int *index)
{
	heap->n_lists++;
	heap->sfl = (dll_list **)realloc(heap->sfl,
									 heap->n_lists * sizeof(dll_list *));
	int j;
	for (j = 0; j < heap->n_lists; j++) {
		if (heap->sfl[j]->data_size > (unsigned int)wanted_bytes) {
			for (int ii = heap->n_lists - 1; ii > j; ii--)
				heap->sfl[ii] = heap->sfl[ii - 1];
			dll_list *list = (dll_list *)malloc(sizeof(dll_list));
			DIE(!list, "malloc failed");
			list->size = 1;
			list->data_size = wanted_bytes;
			list->head = malloc(sizeof(block_info));
			DIE(!list->head, "malloc failed");
			list->head->prev = NULL;
			list->head->data = malloc(sizeof(int));
			DIE(!list->head->data, "malloc failed");
			memcpy(list->head->data, &freed_block->address, sizeof(int));
			list->head->next = NULL;
			heap->sfl[j] = list;
			*index = j;
			break;
		}
	}
}

void put_in_list(heap_struct *heap, int index,
				 block_info *freed_block, unsigned int address)
{
	int poz = 0;
	dll_node_t *current = heap->sfl[index]->head;
	if (current && *(unsigned int *)current->data > freed_block->address) {
		dll_add_nth_node(heap->sfl[index], 0, &address);
		heap->sfl[index]->size++;
	} else {
		while (current && current->next) {
			if (*(unsigned int *)current->next->data >
				address && *(unsigned int *)current->data < address)
				break;
			poz++;
			current = current->next;
		}
		dll_add_nth_node(heap->sfl[index], poz + 1, &address);
		heap->sfl[index]->size++;
	}
}

void FREE(heap_struct *heap, unsigned int address,
		  allocated_block_info *blocks)
{
	int index = 0;
	for (block_info *node = blocks->head; node; index++, node = node->next) {
		if (node->address == address) {
			block_info *freed_block;
			freed_block = find_index(blocks, index);
			int wanted_bytes = freed_block->n_bytes;
			--(blocks->size);
			int ok = 0;
			for (int i = 0; i < heap->n_lists; i++) {
				if ((unsigned int)wanted_bytes == heap->sfl[i]->data_size) {
					index = i;
					ok = 1;
					break;
				}
			}
			if (ok == 0)
				create_missed_list(heap, wanted_bytes, freed_block, &index);
			else
				put_in_list(heap, index, freed_block, address);
			blocks->data_size = blocks->data_size - freed_block->n_bytes;
			free(freed_block->input);
			free(freed_block);
			heap->free_calls++;
			return;
		}
	}
	printf("Invalid free\n");
}

void make_ans(heap_struct *heap, allocated_block_info *block)
{
	heap->total_allocated_memory = block->data_size;
	heap->total_free_memory = heap->total_memory -
							  heap->total_allocated_memory;
	heap->num_allocated_blocks = block->size;
}

void DUMP_MEMORY(heap_struct *heap, allocated_block_info *block)
{
	printf("+++++DUMP+++++\n");
	printf("Total memory: %d bytes\n", heap->total_memory);
	make_ans(heap, block);
	printf("Total allocated memory: %d bytes\n", heap->total_allocated_memory);
	printf("Total free memory: %d bytes\n", heap->total_free_memory);
	printf("Free blocks: %d\n", heap->num_blocks - heap->num_allocated_blocks);
	printf("Number of allocated blocks: %d\n", heap->num_allocated_blocks);
	printf("Number of malloc calls: %d\n", heap->malloc_calls);
	printf("Number of fragmentations: %d\n", heap->fragmentations);
	printf("Number of free calls: %d\n", heap->free_calls);
	for (int i = 0; i < heap->n_lists; ++i) {
		int block_size = heap->sfl[i]->data_size;
		int num_free_blocks_current = heap->sfl[i]->size;
		if (num_free_blocks_current) {
			printf("Blocks with %d bytes - %d free block(s) :", block_size,
				   num_free_blocks_current);
			dll_node_t *current = heap->sfl[i]->head;
			while (current) {
				printf(" 0x%x", *((int *)current->data));
				current = current->next;
			}
			printf("\n");
		}
	}
	printf("Allocated blocks :");
	block_info *curr = block->head;
	for (unsigned int i = 0; i < block->size; i++) {
		printf(" (0x%x - %d)", curr->address, curr->n_bytes);
		curr = curr->next;
	}
	printf("\n");
	printf("-----DUMP-----\n");
}

void DESTROY_HEAP(heap_struct *heap, allocated_block_info *block)
{
	if (!heap)
		return;
	for (int i = 0; i < heap->n_lists; i++)
		dll_free(&heap->sfl[i]);
	free(heap->sfl);
	free(heap);
	dll_free_block(&block);
	free(block);
	exit(0);
}

unsigned int find_my_address(allocated_block_info *blocks,
							 unsigned int address)
{
	block_info *current = blocks->head;
	unsigned int poz = 0;
	while (current) {
		if (address == current->address)
			return poz;
		poz++;
		current = current->next;
	}
	return -1;
}

void verify_if_possible(allocated_block_info *blocks,
						unsigned long data_length, int write_bytes,
						unsigned int *final_poz, unsigned int poz,
						unsigned int *bytes, heap_struct *heap)
{
	//verific daca este posibil sa scriu unde am alocat
	if (poz == (unsigned int)blocks->size) {
		printf("Segmentation fault (core dumped)\n");
		DUMP_MEMORY(heap, blocks);
		DESTROY_HEAP(heap, blocks);
		exit(0);
	}
	if ((unsigned int)write_bytes > data_length)
		write_bytes = data_length;
	(*final_poz) = poz;
	block_info *current = blocks->head;
	for (unsigned int i = 0; i < poz; i++)
		current = current->next;
	while (current->next && current->next->address ==
		   (current->address + current->n_bytes) &&
		   (unsigned int)write_bytes > (*bytes)) {
		(*final_poz)++;
		(*bytes) = (*bytes) + current->n_bytes;
		current = current->next;
	}
	if ((unsigned int)write_bytes > (*bytes)) {
		if (current->prev && current->address == current->prev->address +
			current->n_bytes) {
			(*bytes) = (*bytes) + current->n_bytes;
			(*final_poz)++;
			current = current->next;
		}
	}
	if ((*final_poz) == poz)
		(*bytes) = current->n_bytes;
	if ((unsigned int)write_bytes > (*bytes)) {
		printf("Segmentation fault (core dumped)\n");
		DUMP_MEMORY(heap, blocks);
		DESTROY_HEAP(heap, blocks);
		exit(0);
	}
}

void create_string(char *data, block_info **current)
{
	(*current)->input = malloc((*current)->n_bytes + 1);
	char *string = malloc(600 * sizeof(char));
	strncpy(string, data, (*current)->n_bytes);
	memcpy((*current)->input, string, (*current)->n_bytes + 1);
	(*current)->input[(*current)->n_bytes] = '\0';
	strcpy(string, data + (*current)->n_bytes);
	strcpy(data, string);
	free(string);
}

void create_string_final(int write_bytes, block_info **current, char *data)
{
	(*current)->input = malloc(write_bytes + 1);
	memcpy((*current)->input, data, write_bytes + 1);
	((char *)(*current)->input)[write_bytes] = '\0';
}

void WRITE(allocated_block_info *blocks, heap_struct *heap,
		   unsigned int address, char *data, int write_bytes)
{
	unsigned int data_length = strlen(data);
	unsigned int final_poz;
	unsigned int poz = find_my_address(blocks, address);
	if ((int)poz == -1) {
		printf("Segmentation fault (core dumped)\n");
		DUMP_MEMORY(heap, blocks);
		DESTROY_HEAP(heap, blocks);
		exit(0);
	}
	unsigned int bytes = 0;
	if ((int)strlen(data) > write_bytes)
		write_bytes = strlen(data);
	//verificam daca este posibil sa scriem datele
	verify_if_possible(blocks, data_length, write_bytes,
					   &final_poz, poz, &bytes, heap);
	block_info *current = blocks->head;
	for (unsigned int i = 0; i < poz; i++)
		current = current->next;
	if (final_poz == poz) {
		if (current->input) {
			strncpy(current->input, data, strlen(data));
		} else if (!current->input) {
			current->input = malloc(current->n_bytes + 1);
			DIE(!current->input, "malloc");
			memcpy(current->input, data, strlen(data) + 1);
			(current->input)[strlen(data)] = '\0';
		}
		return;
	}
	//parcurgem blockurile de la poziti curenta pana la final
	for (unsigned int i = poz; i < final_poz - 1; current = current->next,
		 i++) {
		if (!current->input) {
			create_string(data, &current);
			data_length = strlen(data);
			bytes = bytes - current->n_bytes;
		} else {
			strncpy(current->input, data, strlen(data));
			char *string = malloc(600 * sizeof(char));
			DIE(!*string, "malloc failed");
			strcpy(string, data + bytes);
			strcpy(data, string);
			free(string);
		}
	}
	if (current) { //verificam daca suntem la finalul blockurilor
		if (!current->input) {
			create_string_final(write_bytes, &current, data);
		} else {
			if (write_bytes > (int)strlen(current->input)) {
				free(current->input);
				current->input = malloc((write_bytes + 1) * sizeof(char));
				DIE(!current->input, "malloc");
				((char *)current->input)[write_bytes] = '\0';
			}
			strncpy((char *)current->input, data, write_bytes);
		}
	}
}

int check_address(allocated_block_info *blocks, unsigned int address)
{
	block_info *curr = blocks->head;
	int index = 0;
	while (curr) {
		if (curr->address == address)
			return index;
		index++;
		curr = curr->next;
	}
	return -1;
}

void final_print(int nr_bytes, block_info *curr)
{
	int bytes_printed = 0;
	char *to_print = malloc(1 + nr_bytes * sizeof(char));
	strcpy(to_print, "");
	while (bytes_printed + strlen(curr->input) < (size_t)nr_bytes) {
		strcat(to_print, curr->input);
		bytes_printed += strlen(curr->input);
		curr = curr->next;
	}
	int bytes_remained_to_print = nr_bytes - bytes_printed;
	strncat(to_print, curr->input, bytes_remained_to_print);
	printf("%s\n", to_print);
	free(to_print);
}

void READ(allocated_block_info *blocks, heap_struct *heap,
		  unsigned int address, int nr_bytes)
{
	if (!heap)
		return;
	//verific mai intai daca am adresa in blocks
	if (check_address(blocks, address) != -1) {
		block_info *curr = blocks->head;
		while (curr) {
			if (curr->address == address)
				break;
			curr = curr->next;
		}
		int bytes_written = 0;
		if (curr->input) {
			bytes_written = strlen(curr->input);
		} else {
			printf("Segmentation fault (core dumped)\n");
			DUMP_MEMORY(heap, blocks);
			DESTROY_HEAP(heap, blocks);
			exit(0);
		}
		//daca numarul de bytes scrisi in blocu current
		//este acelasi cu numarul de bytes ceruti
		if (bytes_written == nr_bytes) {
			char *to_print = malloc(1 + nr_bytes * sizeof(char));
			strncpy(to_print, curr->input, nr_bytes);
			to_print[nr_bytes] = 0;
			printf("%s\n", to_print);
			free(to_print);
			return;
		}
		while (curr && bytes_written < nr_bytes) {
			int addr1 = curr->address;
			curr = curr->next;
			if (!curr) {
				printf("Segmentation fault (core dumped)\n");
				DUMP_MEMORY(heap, blocks);
				DESTROY_HEAP(heap, blocks);
				exit(0);
			}
			int addr2 = curr->address; //verific daca adresele sunt continue
			//in memorie
			if (addr2 != addr1 + curr->prev->n_bytes) {
				printf("Segmentation fault (core dumped)\n");
				DUMP_MEMORY(heap, blocks);
				DESTROY_HEAP(heap, blocks);
				exit(0);
			}
			bytes_written += strlen(curr->prev->input);
		}
		curr = blocks->head;
		while (curr) {
			if (curr->address == address)
				break;
			curr = curr->next;
		}
		final_print(nr_bytes, curr);
		return;
	}
	printf("Segmentation fault (core dumped)\n");
	DUMP_MEMORY(heap, blocks);
	DESTROY_HEAP(heap, blocks);
	exit(0);
}
