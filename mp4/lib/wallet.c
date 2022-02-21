#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "wallet.h"

/**
 * Initializes an empty wallet.
 */
void wallet_init(wallet_t *wallet) {
  // Implement
}



/**
 * Returns the amount of a given `resource` in the given `wallet`.
 */
int wallet_get(wallet_t *wallet, const char *resource) {
  // Implement this
}

/**
 * Modifies the amount of a given `resource` in a given `wallet by `delta`.
 * - If `delta` is negative, this function MUST NOT RETURN until the resource can be satisfied.
 *   (Ths function MUST WAIT until the wallet has enough resources to satisfy the request;
 *    there are several ways to accomplish this waiting and it does not have to be fancy.)
 */
void wallet_change_resource(wallet_t *wallet, const char *resource, const int delta) {
  // Implement this
}

/**
 * Destroys a wallet, freeing all associated memory.
 */
void wallet_destroy(wallet_t *wallet) {
  // Implement this
}