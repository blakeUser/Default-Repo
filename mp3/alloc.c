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
static metadata_t *startofFreeList = '\0';

static size_t requestSize = 0;
static size_t sbrkSize = 0;

metadata_t *get_addr(void *ptr) {
    metadata_t * meta = ptr - sizeof(metadata_t);
    return meta;
}

void *calloc(size_t num, size_t size) {
  //printf("🌞🌞 calloc called 🌞🌞·\n");
  size_t number = num * size;
  void *ptr = malloc(number);
  if (!ptr) {
    return NULL;
  }
  //also have to consider overflow
  memset(ptr, 0, num * size);
  return ptr;
}

metadata_t *split_mem(metadata_t *ptr, size_t acquire) {
  printf("-- In split_mem -- \n");
  if (ptr->size - sizeof(metadata_t) - acquire >= 32) {
    metadata_t * newAddress = (void*)(ptr + 1) + acquire; //?
    newAddress->size = ptr->size - sizeof(metadata_t) - acquire;
    newAddress->isUsed = 0;
    newAddress->ptrInMeta = newAddress + 1; //?
    ptr->isUsed = 1;
    ptr->size = acquire;
    ptr->ptrInMeta = (void*)(ptr + 1);//?

    newAddress->next = ptr->next;
    newAddress->prev = ptr->prev;
    if (newAddress->next != NULL) {
      newAddress->next->prev = newAddress;
    }
    if (newAddress->prev != NULL) {
      newAddress->prev->next = newAddress;
    }
    if (ptr == startofFreeList) {
      startofFreeList = newAddress;
    }
    ptr->next = NULL;
    ptr->prev = NULL;
    return ptr;

  } else {
    printf("有👂👂👂👂👂👂👂👂👂👂👂👂👂说\n");
    if (ptr->next != NULL) {
      ptr->next->prev = ptr->next;
    }
    if (ptr->prev != NULL) {
      ptr->prev->next = ptr->next;
    }
    if (ptr == startofFreeList) { //# if is start;
     printf("👂 && start \n");
      startofFreeList = ptr->next;
    }
    ptr->next = NULL;
    ptr->prev = NULL;
    ptr->isUsed = 1;
    ptr->ptrInMeta = ptr + sizeof(metadata_t);
    return ptr;
  }
}


void *malloc(size_t size) {
  //printf("%d >> New malloc, size is ⚽️ << \n", size);
  if (size == 0) return NULL; 
  void *endOfHeap = sbrk(0);
  //if (sbrkSize - requestSize >= size) { //In the middle of malloc and we get empty space
  metadata_t * copyOfList = startofFreeList;
  metadata_t * tmp = startofFreeList;
  //printf("%p the start of linked list in malloc \n", tmp);
  if (startofFreeList != NULL) {
    while ( copyOfList != NULL) {  
      //printf("%d the size of each linked list at this round ,\n", copyOfList->size);
      //printf("%p tmp add ~~~~~ \n", tmp);
      //printf("%d tmp size ~~~~~ \n", tmp->size);
      //printf("%d copyOfList size ~~~~~ \n", copyOfList->size);

      if (copyOfList->isUsed == 0 && copyOfList->size == size) { //改到32
        //copyOfList->isUsed = 0;
        //printStateMent(size, endOfHeap, startOfHeap);
        copyOfList->isUsed = 1;
        return (void*)(copyOfList + 1);
      }

      if (copyOfList->isUsed == 0 && copyOfList->size > size) {
         if (copyOfList->size >= tmp->size && copyOfList->size >= size) {
          tmp = copyOfList;
          printf("改变了 🐶🐶🐶🐶🐶 \n");
         }
      }

      if (copyOfList->next == NULL) {
        printf("%d copy of list size before break\n", copyOfList->size);
        break;
      }

      copyOfList = copyOfList->next;
    }
  }

  if (tmp != NULL && tmp->size > size) {
      metadata_t * toReturn = split_mem(tmp, size);  
      requestSize +=  sizeof(metadata_t);
      toReturn->isUsed = 1;
      printf("above 💣\n");
      return (void*)(toReturn + 1);
  } 
    
  //------- when we have to sbrk --------------

  if (startOfHeap == NULL) {
    startOfHeap = sbrk(0);//startofheap 永远不会变，这里住着的永远是第一个meta
  }

  //if we have to increase the heap
  metadata_t *meta = sbrk( sizeof(metadata_t) );
  meta->size = size;
  meta->isUsed = 1;
  void *ptr = sbrk( size );
  meta->ptrInMeta = ptr;
  //printStateMent(size, endOfHeap, startOfHeap);
  //printf("😯😯😯😯😯😯😯😯😯😯😯😯😯 so we have to sbrk .. \n");
  return meta->ptrInMeta;
}

int count = 0;

void free(void *ptr) {
  if (ptr == NULL) {
    return;
  }
  //printf("free is called\n");
  metadata_t *meta = get_addr(ptr);
  meta->isUsed = 0;
  if (startofFreeList == NULL) {
     startofFreeList = meta;
  } else {
    meta->next = startofFreeList;
    startofFreeList->prev = meta; 
    startofFreeList = meta;
  }

  coalesceDown(meta);
  //printf("----------between down and up-----\n");
  coaleseUp(meta);
  //printf("%d start size, in free \n -- - -- free  end -- - -\n", startOfHeap->size);
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
  if (ptr == NULL) {
  return malloc(size);
 }
 if (size <= 0) {
  free(ptr);
  return NULL;
 }
 metadata_t * to_realloc = (void *)ptr - sizeof(metadata_t);
  
 if (size <= to_realloc->size) {
  split_mem(to_realloc, size);
  return ptr;
 } else {
  coalesceDown(to_realloc);
    coaleseUp(to_realloc);

  if (to_realloc->size >= size) {
   return ptr;
  }
  void* new_ptr = malloc(size);
  if (new_ptr == NULL) {
   return NULL;
  }
  memcpy(new_ptr, ptr, to_realloc->size);
  free(ptr);
  return new_ptr;
 }
} 


void coalesceDown(metadata_t *meta) {

  if (meta == startOfHeap) {
    //printf("进来看看, meta = start的情况, return \n");
    return;
  }
  metadata_t * theDownOne = startOfHeap;

  while ( (void*)(theDownOne + 1) + theDownOne->size != meta) { //the one under it.
    theDownOne = (void*)(theDownOne + 1) + theDownOne->size;
    //printf("%p 🔥the down one is ", theDownOne);
  }

  if ( meta->isUsed == 0 && theDownOne->isUsed == 0 ) { //take an action
    theDownOne->size = sizeof(metadata_t) + meta->size + theDownOne->size;
     //printf("%d 🔥 down activated \n", theDownOne->size);
  } else {
     //printf("In down: 上面的 不是free， return \n");
    return;
  }
  /* resemble the linked list  */
  if (meta->next != NULL) {
    meta->next->prev = meta->next;
  }
  if (meta->prev != NULL) {
    meta->prev->next = meta->next;
  }
  if (meta == startofFreeList) { //# if is start;
    startofFreeList = meta->next;
  }
  meta->next = NULL;
  meta->prev = NULL;
  //printf("%d 👀👀👀 down merge resemble done .. 👀👀👀 ·\n", meta->size );
}


void coaleseUp(metadata_t *meta) {
  metadata_t * goUp =  (void*)(meta + 1)  + meta->size;
  //printf("%d meta->size in up case \n ", goUp->size);

  if (goUp == sbrk(0) || goUp->isUsed == 1) {
    //printf("进来看 meta == end的情况 ||  is used!, return \n");
    return;
  } 
  if (goUp->isUsed == 0) {
    meta->isUsed = 0;
   // printStateMent(meta->size, sbrk(0), startOfHeap);
    meta->size += goUp->size + sizeof(metadata_t);
  }
  /* Linked list resemble */
  if (goUp == startofFreeList) { //# if is start;
    startofFreeList = goUp->next;
  }
  if (goUp->next != NULL) {
    goUp->next->prev = goUp->next;
  }
  if (goUp->prev != NULL) {
    goUp->prev->next = goUp->next;
  }
  goUp->next = NULL;
  goUp->prev = NULL;
  //printf("%d 👀👀👀 up merge resemble done .. 👀👀👀 ·\n", goUp->size );
}