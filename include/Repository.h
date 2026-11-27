#ifndef REPOSITORY_H
#define REPOSITORY_H

#include <string>
#include <vector>

#include "FileSystem.h"
#include "FileSnapshot.h"
#include "Commit.h"
#include "Branch.h"

class Repository
{
private:
    // Shared file system
    FileSystem &fileSystem;

    // Repository information
    bool initialized;
    std::string repoRootPath;

    // User information
    std::string userName;
    std::string userEmail;

    // Git data
    std::vector<Commit> commits;
    std::vector<Branch> branches;
    std::vector<FileSnapshot> staging;

    // HEAD information
    std::string currentBranch;

    // Commit ID generator
    int nextCommitId;

private:
    // Path management
    std::vector<std::string> splitPath(
        const std::string &path) const;

    Directory *findDirectoryByPath(
        const std::string &path);

    const Directory *findDirectoryByPath(
        const std::string &path) const;

    std::string getRelativePath(
        const std::string &absolutePath) const;

    std::string getFilePath(
        const std::string &fileName) const;

    bool isInsideRepository(
        const std::string &path) const;

    // Snapshot management
    void collectSnapshot(
        const Directory &directory,
        const std::string &relativePath,
        std::vector<FileSnapshot> &result) const;

    std::vector<FileSnapshot> getCurrentSnapshot() const;

    std::vector<FileSnapshot> getHeadSnapshot() const;

    const FileSnapshot *findSnapshot(
        const std::vector<FileSnapshot> &files,
        const std::string &path) const;

    FileSnapshot *findSnapshot(
        std::vector<FileSnapshot> &files,
        const std::string &path);

    // Commit and branch management
    const Commit *findCommit(int id) const;

    Commit *findCommit(int id);

    const Branch *findBranch(
        const std::string &name) const;

    Branch *findBranch(
        const std::string &name);

    // Comparison
    bool snapshotsEqual(
        const std::vector<FileSnapshot> &first,
        const std::vector<FileSnapshot> &second) const;

    void replaceStagedFile(
        const FileSnapshot &file);

    void removeStagedFile(
        const std::string &path);

public:
    Repository(FileSystem &fs);

    // User configuration
    void setUserName(const std::string &name);

    void setUserEmail(const std::string &email);

    // Repository initialization
    void init();

    // Staging operations
    void add(const std::string &fileName);

    void addAll();

    void reset(const std::string &fileName);

    // Commit operations
    void commit(const std::string &message);

    // Information commands
    void status() const;

    void log() const;

    // Getters
    bool isInitialized() const;

    const std::string &getRootPath() const;

    const std::string &getCurrentBranch() const;
};

#endif
