#include <fstream>
#include <cstring>
#include "VirtualDisk.h"

VirtualDisk::VirtualDisk() {
    this->superblock = Superblock();
    this->bitmap = FreeBlocksBitmap();

    for (auto &inode: this->inodes) {
        inode = INode::empty();
    }
}

void VirtualDisk::saveToFile(const std::string &path) {
    std::ofstream file(path);
    file.write(reinterpret_cast<char *>(this), sizeof(VirtualDisk));
    file.close();
}

INode VirtualDisk::rootINode() {
    return this->inodes[0];
}

void VirtualDisk::createRootDirectory() {
    // TODO use common saveFile method
    auto blockIdx = bitmap.allocateFreeBlocks(1)[0];
    auto rootINode = INode::rootINode();
    rootINode.blocks[0] = blockIdx;
    auto rootDirectory = Directory(0, 0);

    std::memcpy(this->blocks[blockIdx].data, &rootDirectory, sizeof(rootDirectory));
    this->inodes[0] = rootINode;

    this->superblock.nFiles++;
}

VirtualDisk *VirtualDisk::initialize() {
    auto vd = new VirtualDisk();
    vd->createRootDirectory();
    return vd;
}

VirtualDisk *VirtualDisk::loadFromFile(const std::string &path) {
    std::ifstream file(path);
    auto disk = new VirtualDisk();
    file.read(reinterpret_cast<char *>(disk), sizeof(VirtualDisk));
    return disk;
}

uint32_t VirtualDisk::numberOfBlocksNeeded(uint32_t fileSize) {
    return numberOfFullBlocks(fileSize) + numberOfPartialBlocks(fileSize);
}

uint32_t VirtualDisk::numberOfFullBlocks(uint32_t fileSize) {
    return fileSize / BLOCK_SIZE;
}

uint32_t VirtualDisk::numberOfPartialBlocks(uint32_t fileSize) {
    return (fileSize % BLOCK_SIZE == 0) ? 0 : 1;
}

void VirtualDisk::saveFile(const std::string &filename, const std::vector<uint8_t> &bytes) {
    if (filename.length() > FILENAME_SIZE) {
        throw std::invalid_argument("Filename too long");
    }
    if (bytes.size() > MAX_FILE_SIZE) {
        throw std::invalid_argument("File too large");
    }

    this->superblock.nFiles++;

    auto nBlocks = numberOfBlocksNeeded(bytes.size());
    auto blockIdxs = this->allocateBlocks(nBlocks);
    saveBytesToBlocks(blockIdxs, bytes);
    auto iNode = INode::newFile(bytes.size(), blockIdxs);
    auto iNodeNumber = this->insertINode(iNode);

    // add directory entry
    auto blockIdx = this->rootINode().blocks[0];
    auto rootDir = this->loadDirectory(blockIdx);
    auto fileEntry = DirectoryEntry(iNodeNumber, filename);
    rootDir.addEntry(fileEntry);
    this->saveDirectory(rootDir, blockIdx);
}

Directory VirtualDisk::loadDirectory(uint32_t blockIdx) {
    Directory dir;
    std::memcpy(&dir, this->blocks[blockIdx].data, sizeof(Directory));
    return dir;
}

void VirtualDisk::saveDirectory(Directory directory, uint32_t blockIdx) {
    std::memcpy(this->blocks[blockIdx].data, &directory, sizeof(Directory));
}

std::vector<uint8_t> VirtualDisk::readFile(const std::string &filename) {
    auto rootDir = this->loadDirectory(this->rootINode().blocks[0]);
    auto fileINode = this->inodes[rootDir.getINodeNumber(filename)];
    std::vector<uint8_t> bytes(fileINode.fileSize);
    auto nFullBlocks = VirtualDisk::numberOfFullBlocks(fileINode.fileSize);
    auto nPartialBlocks = VirtualDisk::numberOfPartialBlocks(fileINode.fileSize);

    // Read full blocks
    for (uint32_t idx = 0; idx < nFullBlocks; idx++) {
        auto blockIdx = fileINode.blocks[idx];
        auto offset = idx * BLOCK_SIZE;
        auto block = this->blocks[blockIdx].data;
        std::copy(block, block + BLOCK_SIZE, bytes.begin() + offset);
    }

    // Read last partial block (if any)
    if (nPartialBlocks > 0) {
        auto blockIdx = fileINode.blocks[nFullBlocks];
        auto block = this->blocks[blockIdx].data;
        auto offset = nFullBlocks * BLOCK_SIZE;
        auto blockCopySize = fileINode.fileSize - offset;
        std::copy(block, block + blockCopySize, bytes.begin() + offset);
    }

    return bytes;
}

void VirtualDisk::saveBytesToBlocks(std::vector<uint32_t> blockIdxs, std::vector<uint8_t> bytes) {
    auto fileSize = bytes.size();
    auto nFullBlocks = numberOfFullBlocks(fileSize);

    // Copy full blocks
    for (size_t blockNum = 0; blockNum < nFullBlocks; blockNum++) {
        auto blockIdx = blockIdxs[blockNum];
        auto block = this->blocks[blockIdx].data;
        auto offset = blockNum * BLOCK_SIZE;
        std::copy(bytes.begin() + offset, bytes.begin() + offset + BLOCK_SIZE, block);
    }

    // Copy partial block (last one if any)
    if (numberOfPartialBlocks(fileSize) > 0) {
        auto blockIdx = blockIdxs[nFullBlocks];
        auto block = this->blocks[blockIdx].data;
        auto offset = nFullBlocks * BLOCK_SIZE;
        auto sizeToCopy = fileSize - offset;
        std::copy(bytes.begin() + offset, bytes.begin() + offset + sizeToCopy, block);
    }
}

uint8_t VirtualDisk::insertINode(INode iNode) {
    for (size_t idx = 1; idx < N_INODES; idx++) {
        if (!this->inodes[idx].inUse) {
            this->inodes[idx] = iNode;
            return idx;
        }
    }

    throw std::runtime_error("No free i-nodes");
}

std::vector<uint32_t> VirtualDisk::allocateBlocks(uint32_t nBlocks) {
    auto blockIdxs = this->bitmap.allocateFreeBlocks(nBlocks);
    this->superblock.nFreeBlocks -= blockIdxs.size();
    return blockIdxs;
}
