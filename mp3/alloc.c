/*
 * Malloc
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>


typedef struct _metadata_t {
  unsigned int size;     // The size of the memory block.
  unsigned char isUsed;  // 0 if the block is free; 1 if the block is used.
  void * ptrInMeta;
  struct _metadata_t *next;
  struct _metadata_t *prev;
} metadata_t;


static metadata_t *startOfHeap = NULL;
static metadata_t *startofFreeList = NULL;

static size_t requestSize = 0;
static size_t sbrkSize = 0;

metadata_t *get_addr(void *ptr) {
    printf("I was called!");
    metadata_t * meta = ptr - sizeof(metadata_t);
    return meta;
}

/**
 * Allocate space for array in memory
 *
 * Allocates a block of memory for an array of num elements, each of them size
 * bytes long, and initializes all its bits to zero. The effective result is
 * the allocation of an zero-initialized memory block of (num * size) bytes.
 *
 * @param num
 *    Number of elements to be allocated.
 * @param size
 *    Size of elements.
 *
 * @return
 *    A pointer to the memory block allocated by the function.
 *
 *    The type of this pointer is always void*, which can be cast to the
 *    desired type of data pointer in order to be dereferenceable.
 *
 *    If the function failed to allocate the requested block of memory, a
 *    NULL pointer is returned.
 *
 * @see http://www.cplusplus.com/reference/clibrary/cstdlib/calloc/
 */
void *calloc(size_t num, size_t size) {
  void *ptr = malloc(num * size);
  if (!ptr) {
    return NULL;
  }
  //also have to consider overflow
  memset(ptr, 0, num * size);
  return ptr;
}

metadata_t *split_mem(metadata_t *ptr, size_t acquire) {
  //if big enough
  if (ptr->size - sizeof(metadata_t) - acquire >= 100) {
    //modify new Address
    metadata_t * newAddress = ptr + acquire + sizeof(metadata_t);
    newAddress->size = ptr->size - sizeof(metadata_t) - acquire;
    newAddress->isUsed = 0;

    //split and fix the list
    if (ptr->prev == NULL) {
      ptr->next->prev = newAddress;
      newAddress->next = ptr->next;
      newAddress->prev = NULL;
      ptr->next = NULL;
    } else {
      ptr->prev->next = newAddress;
      newAddress->next = ptr->next;
      ptr->next->prev = newAddress;
      newAddress->prev = ptr->prev;
    }
  
    //modify old address
    ptr->isUsed = 1;
    ptr->size = acquire;
    ptr->ptrInMeta = ptr - sizeof(metadata_t);
    printf("有一说一");
    return ptr;
  } else {
    ptr->isUsed = 1;
    ptr->ptrInMeta = ptr - sizeof(metadata_t);
    printf("有👂说");
    return ptr;
  }
}


/**
 * Allocate memory block
 *
 * Allocates a block of size bytes of memory, returning a pointer to the
 * beginning of the block.  The content of the newly allocated block of
 * memory is not initialized, remaining with indeterminate values.
 *
 * @param size
 *    Size of the memory block, in bytes.
 *
 * @return
 *    On success, a pointer to the memory block allocated by the function.
 *
 *    The type of this pointer is always void*, which can be cast to the
 *    desired type of data pointer in order to be dereferenceable.
 *
 *    If the function failed to allocate the requested block of memory,
 *    a null pointer is returned.
 *
 * @see http://www.cplusplus.com/reference/clibrary/cstdlib/malloc/
 */

void printStateMent(size_t size, metadata_t * endOfHeap, metadata_t * startOfHeap) {
  printf("Inside: malloc(%lu):\n", size);
  metadata_t *curMeta = startOfHeap;
  printf("-- Start of Heap (%p) --\n", startOfHeap);
  while ((void *)curMeta < endOfHeap) {   // While we're before the end of the heap...
    printf("metadata for memory %p: (%p, size=%d, isUsed=%d)\n", (void *)curMeta + sizeof(metadata_t), curMeta, curMeta->size, curMeta->isUsed);
    curMeta = (void *)curMeta + curMeta->size + sizeof(metadata_t);
  }
  printf("-- End of Heap (%p) --\n\n", endOfHeap);
}


void *malloc(size_t size) {
  if (size == 0) return NULL; 
  metadata_t *chosenBlock = NULL;
  //metadata_t *copyofHead = startOfHeap;
  metadata_t *curMeta = startOfHeap;
  void *endOfHeap = sbrk(0);


  //if (sbrkSize - requestSize >= size) { //In the middle of malloc and we get empty space
  metadata_t * copyOfList = startofFreeList;
  while ( copyOfList ) {  
       if (copyOfList->isUsed == 0 && copyOfList->size >= size) {
          metadata_t * toReturn = split_mem(copyOfList, size);  
          // requestSize += toReturn->size + sizeof(metadata_t);
          // return toReturn->ptrInMeta;
       }
      copyOfList = copyOfList->next;
    }
  //}

  //metadata_t *meta;
  if (startOfHeap == NULL) {
    startOfHeap = sbrk(0);//startofheap 永远不会变，这里住着的永远是第一个meta
  }

  /* should I use linked list? 
  meta->prev = tail;
  meta->prev->next = meta;
  tail = meta; 
  */

  printStateMent(size, endOfHeap, startOfHeap);
  sbrkSize += size + sizeof(metadata_t);
  requestSize += size + sizeof(metadata_t);

  //if we have to increase the heap
  metadata_t *meta = sbrk( sizeof(metadata_t) );
  meta->size = size;
  meta->isUsed = 1;
  void *ptr = sbrk( size );
  meta->ptrInMeta = ptr;
  return meta->ptrInMeta;
}


/**
 * Deallocate space in memory
 *
 * A block of memory previously allocated using a call to malloc(),
 * calloc() or realloc() is deallocated, making it available again for
 * further allocations.
 *
 * Notice that this function leaves the value of ptr unchanged, hence
 * it still points to the same (now invalid) location, and not to the
 * null pointer.
 *
 * @param ptr
 *    Pointer to a memory block previously allocated with malloc(),
 *    calloc() or realloc() to be deallocated.  If a null pointer is
 *    passed as argument, no action occurs.
 */

void free(void *ptr) {
  metadata_t *meta = get_addr(ptr);
  meta->isUsed = 0;

  if (startofFreeList == NULL) {
     startofFreeList = meta;
  } else {
    meta->next = startofFreeList;
    startofFreeList->prev = meta; 
    startofFreeList = meta;
  }
  //coaleseBlock
}


/**
 * Reallocate memory block
 *
 * The size of the memory block pointed to by the ptr parameter is changed
 * to the size bytes, expanding or reducing the amount of memory available
 * in the block.
 *
 * The function may move the memory block to a new location, in which case
 * the new location is returned. The content of the memory block is preserved
 * up to the lesser of the new and old sizes, even if the block is moved. If
 * the new size is larger, the value of the newly allocated portion is
 * indeterminate.
 *
 * In case that ptr is NULL, the function behaves exactly as malloc, assigning
 * a new block of size bytes and returning a pointer to the beginning of it.
 *
 * In case that the size is 0, the memory previously allocated in ptr is
 * deallocated as if a call to free was made, and a NULL pointer is returned.
 *
 * @param ptr
 *    Pointer to a memory block previously allocated with malloc(), calloc()
 *    or realloc() to be reallocated.
 *
 *    If this is NULL, a new block is allocated and a pointer to it is
 *    returned by the function.
 *
 * @param size
 *    New size for the memory block, in bytes.
 *
 *    If it is 0 and ptr points to an existing block of memory, the memory
 *    block pointed by ptr is deallocated and a NULL pointer is returned.
 *
 * @return
 *    A pointer to the reallocated memory block, which may be either the
 *    same as the ptr argument or a new location.
 *
 * @return
 *    A pointer to the reallocated memory block, which may be either the
 *    same as the ptr argument or a new location.
 *
 * @return
 *    A pointer to the reallocated memory block, which may be either the
 *    same as the ptr argument or a new location.
 *
 * @return
 *    A pointer to the reallocated memory block, which may be either the
 *    same as the ptr argument or a new location.
 *
 *    The type of this pointer is void*, which can be cast to the desired
 *    type of data pointer in order to be dereferenceable.
 *
 *    If the function failed to allocate the requested block of memory,
 *    a NULL pointer is returned, and the memory block pointed to by
 *    argument ptr is left unchanged.
 *
 * @see http://www.cplusplus.com/reference/clibrary/cstdlib/realloc/
 */
void *realloc(void *ptr, size_t size) {

    return NULL;
}


