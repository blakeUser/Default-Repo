#include <stdio.h>
#include <stdlib.h>
#include "lib/png.h"
#include <string.h>
#include <errno.h>

int png_extractGIF(const char *png_filename, const char *gif_filename) {

  PNG *png = PNG_open(png_filename, "r");

  if (!png) { return ERROR_INVALID_FILE; }

  FILE * out = fopen(gif_filename, "w");

  printf("PNG Header written.\n");

  //long step = 8;

  while (1) {
    // Read chunk and ensure we get a valid result (exit on error):
    PNG_Chunk chunk;
    
    if (PNG_read(png, &chunk) == 0) {
      // PNG_close(png);
      // PNG_close(out);
      return ERROR_INVALID_CHUNK_DATA;
    }
    if ( strcmp(chunk.type, "uiuc") == 0 ) {
      fwrite(chunk.data, sizeof(char), chunk.len, out);
      fclose(out);
      PNG_close(png);
      PNG_free_chunk(&chunk);
      break;  
    }
    if ( strcmp(chunk.type, "IEND") == 0 ) {
      PNG_free_chunk(&chunk);
      break;  
    }
  }

  return 0;  
}

int main(int argc, char *argv[]) {
  // Ensure the correct number of arguments:
  if (argc != 3) {
    printf("Usage: %s <PNG File> <GIF Name>\n", argv[0]);
    return ERROR_INVALID_PARAMS;
  }

  
  return png_extractGIF(argv[1], argv[2]);
}