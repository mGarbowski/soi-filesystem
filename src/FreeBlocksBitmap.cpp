#include <stdexcept>
#include "FreeBlocksBitmap.h"

FreeBlocksBitmap::FreeBlocksBitmap() {
    this->bitmap.reset();
}

std::vector<uint32_t> FreeBlocksBitmap::allocateFreeBlocks(uint32_t nBlocks) {
    std::vector<uint32_t> blockNumbers;
    for (size_t idx = 0; idx < bitmap.size(); idx++) {
        if (blockNumbers.size() >= nBlocks) {
            break;
        }

        if (!bitmap.test(idx)) {
            blockNumbers.push_back(idx);
            bitmap.set(idx);
        }
    }

    if (blockNumbers.size() != nBlocks) {
        throw std::runtime_error("Not enough free blocks");
    }

    return blockNumbers;
}
