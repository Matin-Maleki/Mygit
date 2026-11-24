
#include "FileSystem.h"

#include <iostream>
#include <string>
#include <vector>

using namespace std;

int main()
{
    FileSystem fs;

    try
    {

        try
        {
            // Test 1: pwd
            cout << "Current path: " << fs.pwd() << endl;

            // Test 2: mkdir
            fs.mkdir("project");

            // Test 3: cd
            fs.cd("project");

            cout << "Current path: " << fs.pwd() << endl;

            // Test 4: touch
            fs.touch("main.cpp");

            // Test 5: echo >
            fs.echoOverwrite("first line", "main.cpp");

            // Test 6: echo >>
            fs.echoAppend("second line", "main.cpp");

            // Test 7: cat
            cout << "\nFile content:" << endl;

            vector<string> content = fs.cat("main.cpp");

            for (size_t i = 0; i < content.size(); i++)
            {
                cout << content[i] << endl;
            }

            // Test 8: mkdir
            fs.mkdir("src");

            // Test 9: cd using relative path
            fs.cd("src");

            cout << "\nCurrent path: " << fs.pwd() << endl;

            // Test 10: cd ..
            fs.cd("..");

            cout << "\nCurrent path: " << fs.pwd() << endl;

            // Test 11: ls
            cout << "\nDirectory contents:" << endl;

            vector<string> entries = fs.ls();

            for (size_t i = 0; i < entries.size(); i++)
            {
                cout << entries[i] << endl;
            }

            // Test 12: rm
            fs.rm("main.cpp");

            cout << "\nAfter removing main.cpp:" << endl;

            entries = fs.ls();

            for (size_t i = 0; i < entries.size(); i++)
            {
                cout << entries[i] << endl;
            }

            // Test 13: cd /
            fs.cd("/");

            cout << "\nFinal path: " << fs.pwd() << endl;
        }
        catch (const string &error)
        {
            cout << "Error: " << error << endl;
        }

        // Error Tests

        cout << "\n========== ERROR TESTS ==========\n";

        // Test 14
        try
        {
            fs.mkdir("project");
        }
        catch (const string &error)
        {
            cout << "Duplicate directory: " << error << endl;
        }

        // Test 15
        try
        {
            fs.cd("unknown");
        }
        catch (const string &error)
        {
            cout << "Invalid cd: " << error << endl;
        }

        // Test 16
        try
        {
            fs.cat("unknown.txt");
        }
        catch (const string &error)
        {
            cout << "Invalid cat: " << error << endl;
        }

        // Test 17
        try
        {
            fs.rm("unknown.txt");
        }
        catch (const string &error)
        {
            cout << "Invalid rm: " << error << endl;
        }

        // Test 18
        try
        {
            fs.cd("/project");
            fs.rm("src");
        }
        catch (const string &error)
        {
            cout << "Remove directory: " << error << endl;
        }

        cout << "\n========== TESTS FINISHED ==========\n";
    }
    catch (const string &error)
    {
        cout << "Error: " << error << endl;
    }

    return 0;
}