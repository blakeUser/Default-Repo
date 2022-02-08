#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>

#include "crc32.h"
#include "png.h"

const int ERROR_INVALID_PARAMS = 1;
const int ERROR_INVALID_FILE = 2;
const int ERROR_INVALID_CHUNK_DATA = 3;
const int ERROR_NO_UIUC_CHUNK = 4;


/**
 * Opens a PNG file for reading (mode == "r" or mode == "r+") or writing (mode == "w").
 * 
 * (Note: The function follows the same function prototype as `fopen`.)
 * 
 * When the file is opened for reading this function must verify the PNG signature.  When opened for
 * writing, the file should write the PNG signature.
 * 
 * This function must return NULL on any errors; otherwise, return a new PNG struct for use
 * with further fuctions in this library.
 */

PNG * PNG_open(const char * filename, const char * mode) {

  unsigned char signiture[9];
  signiture[0] = '\x89';
  signiture[1] = '\x50';
  signiture[2] = '\x4e';
  signiture[3] = '\x47';
  signiture[4] = '\x0d';
  signiture[5] = '\x0a';
  signiture[6] = '\x1a';
  signiture[7] = '\x0a';
  signiture[8] = '\0';

  PNG * png_toReturn = malloc ( sizeof(PNG) );
  
  unsigned char * buffer = malloc(500);
  FILE *fstream = fopen(filename, mode);

  // ================================= //

    if (fstream == NULL) {
        return NULL;
    }

    long length;
    
    if (fstream) {
      fseek (fstream, 0, SEEK_END);
      length = ftell (fstream);
      fseek (fstream, 0, SEEK_SET);
      buffer = malloc (length);
    }
    if (buffer) {
      fread (buffer, 1, length, fstream);
    }
    buffer[length] = '\0';

    // ================================= //

    unsigned char tmp_Copy[100];
    strncpy(tmp_Copy, buffer, 8);  
    tmp_Copy[8] = '\0'; //好习惯 早养成
    char * pointer = buffer;

    png_toReturn->theString = buffer;
    FILE * tmp = fstream;
    png_toReturn->pointer_key = tmp;

    if (strcmp(tmp_Copy, signiture) == 0) {
      if ( *mode == 'r' ) {
        fseek(tmp, 8, SEEK_SET); 
      }
    } else if ( *mode == 'w' ) {

      fwrite(signiture, 1, 8, fstream);
      ftell(fstream); //This 
      
    } else {
      return NULL;
    }
    free(buffer);
    return png_toReturn;
} 


/**
 * Reads the next PNG chunk from `png`.
 * 
 * If a chunk exists, a the data in the chunk is populated in `chunk` and the
 * number of bytes read (the length of the chunk in the file) is returned.
 * Otherwise, a zero value is returned.
 * 
 * Any memory allocated within `chunk` must be freed in `PNG_free_chunk`.
 * Users of the library must call `PNG_free_chunk` on all returned chunks.
 */
size_t PNG_read(PNG * png, PNG_Chunk * chunk) { //2314 is apparently big endian, but the preivous two are small indian
 
  uint32_t length = 0;

  fread( &length, 1, 4, (png->pointer_key) ); // stored in network order in the file
  
  length = ntohl(length);  
  chunk->len = length;
  fgets (chunk->type, 5, png->pointer_key);

  chunk->data = malloc(chunk->len + 1);

  fread(chunk->data, 1, chunk->len, png->pointer_key);

  chunk->data[chunk->len] = '\0';

  fread(&chunk->crc, 4, 1, png->pointer_key);

  chunk->crc = ntohl(chunk->crc);

  return 4 + 4 + chunk->len + 4;
}

/**
 * Writes a PNG chunk to `png`.
 * 
 * Returns the number of bytes written. 
 */

size_t PNG_write(PNG *png, PNG_Chunk *chunk) {
  //printf("%d first first  \n", ftell(png->pointer_key));

  printf("%d one is one \n", chunk->len);

  //uint32_t theNum = chunk->len;

  // theNum = htonl(theNum);
  // printf("%d two is two \n", theNum);

  unsigned char sz[5];

  sz[0] = chunk->len / 16777216;
  if (sz[0] != 0) {
    chunk->len -= 16777216;
  }
  sz[1] = chunk->len/65536;
  if (sz[1] != 0) {
    chunk->len -= 65536;
  }

  sz[2] = chunk->len / 256;
  if (sz[2] != 0) {
    chunk->len -= 256;
  }
  sz[3] = chunk->len;
  sz[4] = '\0';


  //printf("%d the number should be ... let me think \n", theNum);
  printf("%d the number should be ... let me think \n", chunk->len);
  printf("%d the number 1 should be ... let me think \n", sz[0]);
  printf("%d the number 2 should be ... let me think \n", sz[1]);
  printf("%d the number 3 should be ... let me think \n", sz[2]);
  printf("%d the number 4 should be ... let me think \n", sz[3]);

  fwrite(sz, 1, 4, png->pointer_key);
  fwrite(chunk->type, 1, 4, png->pointer_key);
  fwrite(chunk->data, 1, chunk->len, png->pointer_key);

  char * crc; //为什么在这里能直接assign？又能 char 又能double 又能 int
  crc32(chunk->type, 4, &crc);
  crc32(chunk->data, chunk->len, &crc);
  crc = ntohl(crc);
  fwrite(&crc, 1, 4, png->pointer_key);
  ftell(png->pointer_key);
  return 4 + 4 + chunk->len + 4;
}


/**
 * Frees all memory allocated by this library related to `chunk`.
 */
void PNG_free_chunk(PNG_Chunk *chunk) {
  // if (chunk->data != NULL) {
  //   free(chunk->data);
  //   chunk->data = NULL;
  // }
  // chunk->crc = 0;
  // chunk->len = 0;
}


/**
 * Closes the PNG file and frees all memory related to `png`.
 */
void PNG_close(PNG *png) {

  // if (png->pointer_key != NULL) {
  //     png->pointer_key = NULL;
  // } 

  // if (png->theString != NULL) {
  //   png->theString = NULL;
  // }
}