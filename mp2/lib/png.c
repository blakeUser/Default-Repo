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
      //fseek(fstream, 0, SEEK_SET); 
      fwrite(signiture, sizeof(char), sizeof(buffer), fstream);
      //fseek(fstream, 0, SEEK_SET);
    } else {
      return NULL;
    }
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
  int length = 0;
  fread( &length, 4, 1, (png->pointer_key) );   // stored in network order in the file
  length = ntohl(length);  

  // printf("%d, the length is \n", length);
  // printf("%d where am 0.5 \n", ftell(png->pointer_key));
  chunk->len = (length);
  //fseek(png->pointer_key, 4L, SEEK_CUR); //set to type head
  //printf("%d where am 1 \n", ftell(png->pointer_key));

  fgets (chunk->type, 5, png->pointer_key);
  //fseek(png->pointer_key, 4L, SEEK_CUR); //set to data head
  //printf("%d where am 2 \n", ftell(png->pointer_key));

  fseek(png->pointer_key, (length), SEEK_CUR); //set to crc head
  //printf("%d where am 3 \n", ftell(png->pointer_key));

  fseek(png->pointer_key, (4L), SEEK_CUR); //set to crc end
  //printf("%d where am 4 \n", ftell(png->pointer_key));

  //strncpy(chunk->type, pointer_length, 4);
  chunk->type[4] = '\0';

  // size_t toReturn = 4 + 4 + 4 + (length);
  // png->pointer_key = png->pointer_key + toReturn;
  return length + 4 + 4 + 4;
}

/**
 * Writes a PNG chunk to `png`.
 * 
 * Returns the number of bytes written. 
 */
size_t PNG_write(PNG *png, PNG_Chunk *chunk) {
  printf("%d my motherfucker is \n", ftell(png->pointer_key));
  uint32_t toTransfer = chunk->len;

  // fread( &length, 4, 1, (png->pointer_key) );   // stored in network order in the file
  // length = ntohl(length);  
  
  //fwrite(signiture, sizeof(char), sizeof(buffer), fstream);
  fwrite(htonl(toTransfer), sizeof(uint16_t), sizeof(toTransfer), png->pointer_key);

  return 0;
}


/**
 * Frees all memory allocated by this library related to `chunk`.
 */
void PNG_free_chunk(PNG_Chunk *chunk) {
}


/**
 * Closes the PNG file and frees all memory related to `png`.
 */
void PNG_close(PNG *png) {

}