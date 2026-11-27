#include "Repository.h"

#include <algorithm>
#include <ctime>
#include <iomanip>
#include <iostream>

namespace
{
    bool isValidEntryName(const std::string &name)
    {
        if (name.empty() || name == "." || name == "..")
        {
            return false;
        }

        for (std::size_t i = 0; i < name.size(); i++)
        {
            const char c = name[i];

            const bool valid =
                (c >= 'a' && c <= 'z') ||
                (c >= 'A' && c <= 'Z') ||
                (c >= '0' && c <= '9') ||
                c == '.' || c == '-' || c == '_';

            if (!valid)
            {
                return false;
            }
        }

        return true;
    }

    bool isPathInsideScope(
        const std::string &path,
        const std::string &scope)
    {
        if (scope.empty())
        {
            return true;
        }

        if (path == scope)
        {
            return true;
        }

        if (path.size() <= scope.size())
        {
            return false;
        }

        return path.compare(0, scope.size(), scope) == 0 &&
               path[scope.size()] == '/';
    }

    void sortSnapshots(std::vector<FileSnapshot> &files)
    {
        std::sort(
            files.begin(),
            files.end(),
            [](const FileSnapshot &first, const FileSnapshot &second)
            {
                return first.path < second.path;
            });
    }

    void sortPaths(std::vector<std::string> &paths)
    {
        std::sort(paths.begin(), paths.end());
    }
}

// Constructor
Repository::Repository(FileSystem &fs)
    : fileSystem(fs),
      initialized(false),
      repoRootPath(""),
      userName(""),
      userEmail(""),
      commits(),
      branches(),
      staging(),
      currentBranch(""),
      nextCommitId(1)
{
}

// User configuration
void Repository::setUserName(const std::string &name)
{
    if (name.empty())
    {
        throw std::string("Username cannot be empty.");
    }

    userName = name;
}

void Repository::setUserEmail(const std::string &email)
{
    if (email.empty())
    {
        throw std::string("Email cannot be empty.");
    }

    userEmail = email;
}

// Split an absolute path into its non-empty components
std::vector<std::string> Repository::splitPath(
    const std::string &path) const
{
    std::vector<std::string> parts;
    std::size_t i = 0;

    while (i < path.size())
    {
        while (i < path.size() && path[i] == '/')
        {
            i++;
        }

        const std::size_t start = i;

        while (i < path.size() && path[i] != '/')
        {
            i++;
        }

        if (start < i)
        {
            parts.push_back(path.substr(start, i - start));
        }
    }

    return parts;
}

// Find a directory by its absolute path
Directory *Repository::findDirectoryByPath(
    const std::string &path)
{
    if (path.empty() || path[0] != '/')
    {
        return nullptr;
    }

    Directory *current = &fileSystem.getRoot();
    const std::vector<std::string> parts = splitPath(path);

    for (std::size_t i = 0; i < parts.size(); i++)
    {
        current = current->findDirectory(parts[i]);

        if (current == nullptr)
        {
            return nullptr;
        }
    }

    return current;
}

// Const version
const Directory *Repository::findDirectoryByPath(
    const std::string &path) const
{
    if (path.empty() || path[0] != '/')
    {
        return nullptr;
    }

    const Directory *current = &fileSystem.getRoot();
    const std::vector<std::string> parts = splitPath(path);

    for (std::size_t i = 0; i < parts.size(); i++)
    {
        current = current->findDirectory(parts[i]);

        if (current == nullptr)
        {
            return nullptr;
        }
    }

    return current;
}

// Convert an absolute path inside this repository to a repository-relative path
std::string Repository::getRelativePath(
    const std::string &absolutePath) const
{
    if (!isInsideRepository(absolutePath))
    {
        throw std::string("Path is outside the repository: ") + absolutePath;
    }

    if (absolutePath == repoRootPath)
    {
        return "";
    }

    if (repoRootPath == "/")
    {
        return absolutePath.substr(1);
    }

    return absolutePath.substr(repoRootPath.size() + 1);
}

// Resolve a file name relative to the current working directory
std::string Repository::getFilePath(
    const std::string &fileName) const
{
    if (!isValidEntryName(fileName))
    {
        throw std::string("Invalid file name: ") + fileName;
    }

    const std::string currentPath = fileSystem.getCurrentPath();

    if (!isInsideRepository(currentPath))
    {
        throw std::string("Current directory is outside the repository.");
    }

    const std::string relativeDirectory = getRelativePath(currentPath);

    if (relativeDirectory.empty())
    {
        return fileName;
    }

    return relativeDirectory + "/" + fileName;
}

// Check whether an absolute path belongs to the repository
bool Repository::isInsideRepository(
    const std::string &path) const
{
    if (!initialized || repoRootPath.empty() ||
        path.empty() || path[0] != '/')
    {
        return false;
    }

    if (repoRootPath == "/")
    {
        return true;
    }

    if (path == repoRootPath)
    {
        return true;
    }

    if (path.size() <= repoRootPath.size())
    {
        return false;
    }

    return path.compare(0, repoRootPath.size(), repoRootPath) == 0 &&
           path[repoRootPath.size()] == '/';
}

// Recursively collect every file below a directory
void Repository::collectSnapshot(
    const Directory &directory,
    const std::string &relativePath,
    std::vector<FileSnapshot> &result) const
{
    const std::vector<File> &files = directory.getFiles();

    for (std::size_t i = 0; i < files.size(); i++)
    {
        std::string filePath;

        if (relativePath.empty())
        {
            filePath = files[i].getName();
        }
        else
        {
            filePath = relativePath + "/" + files[i].getName();
        }

        result.push_back(
            FileSnapshot(filePath, files[i].getLines()));
    }

    const std::vector<Directory> &directories =
        directory.getDirectories();

    for (std::size_t i = 0; i < directories.size(); i++)
    {
        std::string childPath;

        if (relativePath.empty())
        {
            childPath = directories[i].getName();
        }
        else
        {
            childPath = relativePath + "/" + directories[i].getName();
        }

        collectSnapshot(directories[i], childPath, result);
    }
}

// Take a complete snapshot of the repository working tree
std::vector<FileSnapshot> Repository::getCurrentSnapshot() const
{
    if (!initialized)
    {
        throw std::string("Repository is not initialized.");
    }

    const Directory *repositoryRoot =
        findDirectoryByPath(repoRootPath);

    if (repositoryRoot == nullptr)
    {
        throw std::string("Repository root directory no longer exists.");
    }

    std::vector<FileSnapshot> result;
    collectSnapshot(*repositoryRoot, "", result);
    sortSnapshots(result);

    return result;
}

// Get the complete snapshot referenced by the current branch
std::vector<FileSnapshot> Repository::getHeadSnapshot() const
{
    if (!initialized)
    {
        throw std::string("Repository is not initialized.");
    }

    if (currentBranch.empty())
    {
        throw std::string("HEAD is detached.");
    }

    const Branch *branch = findBranch(currentBranch);

    if (branch == nullptr)
    {
        throw std::string("Current branch does not exist: ") + currentBranch;
    }

    if (branch->getCommitId() == 0)
    {
        return std::vector<FileSnapshot>();
    }

    const Commit *headCommit = findCommit(branch->getCommitId());

    if (headCommit == nullptr)
    {
        throw std::string("HEAD points to an unknown commit.");
    }

    return headCommit->getSnapshot();
}

// Find a snapshot entry (const version)
const FileSnapshot *Repository::findSnapshot(
    const std::vector<FileSnapshot> &files,
    const std::string &path) const
{
    for (std::size_t i = 0; i < files.size(); i++)
    {
        if (files[i].path == path)
        {
            return &files[i];
        }
    }

    return nullptr;
}

// Find a snapshot entry (mutable version)
FileSnapshot *Repository::findSnapshot(
    std::vector<FileSnapshot> &files,
    const std::string &path)
{
    for (std::size_t i = 0; i < files.size(); i++)
    {
        if (files[i].path == path)
        {
            return &files[i];
        }
    }

    return nullptr;
}

// Find a commit by ID (const version)
const Commit *Repository::findCommit(int id) const
{
    for (std::size_t i = 0; i < commits.size(); i++)
    {
        if (commits[i].getId() == id)
        {
            return &commits[i];
        }
    }

    return nullptr;
}

// Find a commit by ID
Commit *Repository::findCommit(int id)
{
    for (std::size_t i = 0; i < commits.size(); i++)
    {
        if (commits[i].getId() == id)
        {
            return &commits[i];
        }
    }

    return nullptr;
}

// Find a branch by name (const version)
const Branch *Repository::findBranch(
    const std::string &name) const
{
    for (std::size_t i = 0; i < branches.size(); i++)
    {
        if (branches[i].getName() == name)
        {
            return &branches[i];
        }
    }

    return nullptr;
}

// Find a branch by name
Branch *Repository::findBranch(
    const std::string &name)
{
    for (std::size_t i = 0; i < branches.size(); i++)
    {
        if (branches[i].getName() == name)
        {
            return &branches[i];
        }
    }

    return nullptr;
}

// Compare two complete snapshots independent of vector order
bool Repository::snapshotsEqual(
    const std::vector<FileSnapshot> &first,
    const std::vector<FileSnapshot> &second) const
{
    if (first.size() != second.size())
    {
        return false;
    }

    for (std::size_t i = 0; i < first.size(); i++)
    {
        const FileSnapshot *match =
            findSnapshot(second, first[i].path);

        if (match == nullptr || match->lines != first[i].lines)
        {
            return false;
        }
    }

    return true;
}

// Replace one file in staging, or insert it if it is not staged yet
void Repository::replaceStagedFile(const FileSnapshot &file)
{
    FileSnapshot *existing = findSnapshot(staging, file.path);

    if (existing != nullptr)
    {
        *existing = file;
    }
    else
    {
        staging.push_back(file);
        sortSnapshots(staging);
    }
}

// Remove one file from the staging snapshot
void Repository::removeStagedFile(const std::string &path)
{
    for (std::size_t i = 0; i < staging.size(); i++)
    {
        if (staging[i].path == path)
        {
            staging.erase(staging.begin() + i);
            return;
        }
    }
}

// Initialize the repository at the current directory
void Repository::init()
{
    if (initialized)
    {
        throw std::string("A repository already exists.");
    }

    repoRootPath = fileSystem.getCurrentPath();
    commits.clear();
    branches.clear();
    staging.clear();
    currentBranch = "main";
    nextCommitId = 1;

    branches.push_back(Branch("main", 0));
    initialized = true;
}

// Stage one file, or record its deletion
void Repository::add(const std::string &fileName)
{
    if (!initialized)
    {
        throw std::string("Repository is not initialized.");
    }

    const std::string path = getFilePath(fileName);
    Directory &currentDirectory = fileSystem.getCurrentDirectory();
    File *workingFile = currentDirectory.findFile(fileName);

    if (workingFile != nullptr)
    {
        replaceStagedFile(
            FileSnapshot(path, workingFile->getLines()));
        return;
    }

    if (currentDirectory.findDirectory(fileName) != nullptr)
    {
        throw std::string("Cannot add a directory as a file: ") + fileName;
    }

    const std::vector<FileSnapshot> headSnapshot = getHeadSnapshot();
    const bool stagedBefore = findSnapshot(staging, path) != nullptr;
    const bool trackedInHead = findSnapshot(headSnapshot, path) != nullptr;

    if (!stagedBefore && !trackedInHead)
    {
        throw std::string("File not found: ") + fileName;
    }

    // Absence from staging represents a staged deletion relative to HEAD.
    removeStagedFile(path);
}

// Stage all files and deletions in the current directory subtree
void Repository::addAll()
{
    if (!initialized)
    {
        throw std::string("Repository is not initialized.");
    }

    const std::string currentPath = fileSystem.getCurrentPath();

    if (!isInsideRepository(currentPath))
    {
        throw std::string("Current directory is outside the repository.");
    }

    const Directory *currentDirectory =
        findDirectoryByPath(currentPath);

    if (currentDirectory == nullptr)
    {
        throw std::string("Current directory was not found.");
    }

    const std::string scope = getRelativePath(currentPath);
    std::vector<FileSnapshot> workingFiles;
    collectSnapshot(*currentDirectory, scope, workingFiles);
    sortSnapshots(workingFiles);

    // Add or update every file currently present in this subtree.
    for (std::size_t i = 0; i < workingFiles.size(); i++)
    {
        replaceStagedFile(workingFiles[i]);
    }

    // Any previously staged file missing from this subtree is a deletion.
    for (std::size_t i = staging.size(); i > 0; i--)
    {
        const std::size_t index = i - 1;
        const std::string path = staging[index].path;

        if (isPathInsideScope(path, scope) &&
            findSnapshot(workingFiles, path) == nullptr)
        {
            staging.erase(staging.begin() + index);
        }
    }

    sortSnapshots(staging);
}

// Restore one staging entry to its HEAD version
void Repository::reset(const std::string &fileName)
{
    if (!initialized)
    {
        throw std::string("Repository is not initialized.");
    }

    const std::string path = getFilePath(fileName);
    const std::vector<FileSnapshot> headSnapshot = getHeadSnapshot();
    const FileSnapshot *headFile = findSnapshot(headSnapshot, path);

    if (headFile == nullptr)
    {
        removeStagedFile(path);
    }
    else
    {
        replaceStagedFile(*headFile);
    }
}

// Create a commit from the complete staging snapshot
void Repository::commit(const std::string &message)
{
    if (!initialized)
    {
        throw std::string("Repository is not initialized.");
    }

    if (userName.empty() || userEmail.empty())
    {
        throw std::string(
            "Please configure user.name and user.email before committing.");
    }

    if (currentBranch.empty())
    {
        throw std::string("Cannot commit while HEAD is detached.");
    }

    Branch *branch = findBranch(currentBranch);

    if (branch == nullptr)
    {
        throw std::string("Current branch does not exist: ") + currentBranch;
    }

    const int parentId = branch->getCommitId();
    const std::vector<FileSnapshot> headSnapshot = getHeadSnapshot();

    if (snapshotsEqual(staging, headSnapshot))
    {
        const std::vector<FileSnapshot> workingSnapshot =
            getCurrentSnapshot();

        if (snapshotsEqual(workingSnapshot, headSnapshot))
        {
            std::cout << "nothing to commit, working tree clean" << std::endl;
        }
        else
        {
            std::cout << "no changes added to commit" << std::endl;
        }

        return;
    }

    const std::time_t commitTime = std::time(0);

    commits.push_back(
        Commit(
            nextCommitId,
            parentId,
            0,
            message,
            userName,
            userEmail,
            commitTime,
            staging));

    branch->setCommitId(nextCommitId);
    staging = commits.back().getSnapshot();
    nextCommitId++;

    std::cout << "[" << currentBranch << " "
              << commits.back().getId() << "] "
              << message << std::endl;
}

// Display staged, unstaged, and untracked changes
void Repository::status() const
{
    if (!initialized)
    {
        throw std::string("Repository is not initialized.");
    }

    if (!isInsideRepository(fileSystem.getCurrentPath()))
    {
        throw std::string("Current directory is outside the repository.");
    }

    const std::vector<FileSnapshot> headSnapshot = getHeadSnapshot();
    const std::vector<FileSnapshot> workingSnapshot = getCurrentSnapshot();

    std::vector<std::string> stagedNewFiles;
    std::vector<std::string> stagedModifiedFiles;
    std::vector<std::string> stagedDeletedFiles;
    std::vector<std::string> unstagedModifiedFiles;
    std::vector<std::string> unstagedDeletedFiles;
    std::vector<std::string> untrackedFiles;

    // Compare staging against HEAD.
    for (std::size_t i = 0; i < staging.size(); i++)
    {
        const FileSnapshot *headFile =
            findSnapshot(headSnapshot, staging[i].path);

        if (headFile == nullptr)
        {
            stagedNewFiles.push_back(staging[i].path);
        }
        else if (headFile->lines != staging[i].lines)
        {
            stagedModifiedFiles.push_back(staging[i].path);
        }
    }

    for (std::size_t i = 0; i < headSnapshot.size(); i++)
    {
        if (findSnapshot(staging, headSnapshot[i].path) == nullptr)
        {
            stagedDeletedFiles.push_back(headSnapshot[i].path);
        }
    }

    // Compare the working tree against the staging snapshot.
    for (std::size_t i = 0; i < staging.size(); i++)
    {
        const FileSnapshot *workingFile =
            findSnapshot(workingSnapshot, staging[i].path);

        if (workingFile == nullptr)
        {
            unstagedDeletedFiles.push_back(staging[i].path);
        }
        else if (workingFile->lines != staging[i].lines)
        {
            unstagedModifiedFiles.push_back(staging[i].path);
        }
    }

    // Per the project contract, files absent from staging are untracked.
    for (std::size_t i = 0; i < workingSnapshot.size(); i++)
    {
        if (findSnapshot(staging, workingSnapshot[i].path) == nullptr)
        {
            untrackedFiles.push_back(workingSnapshot[i].path);
        }
    }

    sortPaths(stagedNewFiles);
    sortPaths(stagedModifiedFiles);
    sortPaths(stagedDeletedFiles);
    sortPaths(unstagedModifiedFiles);
    sortPaths(unstagedDeletedFiles);
    sortPaths(untrackedFiles);

    std::cout << "On branch " << currentBranch << std::endl;

    const bool hasStagedChanges =
        !stagedNewFiles.empty() ||
        !stagedModifiedFiles.empty() ||
        !stagedDeletedFiles.empty();

    if (hasStagedChanges)
    {
        std::cout << "Staged changes:" << std::endl;

        for (std::size_t i = 0; i < stagedNewFiles.size(); i++)
        {
            std::cout << "  new file: " << stagedNewFiles[i] << std::endl;
        }

        for (std::size_t i = 0; i < stagedModifiedFiles.size(); i++)
        {
            std::cout << "  modified: " << stagedModifiedFiles[i] << std::endl;
        }

        for (std::size_t i = 0; i < stagedDeletedFiles.size(); i++)
        {
            std::cout << "  deleted: " << stagedDeletedFiles[i] << std::endl;
        }
    }

    const bool hasUnstagedChanges =
        !unstagedModifiedFiles.empty() ||
        !unstagedDeletedFiles.empty();

    if (hasUnstagedChanges)
    {
        std::cout << "Modified (not staged):" << std::endl;

        for (std::size_t i = 0; i < unstagedModifiedFiles.size(); i++)
        {
            std::cout << "  modified: "
                      << unstagedModifiedFiles[i] << std::endl;
        }

        for (std::size_t i = 0; i < unstagedDeletedFiles.size(); i++)
        {
            std::cout << "  deleted: "
                      << unstagedDeletedFiles[i] << std::endl;
        }
    }

    if (!untrackedFiles.empty())
    {
        std::cout << "Untracked files:" << std::endl;

        for (std::size_t i = 0; i < untrackedFiles.size(); i++)
        {
            std::cout << "  " << untrackedFiles[i] << std::endl;
        }
    }

    if (!hasStagedChanges && !hasUnstagedChanges &&
        untrackedFiles.empty())
    {
        std::cout << "nothing to commit, working tree clean" << std::endl;
    }
}

// Display the first-parent history of the current branch
void Repository::log() const
{
    if (!initialized)
    {
        throw std::string("Repository is not initialized.");
    }

    if (!isInsideRepository(fileSystem.getCurrentPath()))
    {
        throw std::string("Current directory is outside the repository.");
    }

    if (currentBranch.empty())
    {
        throw std::string("HEAD is detached.");
    }

    const Branch *branch = findBranch(currentBranch);

    if (branch == nullptr)
    {
        throw std::string("Current branch does not exist: ") + currentBranch;
    }

    int currentCommitId = branch->getCommitId();

    if (currentCommitId == 0)
    {
        throw std::string("No commits yet.");
    }

    std::size_t visitedCount = 0;

    while (currentCommitId != 0)
    {
        if (visitedCount >= commits.size())
        {
            throw std::string("Invalid commit history: cycle detected.");
        }

        visitedCount++;

        const Commit *currentCommit = findCommit(currentCommitId);

        if (currentCommit == nullptr)
        {
            throw std::string("Commit not found in history: ") +
                  std::to_string(currentCommitId);
        }

        std::cout << "commit " << currentCommit->getId() << std::endl;
        std::cout << "Author: " << currentCommit->getAuthorName()
                  << " <" << currentCommit->getAuthorEmail() << ">"
                  << std::endl;

        std::cout << "Date: ";
        const std::time_t commitTime = currentCommit->getTimestamp();
        const std::tm *localTime = std::localtime(&commitTime);

        if (localTime != nullptr)
        {
            std::cout << std::put_time(
                localTime, "%Y-%m-%d %H:%M:%S");
        }
        else
        {
            std::cout << "unknown";
        }

        std::cout << "\n\n    "
                  << currentCommit->getMessage()
                  << std::endl;

        currentCommitId = currentCommit->getParentId();

        if (currentCommitId != 0)
        {
            std::cout << std::endl;
        }
    }
}

// Getters
bool Repository::isInitialized() const
{
    return initialized;
}

const std::string &Repository::getRootPath() const
{
    return repoRootPath;
}

const std::string &Repository::getCurrentBranch() const
{
    return currentBranch;
}
