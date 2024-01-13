#include <stdexcept>
#include "INode.h"

INode INode::newFile(uint32_t size, std::vector<uint32_t> blockIdxs) {
    if (blockIdxs.size() > INODE_BLOCKS) {
        throw std::invalid_argument("Too many blocks");
    }

    auto iNode = INode::empty();
    auto now = getCurrentTimestamp();

    iNode.createdTimestamp = now;
    iNode.modifiedTimestamp = now;
    iNode.accessedTimestamp = now;
    iNode.fileSize = size;
    iNode.nLinks = 1;
    iNode.type = TYPE_FILE;
    iNode.inUse = true;

    for (size_t idx = 0; idx < blockIdxs.size(); idx++) {
        iNode.blocks[idx] = blockIdxs[idx];
    }

    return iNode;
}

INode INode::empty() {
    INode inode{};
    inode.inUse = false;
    return inode;
}

INode INode::rootINode() {
    auto iNode = INode::empty();
    auto now = getCurrentTimestamp();
    iNode.createdTimestamp = now;
    iNode.modifiedTimestamp = now;
    iNode.accessedTimestamp = now;
    iNode.fileSize = BLOCK_SIZE;
    iNode.nLinks = 1;
    iNode.type = TYPE_DIRECTORY;
    iNode.inUse = true;
    return iNode;
}
