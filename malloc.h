#ifndef MALLOC_H
#define MALLOC_H
typedef unsigned int size_t;
typedef struct AllocatedMemory {
	void* start; // 4 bytes
	size_t capacity; //4 bytes
} AllocatedMemory;

typedef unsigned int size_t;
void init_malloc(void *heap_start, size_t heap_capacity);
void* mmalloc(size_t size);
void free(void *ptr);

#endif
