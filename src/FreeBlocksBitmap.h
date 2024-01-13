#ifndef SOI_FILESYSTEM_FREEBLOCKSBITMAP_H
#define SOI_FILESYSTEM_FREEBLOCKSBITMAP_H

#include <bitset>
#include <vector>

#include "common.h"

class FreeBlocksBitmap {
private:
    std::bitset<N_BLOCKS> bitmap;

public:
    FreeBlocksBitmap();

    /**
     * Allocate free blocks and mark them as used
     * @param nBlocks number of blocks to allocate
     * @return numbers of allocated blocks
     */
    std::vector<uint32_t> allocateFreeBlocks(uint32_t nBlocks);
};


#endif //SOI_FILESYSTEM_FREEBLOCKSBITMAP_H
