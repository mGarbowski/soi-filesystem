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

void copyFileToVirtualFilesystem(
        const std::string &nativePath,
        const std::string &virtualPath,
        const std::string &diskPath
) {
    auto disk = VirtualDisk::loadFromFile(diskPath);
    auto fileData = readBinaryFile(nativePath);
    disk->saveFile(virtualPath, fileData);
    disk->saveToFile(diskPath);
    delete disk;
    std::cout << "Saved file " << virtualPath << std::endl;
}

void copyFileFromVirtualFilesystem(
        const std::string &virtualPath,
        const std::string &nativePath,
        const std::string &diskPath
) {
    auto disk = VirtualDisk::loadFromFile(diskPath);
    auto fileData = disk->readFile(virtualPath);
    saveBinaryFile(nativePath, fileData);
    disk->saveToFile(diskPath);
    delete disk;
    std::cout << "Saved file " << nativePath << std::endl;
}


int main(int argc, char *argv[]) {
    std::string virtualDiskPath;
    if (argc < 2) {
        virtualDiskPath = "./disk.vd";
        std::cout << "Path to virtual disk file not supplied, using default disk.vd in current working directory"
                  << std::endl;
    } else {
        virtualDiskPath = argv[1];
    }

    auto disk = VirtualDisk::initialize();
    disk->saveToFile(virtualDiskPath);
    delete disk;
    std::cout << "Created virtual disk file at " << virtualDiskPath << std::endl;

    while (true) {
        std::string command;
        std::cout << "Choose operation:" << std::endl;
        std::cout << "1) Copy file to the virtual filesystem" << std::endl;
        std::cout << "2) Copy file from the virtual filesystem" << std::endl;
        std::cout << "q) Quit" << std::endl;
        std::cout << "[1/2/q] > ";
        std::cin >> command;

        if (command == "q") {
            return 0;
        } else if (command == "1") {
            std::string from;
            std::string to;
            std::cout << "Source path > ";
            std::cin >> from;
            std::cout << "Destination path > ";
            std::cin >> to;

            try {
                copyFileToVirtualFilesystem(from, to, virtualDiskPath);
            } catch (std::exception &e) {
                std::cout << e.what();
            }

        } else if (command == "2") {
            std::string from;
            std::string to;
            std::cout << "Source path > ";
            std::cin >> from;
            std::cout << "Destination path > ";
            std::cin >> to;

            try {
                copyFileFromVirtualFilesystem(from, to, virtualDiskPath);
            } catch (std::exception &e) {
                std::cout << e.what();
            }
        } else {
            std::cout << std::endl << "Unknown command" << std::endl;
        }
    }

    return 0;
}
