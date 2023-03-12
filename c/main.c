#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <errno.h>

#define MIN(x, y) x < y ? x : y

const int BUF_SIZE = 1024 * 16;

const char *HEADER = "VPak 1.1";
const int HEADER_SIZE = 8;

FILE *file;
char *header;

FILE *openFile(const char *path, const char *mode)
{
  FILE *f = fopen(path, mode);
  if (!f)
  {
    fprintf(stderr, "Failed to open %s, errno %d (%s)", path, errno, strerror(errno));
    exit(1);
  }
  return f;
}

int usage(char **argv)
{
  printf("Usage: %s pack|unpack [...FILE] OUTFILE\n", argv[0]);
  return 1;
}

void checkedFread(void *ptr, size_t size, size_t n, FILE *fp)
{
  size_t bytesRead = fread(ptr, size, n, fp);
  if (bytesRead != n)
  {
    if (feof(fp))
      fprintf(stderr, "Unexpected EOF while reading %ld elements of size %ld\n", n, size);
    else if (ferror(fp))
      fprintf(stderr, "Error while reading file, errno %d (%s)\n", errno, strerror(errno));
    else
      fprintf(stderr, "Unexpected error while reading file\n");
    exit(1);
  }
}

void checkedFwrite(void *ptr, size_t size, size_t n, FILE *fp)
{
  size_t bytesWritten = fwrite(ptr, size, n, fp);
  if (bytesWritten != n)
  {
    if (feof(fp))
      fprintf(stderr, "Unexpected EOF while writing %ld elements of size %ld\n", n, size);
    else if (ferror(fp))
      fprintf(stderr, "Error while writing file, errno %d (%s)\n", errno, strerror(errno));
    else
      fprintf(stderr, "Unexpected error while writing file\n");
    exit(1);
  }
}

int pack(int argc, char **argv)
{
  if (argc < 4)
  {
    fprintf(stderr, "Usage: %s %s [...FILE] OUTFILE", argv[0], argv[1]);
    return 1;
  }

  file = openFile(argv[argc - 1], "w");

  checkedFwrite(header, sizeof(char), HEADER_SIZE, file);

  for (int i = 2; i < argc - 1; i++)
  {
    char *fileName = argv[i];
    FILE *f = openFile(fileName, "r");

    printf("Packing file %s", fileName);

    uint8_t fileNameSize = strlen(fileName);
    checkedFwrite(&fileNameSize, sizeof(uint8_t), 1, file);
    checkedFwrite(fileName, sizeof(char), fileNameSize, file);

    fseek(f, 0, SEEK_END);
    uint32_t size = ftell(f);
    rewind(f);

    printf(" (%d bytes)\n", size);

    checkedFwrite(&size, sizeof(uint32_t), 1, file);
    char buf[BUF_SIZE];
    size_t n;
    while ((n = fread(buf, sizeof(char), BUF_SIZE, f)) > 0)
    {
      checkedFwrite(buf, sizeof(char), n, file);
    }
  }

  fclose(file);
  free(header);

  return 0;
}

int unpack(int argc, char **argv)
{
  if (argc != 3)
  {
    fprintf(stderr, "Usage: %s %s BUNDLE", argv[0], argv[1]);
    return 1;
  }

  file = openFile(argv[2], "r");

  char headerBuf[HEADER_SIZE];
  if (fread(headerBuf, sizeof(char), HEADER_SIZE, file) != HEADER_SIZE || strcmp(headerBuf, header) != 0)
  {
    fprintf(stderr, "Invalid header. Corrupted or not a VPak file.\n");
    return 1;
  }

  while (1)
  {
    uint8_t fileNameSize;
    if (fread(&fileNameSize, sizeof(uint8_t), 1, file) == 0)
      // Nothing else to read
      break;

    char fileName[fileNameSize + 1];
    // fread does not append NULL so do it ourselves since printf expects NULL
    fileName[fileNameSize] = '\0';
    checkedFread(fileName, sizeof(char), fileNameSize, file);
    printf("File: %s\n", fileName);

    char outFileName[fileNameSize + strlen(".unpacked") + 2];
    strcpy(outFileName, fileName);
    strcat(outFileName, ".unpacked");

    FILE *outFile = openFile(outFileName, "w");

    uint32_t size;
    checkedFread(&size, sizeof(uint32_t), 1, file);
    printf("Size: %d\n", size);

    size_t alreadyRead = 0;
    size_t n = MIN(BUF_SIZE, size);
    char *buf[BUF_SIZE];
    while (n > 0 && (n = fread(buf, sizeof(char), n, file)) > 0)
    {
      alreadyRead += n;
      checkedFwrite(buf, sizeof(char), n, outFile);
      n = MIN(BUF_SIZE, size - alreadyRead);
    }
    if (size != alreadyRead)
    {
      fprintf(stderr, "Unexpected end of input\n");
      return -1;
    }

    printf("Wrote to %s\n", outFileName);
  }

  return 0;
}

int main(int argc, char **argv)
{
  if (argc < 2)
    return usage(argv);

  header = calloc(HEADER_SIZE, 1);
  strcpy(header, HEADER);

  char *op = argv[1];
  if (strcmp(op, "p") == 0 || strcmp(op, "pack") == 0)
    return pack(argc, argv);
  if (strcmp(op, "u") == 0 || strcmp(op, "unpack") == 0)
    return unpack(argc, argv);

  fprintf(stderr, "Invalid operation %s\n", op);
  return usage(argv);
}
