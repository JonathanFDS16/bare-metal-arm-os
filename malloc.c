#include "malloc.h"
#include "utils.h"

typedef struct AllMemory {
	AllocatedMemory **memory;
	void *heap_start;
	void *current_empty;
	size_t bytes_left;
} AllMemory;

AllMemory memories = {0};

void init_malloc(void *heap_start, size_t heap_capacity) {
	usart_print("Heap Starts at: ");
	print_ptr(heap_start);
	memories.heap_start = heap_start;
	memories.current_empty = heap_start + 1024; // Give space for my own heap but it will grow into data :(
	memories.bytes_left = heap_capacity;
}

void* mmalloc(size_t size) {
	if (memories.bytes_left < size) {
		usart_print("No space in heap anymore :(\n");
		return 0;
	}

	AllocatedMemory alloc = {0};
	alloc.start = memories.current_empty;
	alloc.capacity = size;

	// Copy alloc manually to my own heap
	*((AllocatedMemory *)memories.heap_start) = alloc; //Copy alloc starting at heap_start

	usart_print("Alloc Struct is in: ");
	print_ptr(((AllocatedMemory *)memories.heap_start));

	usart_print("Allocated memory will start at:");
	print_ptr(alloc.start);

	//*(memories.memory) = (AllocatedMemory*)memories.heap_start++; //Save pointer of the start AllocatedMemory
	memories.heap_start += sizeof(AllocatedMemory);
	
	// Substract from total memories
	memories.bytes_left -= size;
	memories.current_empty += size; //The currect ptr to an empty space is + size I allocated

	return alloc.start;
}

void free(void *ptr) {
	// frees the memory of the OS
	// go over list of pointers and delete the pointer? 
}
