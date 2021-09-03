# Blob-Archiver

4th university project creating a file archiver that creates blobbetes and blobs

### Functionality
- list the contents of a blob, 
- list the permissions of files in a blob, 
- list the size (number of bytes) of files in a blob, 
- check the blobette magic number, 
- extract files from a blob, 
- check blobette integrity (hashes), 
- set the file permissions of files extracted from a blob, 
- create a blob from a list of files, 
- list, extract, and create blobs that include directories, 
- list, extract, and create blobs that are compressed, 

### Usage
```dcc blobby.c help.c help.h -o blobby ``` 
OR
```gcc blobby.c help.c help.h -o blobby ``` 

#### Commands
Given the -l command line argument blobby.c should for each file in the specified blob print:
- The file/directory permissions in octal
- The file/directory size in bytes
- The file/directory pathname

Given the -c command line argument blobby.c should:
- Create a blob containing the specified files.
- Be able to add files in sub-directories. 

Given the -c and the -z command line argument blobby.c should:
- Compress all bytes of the blob using the external program xz.
- The function posix_spawn or posix_spawnp must be used to run xz.
