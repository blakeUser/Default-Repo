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

  if (ptr->size - 1 - acquire >= 100) {
    printf("%p  the address of.. newAddress\n", ptr);
    printf("%d start->next在开始 \n", startofFreeList->next->size);

    metadata_t * newAddress = (void*)(ptr + 1) + acquire; //?
    newAddress->size = ptr->size - sizeof(metadata_t) - acquire;
    newAddress->isUsed = 0;
    newAddress->ptrInMeta = newAddress + 1; //?

    printf("//_____-----------------------------__________ \n");
    printf("%d  新的 address size \n", newAddress->size);
    printf("%p  新的 address \n", newAddress);

    printf("%d ptr add \n", ptr->size);
    printf("%d ptr add freelist \n", startofFreeList->size);
    printf("//_____-----------------------------__________ \n");

    ptr->isUsed = 1;
    ptr->size = acquire;
    

    ptr->ptrInMeta = (ptr + 1);//?
    
    //split and fix the list
    //printf("segfault专家😄😄😄😄😄😄😄😄😄\n");


    if (ptr == startofFreeList) { //if we find head
      // printf("%p  current ptr \n",ptr);
      // printf("%p  start of the list \n", startofFreeList);
      // printf("%p ptr->next \n", ptr->next);
      printf("segfault专家😄😄😄😄😄😄😄😄😄\n");
      newAddress->next = startofFreeList->next;
      newAddress->next->prev = newAddress;
      startofFreeList = newAddress;
      //ptr->next = 0;
      //ptr->prev = 0;
      // printf("%p  free list \n",startofFreeList);
      // printf("%p  free list -> next\n",startofFreeList->next);
      // printf("%p  free list -> next -> \n",startofFreeList->next->next);
    } else {
      printf("我在唐山和张佳媚 \n");
      printf("%d  free list in 佳媚 \n",startofFreeList->size);
      printf("%d  free list -> next\n",startofFreeList->next->size);
      printf("%p newAddress \n",newAddress);
      printf("%d  newAddress size \n",newAddress->size);
      //printf("%p  free list -> next\n",startofFreeList->next);
      //printf("%p  free list -> next -> \n",startofFreeList->next->next);
      if (ptr->next == NULL) {
        newAddress->next = NULL;
      } else {
        newAddress->next = ptr->next;
      }
      ptr->prev->next = newAddress;

      if (ptr->next != NULL) {
        ptr->next->prev = newAddress;
      } 
      newAddress->prev = ptr->prev;
    }
    return ptr;
  } else {
    printf("有👂👂👂👂👂👂👂👂👂👂👂👂👂说\n");
    printf("%p myth pointer is \n", ptr);
    ptr->isUsed = 1;
    ptr->ptrInMeta = ptr + sizeof(metadata_t);
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
  //metadata_t *chosenBlock = NULL;
  //metadata_t *curMeta = startOfHeap;
  void *endOfHeap = sbrk(0);

  //if (sbrkSize - requestSize >= size) { //In the middle of malloc and we get empty space
  metadata_t * copyOfList = startofFreeList;

  while ( copyOfList ) {  
       if (copyOfList->isUsed == 0 && copyOfList->size >= size) {
          printf("%d😭😭😭😭😭😭😭😭😭😭😭😭··· \n", size);
          metadata_t * toReturn = split_mem(copyOfList, size);  
          requestSize +=  sizeof(metadata_t);
          //toReturn->isUsed = 1;
          printStateMent(size, endOfHeap, startOfHeap);
          return (void*)(toReturn + 1);
       }
      copyOfList = copyOfList->next;
    }

  //metadata_t *meta;
  if (startOfHeap == NULL) {
    startOfHeap = sbrk(0);//startofheap 永远不会变，这里住着的永远是第一个meta
  }

  sbrkSize +=  size + sizeof(metadata_t);
  requestSize += size + sizeof(metadata_t);
  //if we have to increase the heap
  metadata_t *meta = sbrk( sizeof(metadata_t) );
  meta->size = size;
  meta->isUsed = 1;
  void *ptr = sbrk( size );
  meta->ptrInMeta = ptr;
  printStateMent(size, endOfHeap, startOfHeap);
  return meta->ptrInMeta;
}
int count = 0;
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

  coalesceDown(meta);
  count++;

  if (count % 3 == 0) {
    printf("%d the start in free \n", startofFreeList->size);
    printf("%d same with above \n", startofFreeList->next->prev->size);
  }
  


  // if (startofFreeList->next != NULL) {
  //   printf("%p the size of the start in free->next \n", startofFreeList->next);

  //   metadata_t * nextnext = startofFreeList->next;
  //   if (nextnext->next != NULL) {
  //     printf("%p the size of the start in free->next->next \n", nextnext->next);
  //     printf("%p Expecting 2e0\n", nextnext->prev);
      
  //     if (nextnext->next != NULL) {
  //       metadata_t * innermost = nextnext->next;
  //       printf("%p expecting 120\n", innermost->prev);
  //        printf("%p expecting ori\n", innermost);
  //     }
  //     metadata_t * prevprev = nextnext->prev;

  //     //printf("%p \n", prevprev);
  //     //printf("%p \n", nextnext);
  //   }
  // }


  printf("____-----------________-----_______-----\n");
  
  coaleseUp(meta);
}

void *realloc(void *ptr, size_t size) {

    return NULL;
} 

void coalesceDown(metadata_t *meta) {
  if (meta == startOfHeap) {
    printf("进来看看, meta = start的情况\n");
    return;
  } 

  //let the down one equal to meta
  metadata_t * theDownOne = startOfHeap;
  //theOne = (void*)(theOne + 1) + theOne->size;
  //metadata_t * toReturn = startOfHeap;

  while ( (void*)(theDownOne + 1) + theDownOne->size != meta) { //the one under it.
    theDownOne = (void*)(theDownOne + 1) + theDownOne->size;
  }

  if ( meta->isUsed == 0 && theDownOne->isUsed == 0 ) { //take an action
    theDownOne->size += sizeof(metadata_t) + meta->size;
    //·printf("%d 🌋🌋🌋🌋🌋🌋🌋🌋🌋 求求了应该没问题吧 \n", theDownOne->size);
  } else {
    // printf("不是邻居的情况🌋🌋🌋🌋🌋🌋🌋🌋🌋 \n");
    return;
  }

   
  if (meta == startofFreeList) {
    printf("第一种情况\n");

    meta->next->prev = meta->prev;
    startofFreeList = meta->next;
    meta->next = NULL;
    meta->prev = NULL;

    // printf("%p the address of the down one \n", theDownOne);
    // printf("%p the meta next  \n", meta->next);
    // printf("%d the size of the down one \n", theDownOne->size);
    // printf("%d the stage of the down one \n", theDownOne->isUsed);
    // printf("%p the address of the meta  \n", meta);
  } else if (meta->prev != NULL && meta->next != NULL) {
    printf("第二种情况");
    meta->prev->next = meta->next;
    meta->next->prev = meta->prev;
    meta->next = NULL;
    meta->prev = NULL;
  } else if (meta->next == NULL) {
    printf("第三种情况");
    meta->prev->next = NULL;
    meta->next = NULL;
    meta->prev = NULL;
  }

  // printf("%d the size of the startofFreeList \n", startofFreeList->size);
  // if (startofFreeList->next != NULL) {
  //   printf("%d the size of the startofFreeList->next \n", startofFreeList->next->size);
  // }
  // printf("🎩🎩🎩🎩🎩🎩🎩🎩🎩🎩🎩🎩🎩🎩\n");

  // metadata_t * goDown = toReturn;
  
  // printf("%p size of this \n", theOne->size );
  // if (goDown->isUsed == 0) {
  //   goDown->isUsed = 0;
  //   goDown->size += meta->size + sizeof(metadata_t);
  // }
  
}

void coaleseUp(metadata_t *meta) {
   metadata_t * goUp =  (void*)(meta + 1)  + meta->size;
   printf("%d size of this \n", goUp->size );
  if (goUp == sbrk(0)) {
    return;
  } 
  if (goUp->isUsed == 0) {
    meta->isUsed = 0;
    printStateMent(meta->size, sbrk(0), startOfHeap);
    meta->size += goUp->size + sizeof(metadata_t);
  }
}