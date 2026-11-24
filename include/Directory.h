
#ifndef DIRECTORY_H
#define DIRECTORY_H

#include <string>
#include <vector>

#include "File.h"

class Directory
{
private:
    std::string name;

    std::vector<File> files;
    std::vector<Directory> directories;

public:
    // Constructor
    Directory(const std::string &directoryName);

    // Getters
    const std::string &getName() const;

    const std::vector<File> &getFiles() const;

    const std::vector<Directory> &getDirectories() const;

    // Find file or directory
    File *findFile(const std::string &fileName);

    const File *findFile(const std::string &fileName) const;

    Directory *findDirectory(const std::string &directoryName);

    const Directory *findDirectory(
        const std::string &directoryName) const;

    // Create file or directory
    File &createFile(const std::string &fileName);

    Directory &createDirectory(
        const std::string &directoryName);

    // Remove file
    void removeFile(const std::string &fileName);
};

#endif
