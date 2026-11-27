#include "CommandParser.h"
#include "Repository.h"

int main()
{
    FileSystem fs;
    Repository repo(fs);
    CommandParser parser(fs, repo);

    parser.run();

    return 0;
}
