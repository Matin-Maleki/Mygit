#ifndef COMMIT_H
#define COMMIT_H

#include <string>
#include <vector>
#include <ctime>

#include "FileSnapshot.h"

class Commit
{
private:
    int id;

    int parentId;
    int secondParentId;

    std::string message;
    std::string authorName;
    std::string authorEmail;

    std::time_t timestamp;

    std::vector<FileSnapshot> snapshot;

public:
    Commit(
        int commitId,
        int firstParentId,
        int secondParentId,
        const std::string &commitMessage,
        const std::string &author,
        const std::string &email,
        std::time_t commitTime,
        const std::vector<FileSnapshot> &files);

    int getId() const;

    int getParentId() const;

    int getSecondParentId() const;

    const std::string &getMessage() const;

    const std::string &getAuthorName() const;

    const std::string &getAuthorEmail() const;

    std::time_t getTimestamp() const;

    const std::vector<FileSnapshot> &getSnapshot() const;
};

#endif