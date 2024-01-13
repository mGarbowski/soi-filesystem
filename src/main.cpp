#include <iostream>
#include <bitset>
#include <fstream>
#include <cstring>
#include <vector>

#include "common.h"
#include "Superblock.h"
#include "FreeBlocksBitmap.h"
#include "INode.h"



struct Block {
    uint8_t data[BLOCK_SIZE];
};

struct DirectoryEntry {
    uint8_t iNodeNumber;
    char filename[FILENAME_SIZE]{};

    DirectoryEntry() {
        this->iNodeNumber = 0;
    }

    DirectoryEntry(uint8_t iNodeNumber, std::string filename) : iNodeNumber(iNodeNumber) {
        std::strncpy(this->filename, filename.c_str(), FILENAME_SIZE);
        filename[FILENAME_SIZE - 1] = 0;
    }
};

/**
 * A directory cannot take less than a single disk block
 * The maximum number of files in a directory is determined by filename size and disk block size
 */
struct Directory {
    DirectoryEntry entries[DIRECTORY_SIZE]{};

    Directory() = default;

    /**
     * Initialize with . and ..
     */
    Directory(uint8_t selfINodeNumber, uint8_t parentINodeNumber) {
        auto dotdot = DirectoryEntry(parentINodeNumber, "..");
        auto dot = DirectoryEntry(selfINodeNumber, ".");
        this->entries[0] = dotdot;
        this->entries[1] = dot;
    }

    void addEntry(DirectoryEntry newEntry) {
        for (auto &entry: this->entries) {
            if (entry.filename[0] == '\0') {
                entry = newEntry;
                return;
            }
        }

        throw std::runtime_error("Directory full");
    }

    uint8_t getINodeNumber(const std::string &filename) {
        for (auto &entry: this->entries) {
            if (entry.filename == filename) {
                return entry.iNodeNumber;
            }
        }

        throw std::invalid_argument("Not found");
    }
};

/**
 * Cannot be stored on the stack as blocks by default exceeds stack size
 */
struct VirtualDisk {
    Superblock superblock;
    FreeBlocksBitmap bitmap;
    INode inodes[N_INODES]{};
    Block blocks[N_BLOCKS]{};

    VirtualDisk() {
        this->superblock = Superblock();
        this->bitmap = FreeBlocksBitmap();

        for (auto &inode: this->inodes) {
            inode = INode::empty();
        }
    }

    void saveToFile(const std::string &path) {
        std::ofstream file(path);
        file.write(reinterpret_cast<char *>(this), sizeof(VirtualDisk));
        file.close();
    }

    INode rootINode() {
        return this->inodes[0];
    }

    void createRootDirectory() {
        // TODO use common saveFile method
        auto blockIdx = bitmap.allocateFreeBlocks(1)[0];
        auto rootINode = INode::rootINode();
        rootINode.blocks[0] = blockIdx;
        auto rootDirectory = Directory(0, 0);

        std::memcpy(this->blocks[blockIdx].data, &rootDirectory, sizeof(rootDirectory));
        this->inodes[0] = rootINode;
    }

    static VirtualDisk *initialize() {
        auto vd = new VirtualDisk();
        vd->createRootDirectory();
        return vd;
    }

    static VirtualDisk *loadFromFile(const std::string &path) {
        std::ifstream file(path);
        auto disk = new VirtualDisk();
        file.read(reinterpret_cast<char *>(disk), sizeof(VirtualDisk));
        return disk;
    }

    static uint32_t numberOfBlocksNeeded(uint32_t fileSize) {
        return numberOfFullBlocks(fileSize) + numberOfPartialBlocks(fileSize);
    }

    static uint32_t numberOfFullBlocks(uint32_t fileSize) {
        return fileSize / BLOCK_SIZE;
    }

    static uint32_t numberOfPartialBlocks(uint32_t fileSize) {
        return (fileSize % BLOCK_SIZE == 0) ? 0 : 1;
    }

    /**
     * Save new file in the root directory
     * @param filename filename in the virtual filesystem
     * @param bytes binary content of the file
     */
    void saveFile(const std::string &filename, const std::vector<uint8_t> &bytes) {
        if (filename.length() > FILENAME_SIZE) {
            throw std::invalid_argument("Filename too long");
        }
        if (bytes.size() > MAX_FILE_SIZE) {
            throw std::invalid_argument("File too large");
        }

        auto nBlocks = numberOfBlocksNeeded(bytes.size());
        auto blockIdxs = bitmap.allocateFreeBlocks(nBlocks);
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

    Directory loadDirectory(uint32_t blockIdx) {
        Directory dir;
        std::memcpy(&dir, this->blocks[blockIdx].data, sizeof(Directory));
        return dir;
    }

    void saveDirectory(Directory directory, uint32_t blockIdx) {
        std::memcpy(this->blocks[blockIdx].data, &directory, sizeof(Directory));
    }

    /**
     * Read content of a file in root directory
     * @param filename
     * @return
     */
    std::vector<uint8_t> readFile(const std::string &filename) {
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

    void saveBytesToBlocks(std::vector<uint32_t> blockIdxs, std::vector<uint8_t> bytes) {
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

    /**
     * Insert i-node into the i-nodes array in the first free spot
     * @param iNode i-node to insert
     * @return i-node number
     */
    uint8_t insertINode(INode iNode) {
        for (size_t idx = 1; idx < N_INODES; idx++) {
            if (!this->inodes[idx].inUse) {
                this->inodes[idx] = iNode;
                return idx;
            }
        }

        throw std::runtime_error("No free i-nodes");
    }
};


std::vector<uint8_t> readBinaryFile(const std::string &filename) {
    std::ifstream file(filename, std::ios::binary);
    if (!file) {
        throw std::runtime_error("Could not open file " + filename);
    }

    file.seekg(0, std::ios::end);
    size_t size = file.tellg();
    file.seekg(0);

    std::vector<uint8_t> buffer(size);
    file.read(reinterpret_cast<char *>(buffer.data()), size);
    file.close();

    return buffer;
}

void saveBinaryFile(const std::string &outputPath, std::vector<uint8_t> &bytes) {
    std::ofstream outputFile(outputPath, std::ios::binary);
    outputFile.write(reinterpret_cast<char *>(bytes.data()), bytes.size());
    outputFile.close();
}

void printStructSizes() {
    std::cout << "Sizes:" << std::endl;
    std::cout << "Superblock " << sizeof(Superblock) << std::endl;
    std::cout << "Bitmap " << sizeof(FreeBlocksBitmap) << std::endl;
    std::cout << "INode " << sizeof(INode) << std::endl;
    std::cout << "Virtual disk " << sizeof(VirtualDisk) << std::endl;
    std::cout << "Single directory entry " << sizeof(DirectoryEntry) << std::endl;
    std::cout << "Directory " << sizeof(Directory) << " one block: " << BLOCK_SIZE << std::endl;
}


int main() {
    auto imagePath = "/home/mgarbowski/Desktop/image.jpeg";
    auto outImagePath = "/home/mgarbowski/Desktop/image2.jpeg";
    auto virtualDiskPath = "/home/mgarbowski/Desktop/disk.vd";
    auto image = readBinaryFile(imagePath);
    auto disk = VirtualDisk::initialize();
    disk->saveFile("cat.jpeg", image);
    std::cout << "saved cat.jpeg to disk" << std::endl;
    disk->saveToFile(virtualDiskPath);
    std::cout << "saved virtual disk to file" << std::endl;
    delete disk;

    disk = VirtualDisk::loadFromFile(virtualDiskPath);
    std::cout << "Loaded virtual disk from file" << std::endl;
    auto imageFromVirtualDisk = disk->readFile("cat.jpeg");
    saveBinaryFile(outImagePath, imageFromVirtualDisk);
    std::cout << "Loaded data from virtual disk and saved to " << outImagePath << std::endl;

    delete disk;
    return 0;
}
