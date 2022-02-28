#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "wallet.h"

/**
 * Initializes an empty wallet.
 */
void wallet_init(wallet_t *wallet) {
  // Implement
  wallet->head = '\0';
  wallet->head = malloc(100); //initialize head
  //printf("%p head address is \n", wallet->head);
  wallet->head->amount = 0;
  wallet->head->next = NULL;
  wallet->head->resource_name = NULL;

  pthread_mutex_init(&(wallet->lock), NULL);
  pthread_cond_init(&(wallet->cond), NULL);

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
    if (tmp->resource_name == resource) {
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

  wallet_resource * position = wallet->head;
  
  //printf("%p wallet->head\n", wallet->head);
  //printf("%p and wallet is \n", wallet);
  if (wallet->head->resource_name == NULL) { //if head resource is NULL
    wallet->head->resource_name = resource;
    //printf("%p -- the winnng thread is \n", wallet);
  } 

  while (position != NULL) {
    if (position->resource_name != resource) {
      position = position->next;
    } else {
      break;
    }
  } //find or not

  if (position == NULL) { //if not find, insert into the linked list
    printf("should not be here \n");
    position = malloc(100);
    position->next = wallet->head->next;
    wallet->head->next = position;
    position->resource_name = resource;
  }

  while (position->amount + delta < 0) {
    pthread_cond_wait(&(wallet->cond), &(wallet->lock)); //everyone comes down or only one? 
  }

  /* -- critical section -- */
  position->amount += delta;
  //printf("%d number at position is  \n", position->amount);
  /* -- critical section -- */

  pthread_cond_broadcast(&(wallet->cond));
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
    free(tmp);
    tmp = anotherTmp;
  }
  return;
}