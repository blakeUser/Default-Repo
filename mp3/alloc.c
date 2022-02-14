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
    metadata_t * meta = ptr - sizeof(metadata_t);
    return meta;
}

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
    //printf("%p  the address of.. newAddress\n", ptr)
    metadata_t * newAddress = (void*)(ptr + 1) + acquire; //?
    newAddress->size = ptr->size - sizeof(metadata_t) - acquire;
    newAddress->isUsed = 0;
    newAddress->ptrInMeta = newAddress + 1; //?

    printf("%p  the address of.. newAddress\n", newAddress->ptrInMeta);
    printf("%p  the address of.. newAddress\n", newAddress);

    ptr->isUsed = 1;
    ptr->size = acquire;
    ptr->ptrInMeta = ptr + sizeof(metadata_t);

    //split and fix the list

    if (ptr->prev == NULL) {
      // printf("我最喜欢娜美\n");
      // printf("%p  current ptr \n",ptr);
      // printf("%p  start of the list \n", startofFreeList);

      newAddress->next = ptr->next;
      newAddress->next->prev = newAddress;
      startofFreeList = newAddress;
      ptr->next = 0;
      //ptr->prev = 0;
      / printf("%p  free list \n",startofFreeList);
      // printf("%p  free list -> next\n",startofFreeList->next);
      // printf("%p  free list -> next -> \n",startofFreeList->next->next);
    } else {
      printf("\n");
      ptr->prev->next = newAddress;
      newAddress->next = ptr->next;
      ptr->next->prev = newAddress;
      newAddress->prev = ptr->prev;
    }
    //modify old address
    return ptr;
  } else {
    ptr->isUsed = 1;
    ptr->ptrInMeta = ptr + sizeof(metadata_t);
    //printf("有👂👂👂👂👂👂👂👂👂👂👂👂👂说");
    return ptr;
  }
}


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

  metadata_t *curMeta = startOfHeap;

  void *endOfHeap = sbrk(0);

  //if (sbrkSize - requestSize >= size) { //In the middle of malloc and we get empty space
  metadata_t * copyOfList = startofFreeList;

  while ( copyOfList ) {  
       if (copyOfList->isUsed == 0 && copyOfList->size >= size) {
          metadata_t * toReturn = split_mem(copyOfList, size);  
          requestSize +=  sizeof(metadata_t);
          toReturn->isUsed = 1;
          return toReturn + sizeof(metadata_t);
       }
      copyOfList = copyOfList->next;
    }

  //metadata_t *meta;
  if (startOfHeap == NULL) {
    startOfHeap = sbrk(0);//startofheap 永远不会变，这里住着的永远是第一个meta
  }

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

void *realloc(void *ptr, size_t size) {

    return NULL;
}
