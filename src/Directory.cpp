
#include "Directory.h"

// Constructor
Directory::Directory(const std::string &directoryName)
{
    name = directoryName;
}

// Getters
const std::string &Directory::getName() const
{
    return name;
}

const std::vector<File> &Directory::getFiles() const
{
    return files;
}

const std::vector<Directory> &Directory::getDirectories() const
{
    return directories;
}

// Find file
File *Directory::findFile(const std::string &fileName)
{
    for (size_t i = 0; i < files.size(); i++)
    {
        if (files[i].getName() == fileName)
        {
            return &files[i];
        }
    }

    return nullptr;
}

// Find file (const)
const File *Directory::findFile(
    const std::string &fileName) const
{
    for (size_t i = 0; i < files.size(); i++)
    {
        if (files[i].getName() == fileName)
        {
            return &files[i];
        }
    }

    return nullptr;
}

// Find directory
Directory *Directory::findDirectory(
    const std::string &directoryName)
{
    for (size_t i = 0; i < directories.size(); i++)
    {
        if (directories[i].getName() == directoryName)
        {
            return &directories[i];
        }
    }

    return nullptr;
}

// Find directory (const)
const Directory *Directory::findDirectory(
    const std::string &directoryName) const
{
    for (size_t i = 0; i < directories.size(); i++)
    {
        if (directories[i].getName() == directoryName)
        {
            return &directories[i];
        }
    }

    return nullptr;
}

// Create file
File &Directory::createFile(const std::string &fileName)
{
    if (findDirectory(fileName) != nullptr)
    {
        throw std::string("A directory with this name already exists.");
    }

    File *existingFile = findFile(fileName);

    if (existingFile != nullptr)
    {
        return *existingFile;
    }

    files.push_back(File(fileName));

    return files.back();
}

// Create directory
Directory &Directory::createDirectory(
    const std::string &directoryName)
{
    if (findFile(directoryName) != nullptr ||
        findDirectory(directoryName) != nullptr)
    {
        throw std::string("An entry with this name already exists.");
    }

    directories.push_back(Directory(directoryName));

    return directories.back();
}

// Remove file
void Directory::removeFile(const std::string &fileName)
{
    for (size_t i = 0; i < files.size(); i++)
    {
        if (files[i].getName() == fileName)
        {
            files.erase(files.begin() + i);
            return;
        }
    }

    if (findDirectory(fileName) != nullptr)
    {
        throw std::string("Cannot remove a directory using rm.");
    }

    throw std::string("File not found: ") + fileName;
}
