#ifndef SOI_FILESYSTEM_VIRTUALDISK_H
#define SOI_FILESYSTEM_VIRTUALDISK_H

#include <cstdint>
#include "common.h"
#include "Superblock.h"
#include "INode.h"
#include "FreeBlocksBitmap.h"
#include "Directory.h"

struct Block {
    uint8_t data[BLOCK_SIZE];
};


/**
 * Cannot be stored on the stack as blocks by default exceeds stack size
 */
class VirtualDisk {
private:
    Superblock superblock;
    FreeBlocksBitmap bitmap;
    INode inodes[N_INODES]{};
    Block blocks[N_BLOCKS]{};

    VirtualDisk();

    INode rootINode();

    void createRootDirectory();

    static uint32_t numberOfBlocksNeeded(uint32_t fileSize);

    static uint32_t numberOfFullBlocks(uint32_t fileSize);

    static uint32_t numberOfPartialBlocks(uint32_t fileSize);

    Directory loadDirectory(uint32_t blockIdx);

    void saveDirectory(Directory directory, uint32_t blockIdx);

    void saveBytesToBlocks(std::vector<uint32_t> blockIdxs, std::vector<uint8_t> bytes);

    std::vector<uint32_t> allocateBlocks(uint32_t nBlocks);

    /**
     * Insert i-node into the i-nodes array in the first free spot
     * @param iNode i-node to insert
     * @return i-node number
     */
    uint8_t insertINode(INode iNode);

public:

    /**
     * Save new file in the root directory
     * @param filename filename in the virtual filesystem
     * @param bytes binary content of the file
     */
    void saveFile(const std::string &filename, const std::vector<uint8_t> &bytes);

    /**
     * Read content of a file in root directory
     * @param filename
     * @return
     */
    std::vector<uint8_t> readFile(const std::string &filename);

    void saveToFile(const std::string &path);

    static VirtualDisk *initialize();

    static VirtualDisk *loadFromFile(const std::string &path);

};


#endif //SOI_FILESYSTEM_VIRTUALDISK_H
