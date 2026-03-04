#include "malloc.h"
#include "utils.h"

/*
 * RAM:  [xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx]
 * Heap: [xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx]
 [srt] --> [[header][allocatedmem]] -> [[header][allocatedmem]] --> [[header][allocatedmem]]  
 *
 * Start:
 * 	Contains the total bytes_left
 * 	Contains next_ptr --> points to next header
 * Header: 
 * 	Contains the start of header. We know header size, thus we know start of allocated mem 
 * 	Contains the size of allocated memory
 * 	next_ptr --> contains next header
 * AllocatedMem:
 * 	The memory that the process/thread/whatever we architect here can use
 *
 * Free:
 * 	Traverse the linked list until we find the ptr and then "deallocate it". 
 * 	deallocation --> we will skip that piece of mem. When allocating again we will try the end of the list
 * 		if no space we traverse the list checking the space between two allocations
 * 			if there is space we allocat a new header+alloc 
 * 			else we panic
 * */
typedef struct Header Header;

typedef struct Header {
	Header *next_header_ptr; //4bytes
	size_t alloc_size;   //4bytes
} Header ;

Header *start_heap_ptr = 0;
Header *last_header_ptr = 0;
void *last_allocable_addr = 0;


/* [xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx]
*  [header][header][alloc_mem][header]
 * ^       ^	   ^		  ^
 * str     s->nh   return     h->nh
 * */

void init_malloc(void *heap_start, size_t heap_capacity) {
	usart_print("Heap Starts at: ");
	print_ptr(heap_start);
	last_allocable_addr = heap_start + heap_capacity;
	start_heap_ptr = ((Header*)heap_start);
	Header h = {.next_header_ptr=0, .alloc_size=0};
	*start_heap_ptr = h;
	last_header_ptr = start_heap_ptr;
}

void* mmalloc(size_t size) {
	if ((void*)last_header_ptr + sizeof(Header) + last_header_ptr->alloc_size + sizeof(Header) + size >= last_allocable_addr) {
		usart_print("can't malloc anymore :(\n");
		return 0;
	}

	if (!last_header_ptr->next_header_ptr) { 
		last_header_ptr->next_header_ptr = (void*)last_header_ptr + sizeof(Header) + last_header_ptr->alloc_size;
	}

	Header *new_header_ptr = last_header_ptr->next_header_ptr;
	Header new_header = {.next_header_ptr=0, .alloc_size=size};
	*(new_header_ptr) = new_header;
	
	last_header_ptr = new_header_ptr;

	return (void*)new_header_ptr + sizeof(Header);
}

void free(void *ptr) {
	Header *copy = start_heap_ptr->next_header_ptr;
	while (copy->next_header_ptr && ptr != copy + 1) {
		copy = (void*)copy + sizeof(Header) + copy->alloc_size;
		usart_print("ptr != copy + 1\n");
		print_ptr(ptr);
		print_ptr(copy + 1);
	}
	if (ptr == copy + 1) {
		usart_print("Memory Freed\n");
		// TODO implement the freeing method
	}
}
