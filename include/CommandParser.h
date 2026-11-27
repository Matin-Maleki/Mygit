#ifndef COMMANDPARSER_H
#define COMMANDPARSER_H

#include <string>
#include <vector>

#include "FileSystem.h"
#include "Repository.h"

class CommandParser
{
private:
    FileSystem &fileSystem;
    Repository &repository;

    bool running;

    // Convert command line into tokens
    std::vector<std::string> tokenize(
        const std::string &line) const;

    // Execute Shell commands
    void executeShellCommand(
        const std::vector<std::string> &tokens);

    // Execute Git commands
    void executeGitCommand(
        const std::vector<std::string> &tokens);

public:
    // Both objects must outlive the parser. Repository must use the same
    // FileSystem instance passed here.
    CommandParser(FileSystem &fs, Repository &repo);

    // Start command-line interface
    void run();

    // Process one command
    bool executeLine(const std::string &line);
};

#endif
