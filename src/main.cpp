#include <iostream>
#include <bitset>
#include <fstream>
#include <vector>

#include "common.h"
#include "Superblock.h"
#include "FreeBlocksBitmap.h"
#include "INode.h"
#include "Directory.h"
#include "VirtualDisk.h"


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
