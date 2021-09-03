#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <assert.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>

#include "help.h"

char *convert_array_string(int *array, uint16_t length) {
    char *name = malloc((length) * sizeof(char) + 1);
    name[length] = '\0';
    for(int i = 0; i < length;i++) {
        name[i] = array[i];
    }
    return name;
}

int *convert_string_array(char *array) {
    int *name = malloc(sizeof(int)*strlen(array));
    for(int i = 0; i < strlen(array);i++) {
        name[i] = array[i];
    }
    return name;

}


Blob create(char *pathname) {
    FILE *file = fopen(pathname, "r");
    if(file == NULL) {
        printf("Error: file does not exist\n");
        exit(1);
    }
    Blob *start = malloc(sizeof(Blob));
    Blob *head = start;
    head->next = NULL;
    head->magic = fgetc(file);
    while(head->magic == BLOBETTE_MAGIC_NUMBER && head !=NULL) {
        uint8_t hash = bhash(0, BLOBETTE_MAGIC_NUMBER);
        head->mode = 0;
        // getting mode
        for(int i = 2; i >=0;i--) {
            uint32_t bit = fgetc(file);
            hash = bhash(hash,bit);
            uint32_t tmp = bit << 8*i;
            head->mode |= tmp;
        }
        head->pathname_length = 0;
        // getting pathname length
        for(int i = 1;i >=0;i--) {
            uint16_t bit = fgetc(file);
            hash = bhash(hash,bit);
            uint16_t tmp = bit << 8*i;
            head->pathname_length |= tmp;
        }
        head->content_length = 0;
        // getting content_length
        for(int i = 5;i >=0;i--) {
            uint64_t bit = fgetc(file);
            hash = bhash(hash,bit);
            uint64_t tmp = bit << 8*i;
            head->content_length |= tmp;
        }
        // getting pathname
        int p_length = head->pathname_length;
        int *pathname_local = malloc((p_length)*sizeof(int));
        for(int i = 0;i < p_length ;i++) {
            uint8_t bit = fgetc(file);
            hash = bhash(hash,bit);
            pathname_local[i] = bit;
        }
        head->pathname = pathname_local;
        //getting content
        int c_length = head->content_length;
        int *content_local = malloc((c_length)*sizeof(int));
        for(int i = 0;i < c_length ;i++) {
            uint8_t bit = fgetc(file);
            hash = bhash(hash,bit);
            content_local[i] = bit;
        }
        head->content = content_local;
        head->hash_given = fgetc(file);
        head->hash_cal = hash;
        int last = fgetc(file);
        if(last == EOF) {
            break;
        } else if (last == BLOBETTE_MAGIC_NUMBER) {
            Blob *new = malloc(sizeof(Blob));
            new->next = NULL;
            head->next = new;
            head = head->next;
            head->magic = BLOBETTE_MAGIC_NUMBER;
        }
    }
    if(head->magic !=BLOBETTE_MAGIC_NUMBER) {
        fprintf(stderr, "ERROR: Magic byte of blobette incorrect\n");
        head->pathname = NULL;
        head->content = NULL;

    }
    fclose(file);
    return *start;
}

void case_l(char *pathname) {
    Blob blob = create(pathname);
    Blob *head = &blob;
    while(head !=NULL && head->magic == BLOBETTE_MAGIC_NUMBER) {
        printf("%06o",head->mode);
        if(head->content_length > 99999) {
            printf("%7lu ",head->content_length);
        } else printf("%6lu ",head->content_length);
        for(int i = 0; i < head->pathname_length; i++) {
            printf("%c", head->pathname[i]);
        }
        printf("\n");
        head = head->next;
    }
}

void case_x(char *pathname) {
    Blob blob = create(pathname);
    Blob *head = &blob;
    while (head !=NULL && head->magic == BLOBETTE_MAGIC_NUMBER) {
        // converting the integer array into a string 
        // so it can be used for the fopen call
        char *filename = convert_array_string(head->pathname,head->pathname_length);
        if(S_ISDIR(head->mode)) {
            extract_directory(head->mode,filename);
        } else {
            printf("Extracting: %s\n", filename);
            FILE *new = fopen(filename,"w");
            // inputing all the content in the new file
            for(int i = 0; i < head->content_length; i++) {
                fputc(head->content[i], new);
            }
            // printing to stderr because of the hash error
            if(head->hash_cal != head->hash_given) {
                fprintf(stderr, "ERROR: blob hash incorrect\n");
            }
            // giving file the new mode
            chmod(filename,head->mode);
            fclose(new);
        }
        head = head->next;
    }
}

void createBlob(char* blob_path, char *pathname[], int compress_type) {
    for(int j = 0; pathname[j] !=NULL ; j++){
        char *pathname_from = pathname[j];
        struct stat s;
        stat(pathname_from, &s);
        /*if(S_ISDIR(s.st_mode)) {
            recurse(pathname_from, blob_path);
        } else {
            add_file(pathname[j], blob_path);
        }*/
        add_file(pathname[j], blob_path);
    }
}  

void extract_directory(uint32_t mode, char *pathname) {
    struct stat dir;
    if (stat(pathname, &dir)) {
        printf("Creating directory: %s\n", pathname);
        if(mkdir(pathname, mode)) {
            fprintf(stderr, "Directory cannot be created\n");
            exit(1);
        }
    } else {
        if (S_ISDIR(dir.st_mode)) {
            if (chmod(pathname, mode)) {
                fprintf(stderr, "Directory cannot be created\n");
                exit(1);
            }
        } else {
            fprintf(stderr, "Directory cannot be created\n");
            exit(1);
        }
    }

}

/*void recurse(char *path_name, char *blob_path) {
    char fullpath[MAX_LENGTH +1];
    strcpy(fullpath, path_name);
    size_t len = strlen(fullpath);
    struct stat s;
    stat(path_name, &s);
    DIR *dir = opendir(path_name);
    if (dir == NULL) {
        return;
    }
    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        if (S_ISREG(s.st_mode)) {
            add_file(fullpath, blob_path);
        } else if (S_ISDIR(s.st_mode) &&
                   strcmp(entry->d_name, ".") && strcmp(entry->d_name, "..")) {
            printf("Adding: %s/%s\n", fullpath, entry->d_name);
            add_file(path_name, blob_path);
            fullpath[len] = '/';
            fullpath[len + 1] = '\0';
            strcpy(fullpath + len + 1, entry->d_name);
            recurse(fullpath, blob_path);
        }
    }

    closedir(dir);
}*/

void add_file(char *pathname_file, char* blob_path) {
    struct stat s;
    stat(pathname_file, &s);
    FILE *from = fopen(pathname_file, "r");
    FILE *blob;
    printf("Adding: %s\n", pathname_file);
    if(access(blob_path, F_OK) != -1) {
        FILE *help = fopen(blob_path, "a");
        blob = help;
    } else {
        FILE *help = fopen(blob_path, "w");
        blob = help;
    }
    // moving to the end of the file
    fseek(blob, -1, SEEK_END);
    // inputing hash into the specific blob
    uint8_t hash = bhash(0, BLOBETTE_MAGIC_NUMBER);
    fputc(BLOBETTE_MAGIC_NUMBER,blob);

    // getting the mode and then inputing
    
    uint32_t mode = s.st_mode;
    for(int i = 2;i >=0; i--) {
        uint32_t tmp = mode >> 8*i;
        uint8_t byte = tmp;
        fputc(byte, blob);
        hash = bhash(hash, byte);
    }
    
    // getting the length of pathname
    uint16_t pathname_length = strlen(pathname_file);
    for(int i = 1; i >=0;i--) {
        uint16_t tmp = pathname_length >> 8*i;
        uint8_t byte = tmp;
        fputc(byte, blob);
        hash = bhash(hash, byte);
    }
    // getting the size of the file which is the content length
    uint64_t content_length = s.st_size;
    for(int i = 5; i >=0;i--) {
        uint64_t tmp = content_length >> 8*i;
        uint8_t byte = tmp;
        fputc(byte, blob);
        hash = bhash(hash, byte);
    }
    // inputing the pathname
    for(int i = 0; i < pathname_length; i++) {
        fputc(pathname_file[i],blob); // pathname_from is just the pathname[i]
        hash = bhash(hash, pathname_file[i]);
    }
    // inputing the content of the file 
    for(int i = 0;i < content_length; i++) {
        uint8_t byte = fgetc(from); //
        fputc(byte,blob);
        hash = bhash(hash, byte);
    }
    // inputing the final calulated value for the hash
    fputc(hash, blob);
    fclose(blob);
    fclose(from);
}




// copy of blobby_hash_table from blobby.c
const uint8_t hash_table[256] = {
    241, 18,  181, 164, 92,  237, 100, 216, 183, 107, 2,   12,  43,  246, 90,
    143, 251, 49,  228, 134, 215, 20,  193, 172, 140, 227, 148, 118, 57,  72,
    119, 174, 78,  14,  97,  3,   208, 252, 11,  195, 31,  28,  121, 206, 149,
    23,  83,  154, 223, 109, 89,  10,  178, 243, 42,  194, 221, 131, 212, 94,
    205, 240, 161, 7,   62,  214, 222, 219, 1,   84,  95,  58,  103, 60,  33,
    111, 188, 218, 186, 166, 146, 189, 201, 155, 68,  145, 44,  163, 69,  196,
    115, 231, 61,  157, 165, 213, 139, 112, 173, 191, 142, 88,  106, 250, 8,
    127, 26,  126, 0,   96,  52,  182, 113, 38,  242, 48,  204, 160, 15,  54,
    158, 192, 81,  125, 245, 239, 101, 17,  136, 110, 24,  53,  132, 117, 102,
    153, 226, 4,   203, 199, 16,  249, 211, 167, 55,  255, 254, 116, 122, 13,
    236, 93,  144, 86,  59,  76,  150, 162, 207, 77,  176, 32,  124, 171, 29,
    45,  30,  67,  184, 51,  22,  105, 170, 253, 180, 187, 130, 156, 98,  159,
    220, 40,  133, 135, 114, 147, 75,  73,  210, 21,  129, 39,  138, 91,  41,
    235, 47,  185, 9,   82,  64,  87,  244, 50,  74,  233, 175, 247, 120, 6,
    169, 85,  66,  104, 80,  71,  230, 152, 225, 34,  248, 198, 63,  168, 179,
    141, 137, 5,   19,  79,  232, 128, 202, 46,  70,  37,  209, 217, 123, 27,
    177, 25,  56,  65,  229, 36,  197, 234, 108, 35,  151, 238, 200, 224, 99,
    190
};
// notice see .h file for description
uint8_t bhash(uint8_t hash, uint8_t byte) {
    return hash_table[hash ^ byte];
}



/*int main (int argc, char *argv[]) {
    recurse("cunt/3_files.subdirectory.d", "help_me.blob");
    case_x("help_me.blob");
}*/




// testing function for create
/*void text_create(char *pathname) {
    Blob head = create(pathname);
    Blob *list = &head;
    while(list !=NULL) {
    printf("0x%x\n",list->magic);
    printf("%06o\n", list->mode);
    printf("%d\n", list->pathname_length);
    printf("%lu\n", list->content_length);
    for(int i = 0;i < list->pathname_length; i++) {
        printf("%c", list->pathname[i]);
    } 
    printf("\n");
    for(int i = 0; i<list->content_length;i++) {
        printf("%c", list->content[i]);
    }
    printf("\n%d \n",list->hash);
    list = list->next;
    }
} 
char *directory_name(char *pathname) {
    int length = strlen(pathname);
    char *directory = malloc(sizeof(char) * length);
    for(int i = 0; pathname[i] != '/'; i++) {
        directory[i] = pathname[i];
        directory[i+1] = '\0';
    }
    length = strlen(directory) +1;
    char *new_directory = malloc(sizeof(char) *length);
    for(int i = 0; directory[i] != '\0'; i++) {
        new_directory[i] = directory[i];
        new_directory[i + 1] = '\0';
    }
    return new_directory;
} */