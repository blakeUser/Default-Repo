#include "http.h"

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <netinet/in.h>
#include <pthread.h>

void *client_thread(void *vptr) {
  int fd = *((int *)vptr);
  HTTPRequest *req = malloc(sizeof(HTTPRequest));
  char * response = malloc(100);
  httprequest_read(req, fd);

  if (strcmp(req->path, "/") != 0 
  && strcmp(req->path, "/getaway.html") != 0 
  && strcmp(req->path, "/240.png") != 0 
  && strcmp(req->path, "/index.html") != 0) { //404
    memcpy(response, req->version, strlen(req->version));
    response = strcat(response, " 404 Not Found");
    response = strcat(response, "\r\n\r\n");    
    write(fd, response, strlen(response));
    return NULL;
  } 
  
  write(fd, req->version, strlen(req->version));
  write(fd, " 200 OK", strlen(" 200 OK"));
  write(fd, "\r\n", strlen("\r\n"));
  attachTheFile(fd, req);
  close(fd);
  return NULL;
}

void attachTheFile(int fd, HTTPRequest *req) {

  FILE* file;

  if (strcmp(req->path, "/240.png") == 0) {
    file = fopen("static/240.png", "r");
  }

  if (strcmp(req->path, "/") == 0) {
    file = fopen("static/index.html", "r");
  }
  
  if (strcmp(req->path, "/getaway.html") == 0) {
    file = fopen("static/getaway.html", "r");
  } 

  fseek(file, 0, SEEK_END);
  long file_len = ftell(file);
  fseek(file, 0, SEEK_SET);
  char* buffer = malloc(file_len);
  fread(buffer, 1, file_len, file);
  buffer[file_len] = '\0';
  char* len[file_len + 1];
  sprintf(len, "%ld", file_len); 

  write(fd, "Content-Type: ", strlen("Content-Type: ")); // type
  if (strcmp(req->path, "/getaway.html") == 0) {
    write(fd, "text/html", strlen("text/html"));
    write(fd, "\r\n", strlen("\r\n"));
  } else if (strcmp(req->path, "/240.png") == 0) {
    write(fd, "image/png", strlen("image/png"));
    write(fd, "\r\n", strlen("\r\n"));
  }

  write(fd, "Content-Length: ", strlen("Content-Length: "));  //len
  write(fd, len, strlen(len));  
  write(fd, "\r\n\r\n", strlen("\r\n\r\n"));

  write(fd, buffer, file_len);
  free(buffer);
  fclose(file);
}


int main(int argc, char *argv[]) {
  if (argc != 2) {
    printf("Usage: %s <port>\n", argv[0]);
    return 1;
  }
  int port = atoi(argv[1]);
  printf("Binding to port %d. Visit http://localhost:%d/ to interact with your server!\n", port, port);

  // socket:
  int sockfd = socket(AF_INET, SOCK_STREAM, 0);

  // bind:
  struct sockaddr_in server_addr, client_address;
  memset(&server_addr, 0x00, sizeof(server_addr));
  server_addr.sin_family = AF_INET;
  server_addr.sin_addr.s_addr = INADDR_ANY;
  server_addr.sin_port = htons(port);  
  bind(sockfd, (const struct sockaddr *)&server_addr, sizeof(server_addr));

  // listen:
  listen(sockfd, 10);

  // accept:
  socklen_t client_addr_len;
  while (1) {
    int *fd = malloc(sizeof(int));
    *fd = accept(sockfd, (struct sockaddr *)&client_address, &client_addr_len);
    printf("Client connected (fd=%d)\n", *fd);

    pthread_t tid;
    pthread_create(&tid, NULL, client_thread, fd);
    pthread_detach(tid);
  }

  return 0;
}