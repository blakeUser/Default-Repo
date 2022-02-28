#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "wallet.h"

/**
 * Initializes an empty wallet.
 */
void wallet_init(wallet_t *wallet) {
  // Implement
  wallet->head = NULL;
  

  pthread_mutex_init(&(wallet->lock), NULL);
  

  //printf("Initialize done \n");
}

/**
 * Returns the amount of a given `resource` in the given `wallet`.
 */
int wallet_get(wallet_t *wallet, const char *resource) {
  //printf("Inside of get \n");
  // Implement this
  wallet_resource * tmp = wallet->head;
  while(tmp != NULL) {
    if (strcmp(tmp->resource_name, resource) == 0) {
      //printf("%d wallet->head->amount before return in get ?\n", wallet->head->amount);
      return tmp->amount;
    }
    //printf("searching ...\n");
    tmp = tmp->next;
  }
  //printf("Didn't get anything, return 0 \n");
  return 0;
}

/**
 * Modifies the amount of a given `resource` in a given `wallet by `delta`.
 * - If `delta` is negative, this function MUST NOT RETURN until the resource can be satisfied.
 *   (Ths function MUST WAIT until the wallet has enough resources to satisfy the request;
 *    there are several ways to accomplish this waiting and it does not have to be fancy.)
 */
void wallet_change_resource(wallet_t *wallet, const char *resource, const int delta) {
  //printf("%p A new Thread comes in ..! 😭 \n", wallet);

  pthread_mutex_lock(&(wallet->lock)); 
  //printf("after lock?\n");
  
  //printf("%p wallet->head\n", wallet->head);
  //printf("%p and wallet is \n", wallet);
  if (wallet->head == NULL) { //if head resource is NULL
    wallet->head = malloc(sizeof(wallet_resource)); //initialize head
    wallet->head->amount = 0;
    wallet->head->next = NULL;
    wallet->head->resource_name = resource;
    pthread_cond_init(&(wallet->head->cond), NULL);
    //printf("%p -- the winnng thread is \n", wallet);
  } 
  wallet_resource * position = wallet->head;

  while (position != NULL) {
    if (strcmp(position->resource_name, resource) != 0) { //strcmp
      position = position->next;
    } else {
      break;
    }
  } //find or not

  if (position == NULL) { //if not find, insert into the linked list
    printf("should not be here \n");
    position = malloc(sizeof(wallet_resource));
    position->amount = 0;
    position->next = wallet->head->next;
    wallet->head->next = position;
    position->resource_name = resource;
    pthread_cond_init(&(position->cond), NULL);
  }

  while (position->amount + delta < 0) {
    pthread_cond_wait(&(position->cond), &(wallet->lock)); //
  }

  /* -- critical section -- */
  position->amount += delta;
  //printf("%d number at position is  \n", position->amount);
  /* -- critical section -- */

  pthread_cond_broadcast(&(position->cond));
  pthread_mutex_unlock(&(wallet->lock));
  
  //printf("unlocked, round finished (hopefullly) \n");
  //printf("%s -- The finish round is  \n", resource);
  //printf("%d -- The position amount is  \n", position->amount);
}

/**
 * Destroys a wallet, freeing all associated memory.
 */
void wallet_destroy(wallet_t *wallet) {
  //pthread_cond_destroy(&cond);
  // Implement this
  wallet_resource * tmp = wallet->head;
  while (tmp != NULL) {

    wallet_resource * anotherTmp = tmp->next;

    //destroy the lock and cond
    pthread_cond_destroy(&(tmp->cond));

    free(tmp);
    
    tmp = anotherTmp;
  }
  pthread_mutex_destroy(&(wallet->lock));
  return;
}