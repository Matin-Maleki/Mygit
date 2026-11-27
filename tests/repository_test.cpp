#include "Repository.h"

#include <iostream>
#include <sstream>
#include <string>
#include <vector>

namespace
{
    int passedTests = 0;
    int failedTests = 0;

    void check(bool condition, const std::string &testName)
    {
        if (condition)
        {
            passedTests++;
            std::cout << "[PASS] " << testName << std::endl;
        }
        else
        {
            failedTests++;
            std::cout << "[FAIL] " << testName << std::endl;
        }
    }

    void checkContains(
        const std::string &text,
        const std::string &expected,
        const std::string &testName)
    {
        check(text.find(expected) != std::string::npos, testName);
    }

    void checkDoesNotContain(
        const std::string &text,
        const std::string &unexpected,
        const std::string &testName)
    {
        check(text.find(unexpected) == std::string::npos, testName);
    }

    template <typename Action>
    void expectStringError(
        const std::string &testName,
        Action action,
        const std::string &expectedMessagePart = "")
    {
        try
        {
            action();
            check(false, testName);
        }
        catch (const std::string &error)
        {
            if (expectedMessagePart.empty() ||
                error.find(expectedMessagePart) != std::string::npos)
            {
                check(true, testName);
            }
            else
            {
                check(false, testName + " (unexpected error: " + error + ")");
            }
        }
    }

    template <typename Action>
    std::string captureOutput(Action action)
    {
        std::ostringstream output;
        std::streambuf *oldBuffer = std::cout.rdbuf(output.rdbuf());

        action();

        std::cout.rdbuf(oldBuffer);
        return output.str();
    }

    void configure(Repository &repository)
    {
        repository.setUserName("Reza");
        repository.setUserEmail("reza@example.com");
    }

    void testInitializationAndConfiguration()
    {
        std::cout << "\n========== INITIALIZATION AND CONFIGURATION =========="
                  << std::endl;

        FileSystem fileSystem;
        Repository repository(fileSystem);

        check(!repository.isInitialized(),
              "Repository is not initialized at construction");
        check(repository.getRootPath().empty(),
              "Repository root is empty before init");

        expectStringError(
            "add before init is rejected",
            [&repository]() { repository.add("file.txt"); },
            "not initialized");

        expectStringError(
            "status before init is rejected",
            [&repository]() { repository.status(); },
            "not initialized");

        expectStringError(
            "empty username is rejected",
            [&repository]() { repository.setUserName(""); },
            "Username");

        expectStringError(
            "empty email is rejected",
            [&repository]() { repository.setUserEmail(""); },
            "Email");

        // Configuration is allowed before init.
        configure(repository);

        fileSystem.mkdir("workspace");
        fileSystem.cd("workspace");
        repository.init();

        check(repository.isInitialized(), "init marks repository initialized");
        check(repository.getRootPath() == "/workspace",
              "init uses current directory as repository root");
        check(repository.getCurrentBranch() == "main",
              "init creates and selects main branch");

        expectStringError(
            "a second init on the same Repository is rejected",
            [&repository]() { repository.init(); },
            "already exists");
    }

    void testAddCommitAndHistory()
    {
        std::cout << "\n========== ADD, COMMIT AND LOG ==========" << std::endl;

        FileSystem fileSystem;
        fileSystem.mkdir("project");
        fileSystem.cd("project");

        Repository repository(fileSystem);
        configure(repository);
        repository.init();

        fileSystem.touch("README.md");
        fileSystem.echoOverwrite("Git Simulator", "README.md");

        const std::string initialStatus =
            captureOutput([&repository]() { repository.status(); });
        checkContains(initialStatus, "Untracked files:",
                      "status lists an untracked file");
        checkContains(initialStatus, "README.md",
                      "status prints the untracked file path");

        const std::string unstagedCommitOutput = captureOutput(
            [&repository]() { repository.commit("should not be created"); });
        checkContains(unstagedCommitOutput, "no changes added to commit",
                      "commit refuses to commit unstaged changes");

        repository.add("README.md");
        const std::string stagedStatus =
            captureOutput([&repository]() { repository.status(); });
        checkContains(stagedStatus, "Staged changes:",
                      "add stages a new file");
        checkContains(stagedStatus, "new file: README.md",
                      "status classifies a staged new file");

        const std::string firstCommitOutput = captureOutput(
            [&repository]() { repository.commit("initial project"); });
        checkContains(firstCommitOutput, "[main 1] initial project",
                      "first commit receives ID 1");

        std::string logOutput =
            captureOutput([&repository]() { repository.log(); });
        checkContains(logOutput, "commit 1", "log prints the first commit ID");
        checkContains(logOutput, "Author: Reza <reza@example.com>",
                      "log prints configured author information");
        checkContains(logOutput, "initial project",
                      "log prints the commit message");
        checkContains(logOutput, "Date: ", "log prints a commit date");

        // A second commit from a nested directory must use a repository-relative path.
        fileSystem.mkdir("src");
        fileSystem.cd("src");
        fileSystem.touch("main.cpp");
        fileSystem.echoOverwrite("int main() {}", "main.cpp");
        repository.add("main.cpp");

        const std::string secondCommitOutput = captureOutput(
            [&repository]() { repository.commit("add main source"); });
        checkContains(secondCommitOutput, "[main 2] add main source",
                      "commit IDs increase globally within the repository");

        const std::string cleanStatus =
            captureOutput([&repository]() { repository.status(); });
        checkContains(cleanStatus, "nothing to commit, working tree clean",
                      "staging contains the complete snapshot after commit");
        checkDoesNotContain(cleanStatus, "Untracked files:",
                            "previously committed root files remain tracked");

        const std::string cleanCommitOutput = captureOutput(
            [&repository]() { repository.commit("empty commit"); });
        checkContains(cleanCommitOutput,
                      "nothing to commit, working tree clean",
                      "commit does not create an empty commit");

        logOutput = captureOutput([&repository]() { repository.log(); });
        const std::size_t secondPosition = logOutput.find("commit 2");
        const std::size_t firstPosition = logOutput.find("commit 1");
        check(secondPosition != std::string::npos &&
                  firstPosition != std::string::npos &&
                  secondPosition < firstPosition,
              "log lists commits newest first");

    }

    void testModifiedFilesAndReset()
    {
        std::cout << "\n========== MODIFIED FILES AND RESET =========="
                  << std::endl;

        FileSystem fileSystem;
        fileSystem.mkdir("repo");
        fileSystem.cd("repo");

        Repository repository(fileSystem);
        configure(repository);
        repository.init();

        fileSystem.touch("notes.txt");
        fileSystem.echoOverwrite("version 1", "notes.txt");
        repository.add("notes.txt");
        captureOutput([&repository]() { repository.commit("add notes"); });

        fileSystem.echoOverwrite("version 2", "notes.txt");
        std::string status =
            captureOutput([&repository]() { repository.status(); });
        checkContains(status, "Modified (not staged):",
                      "working-tree modification is shown as unstaged");
        checkContains(status, "modified: notes.txt",
                      "unstaged modified path is displayed");

        repository.add("notes.txt");
        status = captureOutput([&repository]() { repository.status(); });
        checkContains(status, "Staged changes:",
                      "add stages a modification");
        checkContains(status, "modified: notes.txt",
                      "status classifies the staged modification");

        fileSystem.echoOverwrite("version 3", "notes.txt");
        status = captureOutput([&repository]() { repository.status(); });
        checkContains(status, "Staged changes:",
                      "staged modification remains recorded");
        checkContains(status, "Modified (not staged):",
                      "later working-tree edit is also shown as unstaged");

        repository.reset("notes.txt");
        status = captureOutput([&repository]() { repository.status(); });
        checkDoesNotContain(status, "Staged changes:",
                            "reset restores the staging entry to HEAD");
        checkContains(status, "Modified (not staged):",
                      "reset does not modify the working tree");

        const std::vector<std::string> workingLines =
            fileSystem.cat("notes.txt");
        check(workingLines.size() == 1 && workingLines[0] == "version 3",
              "reset leaves the working-tree file content unchanged");

        repository.add("notes.txt");
        const std::string secondCommitOutput = captureOutput(
            [&repository]() { repository.commit("update notes"); });
        checkContains(secondCommitOutput, "[main 2] update notes",
                      "second commit succeeds after staging the modification");
    }

    void testDeletionAndReset()
    {
        std::cout << "\n========== DELETION AND RESET ==========" << std::endl;

        FileSystem fileSystem;
        fileSystem.mkdir("repo");
        fileSystem.cd("repo");

        Repository repository(fileSystem);
        configure(repository);
        repository.init();

        fileSystem.touch("tracked.txt");
        fileSystem.echoOverwrite("keep me", "tracked.txt");
        repository.add("tracked.txt");
        captureOutput([&repository]() { repository.commit("track file"); });

        fileSystem.rm("tracked.txt");
        std::string status =
            captureOutput([&repository]() { repository.status(); });
        checkContains(status, "Modified (not staged):",
                      "deletion before add is shown as unstaged");
        checkContains(status, "deleted: tracked.txt",
                      "unstaged deletion path is displayed");

        repository.add("tracked.txt");
        status = captureOutput([&repository]() { repository.status(); });
        checkContains(status, "Staged changes:",
                      "adding a missing tracked file stages its deletion");
        checkContains(status, "deleted: tracked.txt",
                      "status classifies the staged deletion");

        repository.reset("tracked.txt");
        status = captureOutput([&repository]() { repository.status(); });
        checkDoesNotContain(status, "Staged changes:",
                            "reset cancels a staged deletion");
        checkContains(status, "Modified (not staged):",
                      "canceling a staged deletion does not recreate the file");
        checkContains(status, "deleted: tracked.txt",
                      "working-tree deletion remains after reset");

        // Stage and commit the deletion.
        repository.add("tracked.txt");
        const std::string deletionCommit = captureOutput(
            [&repository]() { repository.commit("delete tracked file"); });
        checkContains(deletionCommit, "[main 2] delete tracked file",
                      "a staged deletion can be committed");

        status = captureOutput([&repository]() { repository.status(); });
        checkContains(status, "nothing to commit, working tree clean",
                      "working tree is clean after deletion commit");
    }

    void testAddAllSubtreeAndUntrackedScope()
    {
        std::cout << "\n========== ADD ALL SUBTREE ==========" << std::endl;

        FileSystem fileSystem;
        fileSystem.mkdir("repo");
        fileSystem.cd("repo");

        Repository repository(fileSystem);
        configure(repository);
        repository.init();

        fileSystem.touch("root.txt");
        fileSystem.echoOverwrite("root file", "root.txt");
        repository.add("root.txt");
        captureOutput([&repository]() { repository.commit("root file"); });

        fileSystem.mkdir("docs");
        fileSystem.cd("docs");
        fileSystem.mkdir("spec");
        fileSystem.touch("guide.txt");
        fileSystem.echoOverwrite("guide", "guide.txt");
        fileSystem.cd("spec");
        fileSystem.touch("details.txt");
        fileSystem.echoOverwrite("details", "details.txt");
        fileSystem.cd("..");

        // A file outside /docs but inside the repository must not be staged.
        fileSystem.cd("/repo");
        fileSystem.touch("outside.txt");
        fileSystem.echoOverwrite("outside subtree", "outside.txt");
        fileSystem.cd("/repo/docs");

        repository.addAll();
        std::string status =
            captureOutput([&repository]() { repository.status(); });
        checkContains(status, "new file: docs/guide.txt",
                      "addAll stages a file in the current directory");
        checkContains(status, "new file: docs/spec/details.txt",
                      "addAll recursively stages nested files");
        checkDoesNotContain(status, "new file: outside.txt",
                            "addAll does not stage files outside its subtree");

        const std::string commitOutput = captureOutput(
            [&repository]() { repository.commit("add documentation"); });
        checkContains(commitOutput, "[main 2] add documentation",
                      "addAll snapshot can be committed");

        status = captureOutput([&repository]() { repository.status(); });
        checkContains(status, "Untracked files:",
                      "file outside addAll scope remains untracked");
        checkContains(status, "outside.txt",
                      "untracked file outside subtree is reported");
        checkDoesNotContain(status, "root.txt",
                            "previous root commit remains tracked");

        // Remove one file inside the scope and modify another, then addAll again.
        fileSystem.rm("guide.txt");
        fileSystem.cd("spec");
        fileSystem.echoOverwrite("details updated", "details.txt");
        fileSystem.cd("..");
        repository.addAll();

        status = captureOutput([&repository]() { repository.status(); });
        checkContains(status, "deleted: docs/guide.txt",
                      "addAll stages deletions in its subtree");
        checkContains(status, "modified: docs/spec/details.txt",
                      "addAll stages modifications recursively");

        const std::string updateCommit = captureOutput(
            [&repository]() { repository.commit("update documentation"); });
        checkContains(updateCommit, "[main 3] update documentation",
                      "subtree deletion and modification commit together");
    }


    void testNewFileResetAndAddAfterDelete()
    {
        std::cout << "\n========== NEW FILE RESET AND ADD AFTER DELETE =========="
                  << std::endl;

        FileSystem fileSystem;
        fileSystem.mkdir("repo");
        fileSystem.cd("repo");

        Repository repository(fileSystem);
        configure(repository);
        repository.init();

        fileSystem.touch("temporary.txt");
        fileSystem.echoOverwrite("temporary", "temporary.txt");
        repository.add("temporary.txt");

        std::string status =
            captureOutput([&repository]() { repository.status(); });
        checkContains(status, "new file: temporary.txt",
                      "newly staged file is classified as new");

        repository.reset("temporary.txt");
        status = captureOutput([&repository]() { repository.status(); });
        checkDoesNotContain(status, "Staged changes:",
                            "reset removes a staged file absent from HEAD");
        checkContains(status, "Untracked files:",
                      "reset leaves the new working-tree file untracked");

        repository.add("temporary.txt");
        fileSystem.rm("temporary.txt");
        repository.add("temporary.txt");
        status = captureOutput([&repository]() { repository.status(); });
        checkDoesNotContain(status, "Staged changes:",
                            "adding a deleted new file cancels its staged addition");
        checkDoesNotContain(status, "temporary.txt",
                            "canceled new file no longer appears in status");
    }


    void testMissingConfigurationAndRootRepository()
    {
        std::cout << "\n========== CONFIGURATION AND ROOT REPOSITORY =========="
                  << std::endl;

        FileSystem unconfiguredFileSystem;
        unconfiguredFileSystem.mkdir("project");
        unconfiguredFileSystem.cd("project");

        Repository unconfiguredRepository(unconfiguredFileSystem);
        unconfiguredRepository.init();
        unconfiguredFileSystem.touch("file.txt");
        unconfiguredRepository.add("file.txt");

        expectStringError(
            "commit without user configuration is rejected",
            [&unconfiguredRepository]()
            {
                unconfiguredRepository.commit("missing identity");
            },
            "configure user.name and user.email");

        configure(unconfiguredRepository);
        const std::string configuredCommit = captureOutput(
            [&unconfiguredRepository]()
            {
                unconfiguredRepository.commit("configured identity");
            });
        checkContains(configuredCommit, "[main 1] configured identity",
                      "commit succeeds after configuring user identity");

        // A repository initialized at '/' must still produce relative paths.
        FileSystem rootFileSystem;
        Repository rootRepository(rootFileSystem);
        configure(rootRepository);
        rootRepository.init();

        rootFileSystem.mkdir("module");
        rootFileSystem.cd("module");
        rootFileSystem.touch("unit.cpp");
        rootFileSystem.echoOverwrite("int unit = 1;", "unit.cpp");
        rootRepository.add("unit.cpp");

        const std::string rootCommit = captureOutput(
            [&rootRepository]() { rootRepository.commit("root repository"); });
        checkContains(rootCommit, "[main 1] root repository",
                      "repository initialized at virtual root can commit");

        const std::string rootStatus = captureOutput(
            [&rootRepository]() { rootRepository.status(); });
        checkContains(rootStatus, "nothing to commit, working tree clean",
                      "root repository uses correct relative snapshot paths");
    }

    void testErrorsAndRepositoryBoundary()
    {
        std::cout << "\n========== ERRORS AND REPOSITORY BOUNDARY =========="
                  << std::endl;

        FileSystem fileSystem;
        fileSystem.mkdir("repo");
        fileSystem.cd("repo");

        Repository repository(fileSystem);
        configure(repository);
        repository.init();

        expectStringError(
            "adding a nonexistent untracked file is rejected",
            [&repository]() { repository.add("missing.txt"); },
            "File not found");

        fileSystem.mkdir("folder");
        expectStringError(
            "adding a directory as a file is rejected",
            [&repository]() { repository.add("folder"); },
            "directory");

        expectStringError(
            "invalid file names are rejected",
            [&repository]() { repository.add("bad/name.txt"); },
            "Invalid file name");

        fileSystem.cd("/");
        expectStringError(
            "add outside repository is rejected",
            [&repository]() { repository.add("outside.txt"); },
            "outside the repository");

        expectStringError(
            "addAll outside repository is rejected",
            [&repository]() { repository.addAll(); },
            "outside the repository");

        expectStringError(
            "status outside repository is rejected",
            [&repository]() { repository.status(); },
            "outside the repository");

        expectStringError(
            "log outside repository is rejected",
            [&repository]() { repository.log(); },
            "outside the repository");

        fileSystem.cd("/repo");
        expectStringError(
            "log with no commits is rejected",
            [&repository]() { repository.log(); },
            "No commits yet");

        expectStringError(
            "reset before init is rejected in a separate Repository",
            []()
            {
                FileSystem anotherFileSystem;
                Repository anotherRepository(anotherFileSystem);
                anotherRepository.reset("file.txt");
            },
            "not initialized");
    }
}

int main()
{
    testInitializationAndConfiguration();
    testAddCommitAndHistory();
    testModifiedFilesAndReset();
    testDeletionAndReset();
    testAddAllSubtreeAndUntrackedScope();
    testNewFileResetAndAddAfterDelete();
    testMissingConfigurationAndRootRepository();
    testErrorsAndRepositoryBoundary();

    std::cout << "\n========== TEST SUMMARY ==========" << std::endl;
    std::cout << "Passed: " << passedTests << std::endl;
    std::cout << "Failed: " << failedTests << std::endl;

    return failedTests == 0 ? 0 : 1;
}
