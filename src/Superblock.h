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

    Superblock() {
        this->filesystemCreatedTimestamp = getCurrentTimestamp();
        this->diskSize = BLOCK_SIZE * N_BLOCKS;
        this->nFreeBlocks = N_BLOCKS;
        this->nFiles = 0;
        this->blockSize = BLOCK_SIZE;
    }

    void printInfo() const {
        std::cout << "Filesystem created at " << this->filesystemCreatedTimestamp << std::endl;
        std::cout << "Disk size " << this->diskSize << std::endl;
        std::cout << "Block size " << this->blockSize << std::endl;
        std::cout << "Number of free blocks " << this->nFreeBlocks << std::endl;
        std::cout << "Number of files " << (int) this->nFiles << std::endl;
    }
};


#endif //SOI_FILESYSTEM_SUPERBLOCK_H
