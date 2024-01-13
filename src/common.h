#ifndef SOI_FILESYSTEM_COMMON_H
#define SOI_FILESYSTEM_COMMON_H

#define BLOCK_SIZE (32 * 1024) // 32kB
#define N_INODES 256
#define N_BLOCKS 4096  // for total disk capacity of 128MB
#define DIRECTORY_SIZE 2048  // BLOCK_SIZE / sizeof(DirectoryEntry
#define FILENAME_SIZE 15
#define INODE_BLOCKS 16
#define MAX_FILE_SIZE (INODE_BLOCKS * BLOCK_SIZE)

//TODO enum
#define TYPE_FILE 0
#define TYPE_DIRECTORY 1

int64_t getCurrentTimestamp();


#endif //SOI_FILESYSTEM_COMMON_H
