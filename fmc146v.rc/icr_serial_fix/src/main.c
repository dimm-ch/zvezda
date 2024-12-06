#include "brdapi.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

uint32_t reverse_bytes(uint32_t bytes);
void write_to_file(FILE *fp, uint32_t num);

int BRDC_main(int argc, char *argv[]) {
  if ((argc > 3) || (argc < 2)) {
    printf("please specify a filename and serial, exiting..\n");
    exit(1);
  }

  size_t filename = (strnlen(argv[2], 257));
  if ((!filename) || (filename == 257)) {
    printf("please use a valid filename\n");
    exit(1);
  }
  // open file
  FILE *fp;
  // open file
  if ((fp = fopen(argv[2], "r+")) == NULL) {
    printf("error while reading the file, aborting..\n");
    exit(1);
  }
  // read serial
  char *pCh;
  uint32_t serial = strtoul(argv[1], &pCh, 10);
  // serial = reverse_bytes(serial);
  printf("reversed is 0X%X\n", serial);
  //  write to file
  write_to_file(fp, serial);
  fclose(fp);
  return 0;
}

uint32_t reverse_bytes(uint32_t bytes) {
  uint32_t aux = 0;
  uint8_t byte;
  int i;

  for (i = 0; i < 32; i += 8) {
    byte = (bytes >> i) & 0xff;
    aux |= byte << (32 - 8 - i);
  }
  return aux;
}

void write_to_file(FILE *fp, uint32_t num) {
  printf("wiriting to file\n");
  fseek(fp, 6, SEEK_SET);
  putc(num & 0xff, fp);
  putc((num >> 8) & 0xff, fp);
  putc((num >> 16) & 0xff, fp);
  putc((num >> 24) & 0xff, fp);
}
