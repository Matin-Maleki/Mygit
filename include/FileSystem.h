
#ifndef FILESYSTEM_H
#define FILESYSTEM_H

#include <string>
#include <vector>

#include "Directory.h"

class FileSystem
{
private:
    Directory root;
    std::string currentPath;

    // Split a path into its components
    std::vector<std::string> splitPath(
        const std::string &path) const;

    // Find a directory using an absolute path
    Directory *getDirectoryByPath(
        const std::string &path);

    const Directory *getDirectoryByPath(
        const std::string &path) const;

    // Validate file and directory names
    void validateName(const std::string &name) const;

public:
    // Constructor
    FileSystem();

    // Basic commands
    std::string pwd() const;

    void mkdir(const std::string &directoryName);

    void cd(const std::string &address);

    std::vector<std::string> ls() const;

    void touch(const std::string &fileName);

    void echoOverwrite(
        const std::string &text,
        const std::string &fileName);

    void echoAppend(
        const std::string &text,
        const std::string &fileName);

    std::vector<std::string> cat(
        const std::string &fileName) const;

    void rm(const std::string &fileName);

    // Accessors for Repository
    Directory &getRoot();

    const Directory &getRoot() const;

    Directory &getCurrentDirectory();

    const Directory &getCurrentDirectory() const;

    const std::string &getCurrentPath() const;
};

#endif
