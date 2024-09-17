//Copyright Caruntu Dana-Maria 311CA
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "dll.h"

#define COMM_SIZE 100

int main(void)
{
	int n_lists, bytes_per_list, tip_r;
	unsigned int n_bytes;
	unsigned long address;
	heap_struct *heap_base = NULL;
	char command[COMM_SIZE];
	scanf("%s", command);
	allocated_block_info *blocks;
	while (1) {
		if (strcmp(command, "INIT_HEAP") == 0) {
			scanf("%lx%d%d%d", &address, &n_lists, &bytes_per_list, &tip_r);
			blocks = CREATE_BLOCK();
			heap_base = INIT_HEAP(address, n_lists, bytes_per_list, tip_r);
		} else if (strcmp(command, "MALLOC") == 0) {
			scanf("%d", &n_bytes);
			MALLOC(heap_base, n_bytes, blocks);
		} else if (strcmp(command, "FREE") == 0) {
			char addr[200];
			scanf("%s", addr);
			address = parsehex(addr + 2);
			FREE(heap_base, address, blocks);
		} else if (strcmp(command, "DUMP_MEMORY") == 0) {
			DUMP_MEMORY(heap_base, blocks);
		} else if (strcmp(command, "WRITE") == 0) {
			char data[520];
			char addr[200];
			scanf("%s", addr);
			address = parsehex(addr + 2);
			char line[520];
			fgets(line, sizeof(line), stdin);
			char *start = strchr(line, '"');
			if (start) {
				char *end = strchr(start + 1, '"');
				if (end && (end - start - 1 < (int)sizeof(data))) {
					strncpy(data, start + 1, end - start - 1);
					data[end - start - 1] = '\0';
					if (end)
						sscanf(end + 1, "%d", &n_bytes);
					else
						exit(EXIT_FAILURE);
				}
			}
			WRITE(blocks, heap_base, address, data, n_bytes);
		} else if (strcmp(command, "READ") == 0) {
			char addr[200];
			scanf("%s", addr);
			address = parsehex(addr + 2);
			scanf("%d", &n_bytes);
			READ(blocks, heap_base, address, n_bytes);
		} else if (strcmp(command, "DESTROY_HEAP") == 0) {
			DESTROY_HEAP(heap_base, blocks);
			break;
		}
		scanf("%s", command);
	}
	return 0;
}
