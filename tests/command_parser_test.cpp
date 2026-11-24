
#include "CommandParser.h"

int main()
{
    FileSystem fs;

    CommandParser parser(fs);

    parser.run();

    return 0;
}
