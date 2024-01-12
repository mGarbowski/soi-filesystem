#include <iostream>
#include <bitset>

#define BLOCK_SIZE (32 * 1024) // 32kB
#define N_INODES 256
#define N_BLOCKS 4096  // for total disk capacity of 128MB

struct Superblock {
    int64_t filesystemCreatedTimestamp;
    uint32_t diskSize;
    uint32_t blockSize;
    uint32_t nFreeBlocks;
    uint8_t nFiles;
};

struct FreeBlocksBitmap {
    std::bitset<4096> bitmap;
};

struct INode {
    int64_t createdTimestamp;
    int64_t modifiedTimestamp;
    int64_t accessedTimestamp;
    uint32_t fileSize;
    uint32_t blocks[16];
    uint8_t nLinks;
    uint8_t type;
    bool inUse;
};

struct Block {
    uint8_t data[BLOCK_SIZE];
};

struct VirtualDisk {
    Superblock superblock;
    FreeBlocksBitmap bitmap;
    INode inodes[N_INODES];
    Block blocks[N_BLOCKS];
};

class FileSystem {
private:
    VirtualDisk disk;

    Superblock initializeSuperblock() {

    }
};

int main() {
    std::cout << "Sizes:" << std::endl;
    std::cout << "Superblock " << sizeof(Superblock) << std::endl;
    std::cout << "Bitmap " << sizeof(FreeBlocksBitmap) << std::endl;
    std::cout << "INode " << sizeof(INode) << std::endl;
    std::cout << "Virtual disk " << sizeof(VirtualDisk) << std::endl;

    std::cout << sizeof(bool);
    return 0;
}
