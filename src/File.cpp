#include "File.h"

File::File(const std::string &fileName)
{
    name = fileName;
}

const std::string &File::getName() const
{
    return name;
}

const std::vector<std::string> &File::getLines() const
{
    return lines;
}

void File::setLines(const std::vector<std::string> &newLines)
{
    lines = newLines;
}

void File::overwrite(const std::string &text)
{
    lines.clear();
    lines.push_back(text);
}

void File::appendLine(const std::string &text)
{
    lines.push_back(text);
}