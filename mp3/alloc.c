
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

int countFreeList (metadata_t * startofFreeList) {
  int counter = 0;

  while (startofFreeList->next != NULL) {
   // printf("s\n");
    startofFreeList = startofFreeList->next;
    counter++;
  }
  return counter;
}

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
  //printf("in split \n");
  int one = ptr->size - 32 - acquire;
  int two = 32;

  if (one > two) {
    //printf("so one > two\n");
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
    //printf("out of split \n");
    if (newAddress != NULL || newAddress >= sbrk(0)) {
      //printf("not here right?\n");
      coaleseUp(newAddress);
    }
  
    return ptr;
  } else {
    printf("大耳朵 👂👂👂👂👂👂👂👂👂👂👂👂👂👂\n");
    if (ptr->next != NULL) {
      ptr->next->prev = ptr->prev;
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
    //printf("out of split \n");
    return ptr;
  }
}

void *malloc(size_t size) {
  //printf("%d >> New malloc, size is ⚽️ << \n", size);
  if (size == 0) return NULL; 
  void *endOfHeap = sbrk(0);
  metadata_t * copyOfList = startofFreeList;
  metadata_t * tmp = startofFreeList;
  
  if (startofFreeList != NULL) {
    while ( copyOfList != NULL) {  
      //printf("? \n");
      if (copyOfList->isUsed == 0 && copyOfList->size == size) { //改到32
        //copyOfList->isUsed = 0;
        copyOfList->isUsed = 1;
       // printStateMent(size, endOfHeap, startOfHeap);
        if (copyOfList->next != NULL) {
          copyOfList->next->prev = copyOfList->prev;
        }
        if (copyOfList->prev != NULL) {
          copyOfList->prev->next = copyOfList->next;
        }
        if (copyOfList == startofFreeList) { //# if is start;
          //printf("👂 && start \n");
          startofFreeList = copyOfList->next;
        }
        copyOfList->next = NULL;
        copyOfList->prev = NULL;
        return (void*)(copyOfList + 1);
      }

      if (copyOfList->isUsed == 0 && copyOfList->size > size) {
         if (copyOfList->size >= tmp->size && copyOfList->size >= size) {
          tmp = copyOfList;
          //printf("改变了 🐶🐶🐶🐶🐶 \n");
         }
      }

      if (copyOfList->next == NULL) {
        printf("%d copy of list size before break\n", tmp->size);
        break;
      }
      copyOfList = copyOfList->next;
    }
  }
  //printf("should get here now \n");
  if (tmp != NULL && tmp->size > size) {
      //printStateMent(size, endOfHeap, startOfHeap);
      metadata_t * toReturn = split_mem(tmp, size);  
      toReturn->isUsed = 1;
      
      return (void*)(toReturn + 1);
  } 
    
  // ------- when we have to sbrk --------- //

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
  return meta->ptrInMeta;
}

int count = 0;

void free(void *ptr) {

  if (ptr == NULL) {
    return;
  }
  //printf("🆓 is called\n");
  metadata_t *meta = get_addr(ptr);
  meta->isUsed = 0;

  if (startofFreeList == NULL) {
    //printf("suppose be here, start NULL \n");
     startofFreeList = meta;
  } else {
    //printf("start is no null\n");

    meta->next = startofFreeList;
    startofFreeList->prev = meta; 
    startofFreeList = meta;
  }

  // printf("%d \n", startofFreeList->size);

  // if (startofFreeList->next != NULL) {
  //   printf("%d \n", startofFreeList->next->size);
  //   printf("%d \n", startofFreeList->next->isUsed);

  //     if (startofFreeList->next->next != NULL) {
  //     printf("%d \n", startofFreeList->next->next->size);
  //     }
  // }
  

  coalesceDown(meta);
  coaleseUp(meta);

  //printf("%d start size, in free \n -- - -- free  end -- - -\n", startOfHeap->size);
}

void *realloc(void *ptr, size_t size) {
  //printf("%d new size 🏀\n", size);  
  if (ptr == NULL) {
    return malloc(size);
  }
  if (size <= 0) {
    free(ptr);
    return NULL;
  }

  metadata_t * to_realloc = (void *)ptr - sizeof(metadata_t);
  //printf("%d new realloc size 🏀\n", to_realloc->size); 
  if (to_realloc->size == size) {
    return ptr;
  }

 if (size < to_realloc->size) {
  split_mem(to_realloc, size);
  //printf("happen at 265\n");
 // printStateMent(size, sbrk(0), startOfHeap);
  return ptr;
 } else {
   coaleseUp(to_realloc);
   //coalesceDown(to_realloc);

  if (to_realloc->size >= size) {
    //printf("%d happen at 271\n", to_realloc->size);
     //printStateMent(size, sbrk(0), startOfHeap);
    return ptr;
  }

  void* new_ptr = malloc(size);
  if (new_ptr == NULL) {
    // printStateMent(size, sbrk(0), startOfHeap);
    //printf("happen at 276\n");
   return NULL;
  }

  memcpy(new_ptr, ptr, to_realloc->size);
  free(ptr);
  //printf("happen at 281\n");
  //printStateMent(size, sbrk(0), startOfHeap);
  return new_ptr;
  }
} 


void coalesceDown(metadata_t *meta) {

  if (meta == startOfHeap || meta == NULL) {
    //printf("进来看看, meta = start的情况, return \n");
    return;
  }

  metadata_t * theDownOne;
  if (startofFreeList != NULL) {
    theDownOne = startofFreeList;
    while (theDownOne) {
      //printf("%d searching linked list in coalesceDown 🏀\n", countFreeList(startofFreeList)); 
      if ((void*)(theDownOne + 1) + theDownOne->size == meta) {
        break;
      }
      theDownOne = theDownOne->next;
    }
    
    if (theDownOne == NULL) {
      //printf("%p down one cannot search the down one, return\n", startofFreeList->next);
      return;
    }
  }

  // while ( (void*)(theDownOne + 1) + theDownOne->size != meta) { //the one under it.
  //   theDownOne = (void*)(theDownOne + 1) + theDownOne->size;
  //   printf("%d 🔥meta \n", theDownOne->size);
  //   if ((void*)(theDownOne + 1) + theDownOne->size == sbrk(0)) {
  //     return;
  //   }
  //   //printf("%d 🔥the down one is \n", theDownOne->size);
  //   //printf("%d 🔥meta \n", theDownOne->size);
  // }

  if ( meta->isUsed == 0 && theDownOne->isUsed == 0 ) { //take an action
    theDownOne->size = sizeof(metadata_t) + meta->size + theDownOne->size;
    //printf("%d 🔥 down activated \n", theDownOne->size);
  } else {
    //printf("In down: 上面的 不是free， return \n");
    return;
  }
  /* resemble the linked list  */
  if (meta == startofFreeList) { //# if is start;
    startofFreeList = meta->next;
  }
  if (meta->next != NULL) {
    meta->next->prev = meta->prev;
  }
  if (meta->prev != NULL) {
    meta->prev->next = meta->next;
  }
  meta->next = NULL;
  meta->prev = NULL;
  //printf("%d 👀👀👀 down merge resemble done .. 👀👀👀 ·\n", meta->size );
}


void coaleseUp(metadata_t *meta) {
  if (meta == sbrk(0)) {
    return;
  }
  metadata_t * goUp =  (void*)(meta + 1)  + meta->size;
  //printf("%d meta->size in up case \n ", goUp->size);

  if (goUp == sbrk(0) || goUp->isUsed == 1 || goUp == NULL) {
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
    goUp->next->prev = goUp->prev;
  }

  if (goUp->prev != NULL) {
    goUp->prev->next = goUp->next;
  }
  goUp->next = NULL;
  goUp->prev = NULL;

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

