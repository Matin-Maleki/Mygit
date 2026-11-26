#ifndef FILESNAPSHOT_H
#define FILESNAPSHOT_H

#include <string>
#include <vector>

struct FileSnapshot
{
    std::string path;
    std::vector<std::string> lines;

    FileSnapshot()
        : path(""), lines()
    {
    }

    FileSnapshot(
        const std::string &filePath,
        const std::vector<std::string> &fileLines)
        : path(filePath), lines(fileLines)
    {
    }
};

#endif