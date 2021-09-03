#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <assert.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

// defines
#define BYTE_MASK 0xFF
#define MAX_LENGTH 4096

#define BLOBETTE_MAGIC_NUMBER          0x42
// field size for blobs
#define BLOBETTE_MAGIC_NUMBER_BYTES    1
#define BLOBETTE_MODE_LENGTH_BYTES     3
#define BLOBETTE_PATHNAME_LENGTH_BYTES 2
#define BLOBETTE_CONTENT_LENGTH_BYTES  6
#define BLOBETTE_HASH_BYTES            1

// maximum number of bytes in variable-length blobette fields
#define BLOBETTE_MAX_PATHNAME_LENGTH   65535
#define BLOBETTE_MAX_CONTENT_LENGTH    281474976710655



// struct that contains all of the blob data
struct blob {
    uint8_t magic;
    uint32_t mode;
    uint16_t pathname_length;
    uint64_t content_length;
    int *pathname;
    int *content;
    uint8_t hash_given;
    uint8_t hash_cal;
    struct blob *next;
};
// typedef for struct blob
typedef struct blob Blob;


// function declarations

// creates and returns a linked list with all blobettes details in blob file
Blob create(char *pathname);

// prints for case -l 
void case_l(char *pathname);

// extracts files from blob and creates
void case_x(char *pathname);

// create blobs from files pathname[] - case_c
void createBlob(char* blob_path, char *pathname[], int compress_type);

// converts the array for the file name into a usable string
char *convert_array_string(int *array, uint16_t length);

// reverse of convert_array_string; string to int array
int *convert_string_array(char *array);

// extracts a directory
void extract_directory(uint32_t mode, char *pathname);

void add_file(char *pathname_file, char* blob_path);

// creates a directory inside blob
//void create_directory(char *pathname, char* blob_path);

void recurse(char *path_name, char *blob_path);



// copy of blobby hash from blobby.c (no changes made)
uint8_t bhash(uint8_t hash, uint8_t byte);






