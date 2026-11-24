
#include "FileSystem.h"

#include <algorithm>

// Constructor
FileSystem::FileSystem()
    : root("/"), currentPath("/")
{
}

// Split path into components
std::vector<std::string> FileSystem::splitPath(
    const std::string &path) const
{
    std::vector<std::string> parts;

    size_t i = 0;

    while (i < path.size())
    {
        // Skip '/'
        while (i < path.size() && path[i] == '/')
        {
            i++;
        }

        size_t start = i;

        // Find the next '/'
        while (i < path.size() && path[i] != '/')
        {
            i++;
        }

        if (start < i)
        {
            parts.push_back(path.substr(start, i - start));
        }
    }

    return parts;
}

// Validate file and directory names
void FileSystem::validateName(
    const std::string &name) const
{
    if (name.empty() || name == "." || name == "..")
    {
        throw std::string("Invalid name.");
    }

    for (size_t i = 0; i < name.size(); i++)
    {
        char c = name[i];

        bool valid =
            (c >= 'a' && c <= 'z') ||
            (c >= 'A' && c <= 'Z') ||
            (c >= '0' && c <= '9') ||
            c == '.' || c == '-' || c == '_';

        if (!valid)
        {
            throw std::string("Invalid name: ") + name;
        }
    }
}

// Find directory using absolute path
Directory *FileSystem::getDirectoryByPath(
    const std::string &path)
{
    Directory *current = &root;

    std::vector<std::string> parts = splitPath(path);

    for (size_t i = 0; i < parts.size(); i++)
    {
        current = current->findDirectory(parts[i]);

        if (current == nullptr)
        {
            throw std::string("Directory not found: ") + parts[i];
        }
    }

    return current;
}

// Const version
const Directory *FileSystem::getDirectoryByPath(
    const std::string &path) const
{
    const Directory *current = &root;

    std::vector<std::string> parts = splitPath(path);

    for (size_t i = 0; i < parts.size(); i++)
    {
        current = current->findDirectory(parts[i]);

        if (current == nullptr)
        {
            throw std::string("Directory not found: ") + parts[i];
        }
    }

    return current;
}

// Get current directory
Directory &FileSystem::getCurrentDirectory()
{
    return *getDirectoryByPath(currentPath);
}

const Directory &FileSystem::getCurrentDirectory() const
{
    return *getDirectoryByPath(currentPath);
}

// pwd
std::string FileSystem::pwd() const
{
    return currentPath;
}

// mkdir
void FileSystem::mkdir(const std::string &directoryName)
{
    validateName(directoryName);

    getCurrentDirectory().createDirectory(directoryName);
}

// cd
void FileSystem::cd(const std::string &address)
{
    if (address.empty())
    {
        throw std::string("Invalid directory address.");
    }

    std::vector<std::string> parts;

    // Absolute path
    if (address[0] != '/')
    {
        parts = splitPath(currentPath);
    }

    std::vector<std::string> newParts = splitPath(address);

    for (size_t i = 0; i < newParts.size(); i++)
    {
        const std::string &part = newParts[i];

        if (part == ".")
        {
            continue;
        }

        if (part == "..")
        {
            if (!parts.empty())
            {
                parts.pop_back();
            }
        }
        else
        {
            validateName(part);
            parts.push_back(part);
        }
    }

    // Build normalized absolute path
    std::string newPath = "/";

    for (size_t i = 0; i < parts.size(); i++)
    {
        if (i > 0)
        {
            newPath += "/";
        }

        newPath += parts[i];
    }

    // Check whether destination exists
    getDirectoryByPath(newPath);

    // Update current path only after successful validation
    currentPath = newPath;
}

// ls
std::vector<std::string> FileSystem::ls() const
{
    std::vector<std::string> result;

    const Directory &current = getCurrentDirectory();

    const std::vector<File> &files = current.getFiles();

    const std::vector<Directory> &directories =
        current.getDirectories();

    for (size_t i = 0; i < files.size(); i++)
    {
        result.push_back(files[i].getName());
    }

    for (size_t i = 0; i < directories.size(); i++)
    {
        result.push_back(directories[i].getName());
    }

    std::sort(result.begin(), result.end());

    return result;
}

// touch
void FileSystem::touch(const std::string &fileName)
{
    validateName(fileName);

    getCurrentDirectory().createFile(fileName);
}

// echo >
void FileSystem::echoOverwrite(
    const std::string &text,
    const std::string &fileName)
{
    validateName(fileName);

    File &file = getCurrentDirectory().createFile(fileName);

    file.overwrite(text);
}

// echo >>
void FileSystem::echoAppend(
    const std::string &text,
    const std::string &fileName)
{
    validateName(fileName);

    File &file = getCurrentDirectory().createFile(fileName);

    file.appendLine(text);
}

// cat
std::vector<std::string> FileSystem::cat(
    const std::string &fileName) const
{
    validateName(fileName);
    const File *file = getCurrentDirectory().findFile(fileName);

    if (file == nullptr)
    {
        throw std::string("File not found: ") + fileName;
    }

    return file->getLines();
}

// rm
void FileSystem::rm(const std::string &fileName)
{
    validateName(fileName);

    getCurrentDirectory().removeFile(fileName);
}

// Accessors for Repository
Directory &FileSystem::getRoot()
{
    return root;
}

const Directory &FileSystem::getRoot() const
{
    return root;
}

const std::string &FileSystem::getCurrentPath() const
{
    return currentPath;
}
