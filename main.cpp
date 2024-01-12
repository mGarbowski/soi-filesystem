#include <iostream>
#include <bitset>
#include <chrono>
#include <fstream>

#define BLOCK_SIZE (32 * 1024) // 32kB
#define N_INODES 256
#define N_BLOCKS 4096  // for total disk capacity of 128MB

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
    std::bitset<4096> bitmap;

public:
    FreeBlocksBitmap() {
        this->bitmap.reset();
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
};

struct Block {
    uint8_t data[BLOCK_SIZE];
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
}


int main() {
    printStructSizes();

    auto virtualDisk = new VirtualDisk();  // exceeds stack size
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

    delete virtualDisk;
    delete diskFromFile;
    return 0;
}
