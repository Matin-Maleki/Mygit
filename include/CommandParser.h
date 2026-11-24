
#ifndef COMMANDPARSER_H
#define COMMANDPARSER_H

#include <string>
#include <vector>

#include "FileSystem.h"

class CommandParser
{
private:
    FileSystem &fileSystem;

    bool running;

    // Convert command line into tokens
    std::vector<std::string> tokenize(
        const std::string &line) const;

    // Execute Shell commands
    void executeShellCommand(
        const std::vector<std::string> &tokens);

public:
    // Constructor
    CommandParser(FileSystem &fs);

    // Start command-line interface
    void run();

    // Process one command
    bool executeLine(const std::string &line);
};

#endif
