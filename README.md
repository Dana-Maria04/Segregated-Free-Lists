# Segregated-Free-Lists

### Description:
This project is designed to function as a Segregated Free Lists, a data structure consisting of an array where each entry stores the starting address of a doubly linked list containing free memory blocks of the same size.
The project includes commands such as:

-> creating the SFL = INIT_HEAP<adr_start_heap><nr_lists><nr_bytes_per_list><reconstruction_type>
-> allocating <nr_bytes> bytes of memory = MALLOC<nr_bytes>
-> marking an allocated block as free = FREE<address>
-> displaying the memory in ASCII format from a given address = READ <address><nr_bytes>
-> writing from the character vector = WRITE <address><data><nr_bytes>
-> displaying the free and allocated areas and their contents = DUMP_MEMORY

Since Segregated Free Lists, as the name suggests, only stores unallocated blocks, for the allocated ones I chose to use a doubly linked list as the data structure.

INIT_HEAP <adr_start_heap><nr_lists><nr_bytes_per_list><reconstruction_type>
To initialize the heap, I used a structure heap_struct that contains the number of lists, the number of bytes per list, and its type.
To get the total memory, I multiplied each list by the number of bytes.
To update the base, I used the variable value, calculated using the formula:

(2 ^ nr_list) * 8
I optimized it with bitwise operations for more efficient and faster execution:

(1 << nr_list) * 8
MALLOC <nr_bytes>
To handle the "Out of memory" error, I traverse all the lists in the Segregated Free Lists to check if there is a free block large enough to accommodate n_bytes.

At the core of the malloc function, there is a counter in the heap_struct structure, which is incremented if memory is available.

During the traversal in malloc, I search for a block of the required size, removing it from the list. If the block is larger than needed, I begin the fragmentation process, adding the remainder back into the segregated lists.

For fragmentation, I reallocate the list array to fit the new dimensions. The formula used is the difference between the total size of the heap and the sum of the sizes of the allocated blocks.

The traversal for finding the appropriate block, adding it to the allocated block list, as well as removing and reordering the lists, each have a time complexity of O(n).

FREE <address>
Similar to the MALLOC function, the FREE function is based on traversing the allocated blocks list to free a previously allocated memory block and reinsert it into the heap's free block lists. Otherwise, an "Invalid free" error is displayed.

To remove the allocated block, I remove the corresponding node from the allocated blocks list and reconnect the links, with a time complexity of O(1).
The previously freed block is reinserted into the SFL's free blocks in the heap as follows: If there is already a list of blocks of the corresponding size, the block is added there. Otherwise, I create a new segregated list with the new size, reallocating the list array and reordering the lists.

READ <address><nr_bytes>
First, to display the "Segmentation fault (core dumped)" message, I considered the following scenarios: if the address is found (for which I used the check_address function), if the memory block contains information (if the input is NULL), and if there is any error in traversing the memory blocks (curr == NULL or addr2 != addr1 + curr->prev->n_bytes).

Along with displaying this message, I performed a memory dump (DUMP_MEMORY), deallocated all resources (DESTROY_HEAP), and forcefully terminated execution (exit(0)).

Second, I traversed the memory blocks until I found the memory block with the desired address. If the byte count for the found block matches the number of bytes in the message, I display it. Otherwise, I continue traversing and store the characters from each memory block for display.

WRITE <address><data><nr_bytes>
First, it calls the verify_if_possible function, which checks if enough bytes are allocated for writing the input and whether the area is contiguous. It uses the pos variable, which is processed and returned by the find_my_address function, calculating the position where the input will be written, with a time complexity of O(n).

To handle Segmentation fault (core dumped) cases, I considered the scenario of exceeding writing limits and the case where the desired address is invalid.

Second, if after traversing I reach the last block and it has an allocated string, I update it and reallocate the necessary space for complete writing. Otherwise, if it doesn't have an allocated string, I call the create_string_final and create_string functions, which allocate the necessary space for writing depending on the situation.

DUMP_MEMORY
Its purpose is to display the heap information, the free memory block information, and the allocated ones. All the information, such as total memory, total allocated memory, the number of freed blocks, the number of malloc and free calls, and others, are stored in the heap_struct, allocated_block_info, and block_info structures.
By traversing, I also displayed the addresses for the free blocks, as well as the allocated ones.

DESTROY_HEAP
Frees all allocated memory and ends the program, calling the dll_free and dll_free_block functions.
