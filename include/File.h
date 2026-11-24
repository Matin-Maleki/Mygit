#ifndef FILE_H
#define FILE_H

#include <string>
#include <vector>

class File
{
private:
    std::string name;
    std::vector<std::string> lines;

public:
    File(const std::string &fileName);

    const std::string &getName() const;

    const std::vector<std::string> &getLines() const;

    void setLines(const std::vector<std::string> &newLines);

    void overwrite(const std::string &text);

    void appendLine(const std::string &text);
};

#endif