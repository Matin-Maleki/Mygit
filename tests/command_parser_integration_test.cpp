#include "CommandParser.h"

#include <iostream>
#include <string>

int main()
{
    FileSystem fs;
    Repository repo(fs);
    CommandParser parser(fs, repo);

    if (!parser.executeLine("git config user.name \"Test User\"")) return 1;
    if (!parser.executeLine("git config user.email \"test@example.com\"")) return 2;
    if (!parser.executeLine("git init")) return 3;
    if (!repo.isInitialized()) return 4;

    if (!parser.executeLine("touch readme.txt")) return 5;
    if (!parser.executeLine("echo \"hello world\" > readme.txt")) return 6;
    if (!parser.executeLine("git add readme.txt")) return 7;
    if (!parser.executeLine("git commit -m \"first commit with spaces\"")) return 8;

    if (repo.getCurrentBranch() != "main") return 9;

    if (!parser.executeLine("touch notes.txt")) return 10;
    if (!parser.executeLine("git add .")) return 11;
    if (!parser.executeLine("git reset notes.txt")) return 12;

    if (!parser.executeLine("git status")) return 13;
    if (!parser.executeLine("git log")) return 14;

    if (parser.executeLine("exit")) return 15;

    std::cout << "CommandParser integration smoke test passed.\n";
    return 0;
}
