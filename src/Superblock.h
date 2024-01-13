#ifndef SOI_FILESYSTEM_SUPERBLOCK_H
#define SOI_FILESYSTEM_SUPERBLOCK_H

#include <iostream>
#include "common.h"

struct Superblock {
    int64_t filesystemCreatedTimestamp;
    uint32_t diskSize;
    uint32_t blockSize;
    uint32_t nFreeBlocks;
    uint8_t nFiles;

    Superblock();

    void printInfo() const;
};


#endif //SOI_FILESYSTEM_SUPERBLOCK_H
