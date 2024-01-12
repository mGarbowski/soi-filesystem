#include <iostream>
#include <bitset>
#include <chrono>
#include <fstream>
#include <cstring>
#include <vector>

#define BLOCK_SIZE (32 * 1024) // 32kB
#define N_INODES 256
#define N_BLOCKS 4096  // for total disk capacity of 128MB
#define DIRECTORY_SIZE 2048  // BLOCK_SIZE / sizeof(DirectoryEntry
#define FILENAME_SIZE 15

#define TYPE_FILE 0
#define TYPE_DIRECTORY 0

int64_t getCurrentTimestamp() {
    return std::chrono::system_clock::now().time_since_epoch().count();
}

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
        std::cout << "Number of files " << (int)this->nFiles << std::endl;
    }
};

class FreeBlocksBitmap {
private:
    std::bitset<N_BLOCKS> bitmap;

public:
    FreeBlocksBitmap() {
        this->bitmap.reset();
    }

    /**
     * Allocate free blocks and mark them as used
     * @param nBlocks number of blocks to allocate
     * @return numbers of allocated blocks
     */
    std::vector<uint32_t> allocateFreeBlocks(uint32_t nBlocks) {
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

    static INode empty() {
        INode inode{};
        inode.inUse = false;
        return inode;
    }

    static INode rootINode() {
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
};

struct Block {
    uint8_t data[BLOCK_SIZE];
};

struct DirectoryEntry {
    uint8_t iNodeNumber;
    char filename[FILENAME_SIZE]{};

    DirectoryEntry() {
        this->iNodeNumber=0;
    }

    DirectoryEntry(uint8_t iNodeNumber, std::string filename) : iNodeNumber(iNodeNumber) {
        std::strncpy(this->filename, filename.c_str(), FILENAME_SIZE);
        filename[FILENAME_SIZE-1] = 0;
    }
};

/**
 * A directory cannot take less than a single disk block
 * The maximum number of files in a directory is determined by filename size and disk block size
 */
struct Directory {
    DirectoryEntry entries[DIRECTORY_SIZE];

    /**
     * Initialize with . and ..
     */
    Directory(uint8_t selfINodeNumber, uint8_t parentINodeNumber) {
        auto dotdot = DirectoryEntry(parentINodeNumber, "..");
        auto dot = DirectoryEntry(selfINodeNumber, ".");
        this->entries[0] = dotdot;
        this->entries[1] = dot;
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

    void saveToFile(std::ofstream &file) {
        file.write(reinterpret_cast<char*>(this), sizeof(VirtualDisk));
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

    static VirtualDisk *loadFromFile(std::ifstream &file) {
        auto disk = new VirtualDisk();
        file.read(reinterpret_cast<char*>(disk), sizeof(VirtualDisk));
        return disk;
    }
};


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
    printStructSizes();

    auto virtualDisk = new VirtualDisk();  // exceeds stack size
    virtualDisk->createRootDirectory();
    virtualDisk->superblock.nFiles = 15;
    virtualDisk->superblock.printInfo();

    auto path = "disk.vd";
    std::ofstream outFile(path, std::ios::binary);
    virtualDisk->saveToFile(outFile);
    outFile.close();
    std::cout << "Saved virtual disk to file " << path << std::endl;

    std::ifstream inFile(path, std::ios::binary);
    auto diskFromFile = VirtualDisk::loadFromFile(inFile);
    std::cout << "Loaded virtual disk from file " << path << std::endl;
    diskFromFile->superblock.printInfo();
    std::cout << "root inode valid: " << diskFromFile->inodes[0].inUse << std::endl;
    std::cout << "root block idx: " << diskFromFile->inodes[0].blocks[0] << std::endl;

    delete virtualDisk;
    delete diskFromFile;
    return 0;
}
