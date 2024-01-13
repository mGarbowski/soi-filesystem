#ifndef SOI_FILESYSTEM_INODE_H
#define SOI_FILESYSTEM_INODE_H

#include <vector>
#include <cstdint>
#include "common.h"

struct INode {
    int64_t createdTimestamp;
    int64_t modifiedTimestamp;
    int64_t accessedTimestamp;
    uint32_t fileSize;
    uint32_t blocks[INODE_BLOCKS];
    uint8_t nLinks;
    uint8_t type;
    bool inUse;

    static INode newFile(uint32_t size, std::vector<uint32_t> blockIdxs);

    static INode empty();

    static INode rootINode();
};


#endif //SOI_FILESYSTEM_INODE_H
