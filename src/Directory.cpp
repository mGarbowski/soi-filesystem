#include <stdexcept>
#include <cstring>
#include "Directory.h"

DirectoryEntry::DirectoryEntry() {
    this->iNodeNumber = 0;
}

DirectoryEntry::DirectoryEntry(uint8_t iNodeNumber, std::string filename) : iNodeNumber(iNodeNumber) {
    std::strncpy(this->filename, filename.c_str(), FILENAME_SIZE);
    filename[FILENAME_SIZE - 1] = 0;
}

Directory::Directory(uint8_t selfINodeNumber, uint8_t parentINodeNumber) {
    auto dotdot = DirectoryEntry(parentINodeNumber, "..");
    auto dot = DirectoryEntry(selfINodeNumber, ".");
    this->entries[0] = dotdot;
    this->entries[1] = dot;
}

void Directory::addEntry(DirectoryEntry newEntry) {
    for (auto &entry: this->entries) {
        if (entry.filename[0] == '\0') {
            entry = newEntry;
            return;
        }
    }

    throw std::runtime_error("Directory full");
}

uint8_t Directory::getINodeNumber(const std::string &filename) {
    for (auto &entry: this->entries) {
        if (entry.filename == filename) {
            return entry.iNodeNumber;
        }
    }

    throw std::invalid_argument("Not found");
}
