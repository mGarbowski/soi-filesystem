#ifndef SOI_FILESYSTEM_DIRECTORY_H
#define SOI_FILESYSTEM_DIRECTORY_H


#include <cstdint>
#include <string>
#include "common.h"

struct DirectoryEntry {
    uint8_t iNodeNumber;
    char filename[FILENAME_SIZE]{};

    DirectoryEntry();

    DirectoryEntry(uint8_t iNodeNumber, std::string filename);
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
    Directory(uint8_t selfINodeNumber, uint8_t parentINodeNumber);

    void addEntry(DirectoryEntry newEntry);

    uint8_t getINodeNumber(const std::string &filename);
};


#endif //SOI_FILESYSTEM_DIRECTORY_H
