#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include "http.h"

/**
 * httprequest_parse_headers
 * 
 * Populate a `req` with the contents of `buffer`, returning the number of bytes used from `buf`.
 */
ssize_t httprequest_parse_headers(HTTPRequest *req, char *buffer, ssize_t buffer_len) {
  req->head = NULL;

  printf("%sbuffer is here, in parse header\n", buffer);
  printf("%dlen was \n", buffer_len);

  size_t counter = 0;

  size_t meth_len = strcspn(buffer, " ");

  counter += meth_len;

  char * tmp1 = malloc(meth_len + 1);

  if (memcmp(buffer, "GET", strlen("GET")) == 0) {
    memcpy(tmp1, buffer, meth_len);
    tmp1[meth_len] = '\0';
    printf("I have hit here\n");
    req->action = tmp1;
  } //Get done

  buffer += meth_len + 1; //the first is /

  size_t url_len = strcspn(buffer, " ");

  char * tmp = malloc(url_len + 1); //const issue

  counter += url_len;

  memcpy(tmp, buffer, url_len);

  tmp[url_len] = '\0';

  req->path = tmp;

  buffer += url_len + 1; // move past <SP>

  size_t ver_len = strcspn(buffer, "\r\n");

  char * version = malloc(ver_len + 1);

  memcpy(version, buffer, ver_len);

  version[ver_len] = '\0';

  req->version = version;

  buffer += ver_len + 2;

  // printf("%d buffer len is \n", strlen(buffer));
  // printf("%d counter len is \n", counter);
  

  while (strlen(buffer) != 0) {

    size_t key_len = strcspn(buffer, ":");

    char * key = malloc(key_len + 1);

    memcpy(key, buffer, key_len);

    key[key_len] = '\0';

    buffer += key_len + 2;
    
    size_t value_len = strcspn(buffer, "\r\n");

    char * value = malloc(value_len + 1);

    memcpy(value, buffer, value_len);

    value[value_len] = '\0';

    //printf("%svalue is \n", value);

    header_t * node = malloc(sizeof(header_t));

    node->key = key;

    node->value = value;

    buffer += value_len + 2; //check whether \r\n later
    
    //printf("%skey is\n", key);
    //printf("%svalue is \n", value);
    //printf("%snext is \n", buffer);

    if (req->head == NULL) {
      printf("😭\n");
      req->head = node;
      req->head->next = NULL;
      // req->head->next->key = NULL;
      // req->head->next->value = NULL;
    } else {
      printf("不😭\n");
      node->next = req->head;
      req->head = node;
    }

    // printf("%shead key is\n", req->head->key);
    // printf("%shead key is\n", req->head->value);

    //printf("%sbuffer at the end is\n", buffer);

    size_t len_myth = strcspn(buffer, "\r\n\r\n");

    printf("%d len_myth \n", len_myth);
    if (len_myth == 0) {
      printf("I want to break! \n");
      break;
    }
    printf("%s inside of the loop \n", buffer);
    // printf("- - - - - -  - - \n");
  }

  buffer += 2;
  //printf("%s the bigbig string is \n", buffer);
  if (strlen(buffer) == 0) {
    //printf("nothing after me, \n");
    req->payload = NULL;
    return buffer_len;
  }

  // printf("len now is %d\n", strlen(buffer));
  // char * payload = malloc(strlen(buffer) + 1);
  // memcpy(payload, buffer, strlen(buffer));
  // payload[strlen(buffer)] = '\0';
  // printf("buffer now is %s\n", (buffer));
  printf("payload now is %s\n", (buffer));
  // req->payload = payload;

  req->payload = buffer;

  return buffer_len;
}

char * global = NULL;
/**
 * httprequest_read
 * 
 * Populate a `req` from the socket `sockfd`, returning the number of bytes read to populate `req`.
 */
ssize_t httprequest_read(HTTPRequest *req, int sockfd) {
  //open(sockfd);

  char * buffer = malloc(5000); //realloc the memeory

  ssize_t returnint = read(sockfd, (void*)buffer, 5000);

  buffer[returnint] = '\0';

  global = buffer;

  // if (theDeleteList == NULL) {
  //   theDeleteList = malloc(1000);
  //   theDeleteList->value = buffer;
  //   theDeleteList->next = NULL;
  // } else {
  //   delete_t * node = malloc(1000);
  //   node->value = buffer;
  //   node->next = theDeleteList;
  //   theDeleteList = node;
  // }

  //free(buffer);

  return httprequest_parse_headers(req, buffer, returnint);
}


/**
 * httprequest_get_action
 * 
 * Returns the HTTP action verb for a given `req`.
 */
const char *httprequest_get_action(HTTPRequest *req) {
  return req->action;
}


/**
 * httprequest_get_header
 * 
 * Returns the value of the HTTP header `key` for a given `req`.
 */
const char *httprequest_get_header(HTTPRequest *req, const char *key) {
  struct header_t_ * headTmp = req->head;

  while (headTmp != NULL) {
    if (strcmp(headTmp->key, key) == 0) {
      return headTmp->value;
    }
    headTmp = headTmp->next;
  }

  return NULL;
}


/**
 * httprequest_get_path
 * 
 * Returns the requested path for a given `req`.
 */
const char *httprequest_get_path(HTTPRequest *req) {

  return req->path;
}


/**
 * httprequest_destroy
 * 
 * Destroys a `req`, freeing all associated memory.
 */
void httprequest_destroy(HTTPRequest *req) {

  if (req->action != NULL) {
    free(req->action);
    req->action = NULL;
  }

  if (req->path != NULL) {
    free(req->path);
    req->path = NULL;
  }
  if (req->version != NULL) {
    free(req->version);
    req->version = NULL;
  }
  // if (req->payload != NULL) {
  //   free(req->payload);
  //   req->payload = NULL;
  // }

  header_t * tmp = req->head;

  while (tmp != NULL) {
    header_t * tmphelper = tmp->next;
    free(tmp->key);
    tmp->key = NULL;
    free(tmp->value);
    tmp->value = NULL;
    free(tmp);
    tmp = tmphelper;
  }

  // delete_t * deleteTMP = theDeleteList;

  // while (deleteTMP != NULL) {
  //   delete_t * deleteTMPhelper = deleteTMP->next;
  //   free(deleteTMP->value);
  //   deleteTMP->value = NULL;
  //   free(deleteTMP);
  //   deleteTMP = deleteTMPhelper;
  // }

  if (global != NULL) {
    free(global);
    global = NULL;
  }
  return;
}