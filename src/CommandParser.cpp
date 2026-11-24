
#include "CommandParser.h"

#include <iostream>
#include <cctype>

// Constructor
CommandParser::CommandParser(FileSystem &fs)
    : fileSystem(fs), running(true)
{
}

// Tokenize command
std::vector<std::string> CommandParser::tokenize(
    const std::string &line) const
{
    std::vector<std::string> tokens;

    std::string currentToken;

    bool insideQuotes = false;
    bool tokenStarted = false;

    char quoteType = '\0';

    for (std::size_t i = 0; i < line.size(); i++)
    {
        char c = line[i];

        // Inside quotation marks
        if (insideQuotes)
        {
            if (c == quoteType)
            {
                insideQuotes = false;
            }
            else
            {
                currentToken += c;
            }

            continue;
        }

        // Start quotation marks
        if (c == '"' || c == '\'')
        {
            insideQuotes = true;
            quoteType = c;
            tokenStarted = true;
            continue;
        }

        // Handle whitespace
        if (std::isspace(static_cast<unsigned char>(c)))
        {
            if (tokenStarted)
            {
                tokens.push_back(currentToken);

                currentToken.clear();
                tokenStarted = false;
            }

            continue;
        }

        // Handle redirection operators
        if (c == '>')
        {
            if (tokenStarted)
            {
                tokens.push_back(currentToken);

                currentToken.clear();
                tokenStarted = false;
            }

            if (i + 1 < line.size() && line[i + 1] == '>')
            {
                tokens.push_back(">>");
                i++;
            }
            else
            {
                tokens.push_back(">");
            }

            continue;
        }

        currentToken += c;
        tokenStarted = true;
    }

    // Check quotation marks
    if (insideQuotes)
    {
        throw std::string("Unclosed quotation marks.");
    }

    // Add last token
    if (tokenStarted)
    {
        tokens.push_back(currentToken);
    }

    return tokens;
}

// Execute Shell commands
void CommandParser::executeShellCommand(
    const std::vector<std::string> &tokens)
{
    if (tokens.empty())
    {
        return;
    }

    const std::string &command = tokens[0];

    // pwd
    if (command == "pwd")
    {
        if (tokens.size() != 1)
        {
            throw std::string("Usage: pwd");
        }

        std::cout << fileSystem.pwd() << std::endl;
    }

    // mkdir
    else if (command == "mkdir")
    {
        if (tokens.size() != 2)
        {
            throw std::string("Usage: mkdir <DIRECTORY_NAME>");
        }

        fileSystem.mkdir(tokens[1]);
    }

    // cd
    else if (command == "cd")
    {
        if (tokens.size() != 2)
        {
            throw std::string("Usage: cd <DIRECTORY_ADDRESS>");
        }

        fileSystem.cd(tokens[1]);
    }

    // ls
    else if (command == "ls")
    {
        if (tokens.size() != 1)
        {
            throw std::string("Usage: ls");
        }

        std::vector<std::string> entries = fileSystem.ls();

        for (std::size_t i = 0; i < entries.size(); i++)
        {
            std::cout << entries[i] << std::endl;
        }
    }

    // touch
    else if (command == "touch")
    {
        if (tokens.size() != 2)
        {
            throw std::string("Usage: touch <FILE_NAME>");
        }

        fileSystem.touch(tokens[1]);
    }

    // echo
    else if (command == "echo")
    {
        if (tokens.size() != 4)
        {
            throw std::string(
                "Usage: echo \"<TEXT>\" > <FILE_NAME>");
        }

        const std::string &text = tokens[1];
        const std::string &operation = tokens[2];
        const std::string &fileName = tokens[3];

        if (operation == ">")
        {
            fileSystem.echoOverwrite(text, fileName);
        }
        else if (operation == ">>")
        {
            fileSystem.echoAppend(text, fileName);
        }
        else
        {
            throw std::string(
                "Invalid echo operator. Use > or >>.");
        }
    }

    // cat
    else if (command == "cat")
    {
        if (tokens.size() != 2)
        {
            throw std::string("Usage: cat <FILE_NAME>");
        }

        std::vector<std::string> lines =
            fileSystem.cat(tokens[1]);

        for (std::size_t i = 0; i < lines.size(); i++)
        {
            std::cout << lines[i] << std::endl;
        }
    }

    // rm
    else if (command == "rm")
    {
        if (tokens.size() != 2)
        {
            throw std::string("Usage: rm <FILE_NAME>");
        }

        fileSystem.rm(tokens[1]);
    }

    // Unknown command
    else
    {
        throw std::string("Unknown command: ") + command;
    }
}

// Execute one command
bool CommandParser::executeLine(const std::string &line)
{
    try
    {
        std::vector<std::string> tokens = tokenize(line);

        // Empty command
        if (tokens.empty())
        {
            return true;
        }

        // exit
        if (tokens[0] == "exit")
        {
            if (tokens.size() != 1)
            {
                throw std::string("Usage: exit");
            }

            running = false;
            return false;
        }

        executeShellCommand(tokens);
    }
    catch (const std::string &error)
    {
        std::cout << "Error: " << error << std::endl;
    }

    return running;
}

// Run command-line interface
void CommandParser::run()
{
    std::string line;

    while (running)
    {
        std::cout << "$ ";

        if (!std::getline(std::cin, line))
        {
            running = false;
            break;
        }

        executeLine(line);
    }
}
