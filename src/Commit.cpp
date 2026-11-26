#include "Commit.h"

Commit::Commit(
    int commitId,
    int firstParentId,
    int secondParentId,
    const std::string &commitMessage,
    const std::string &author,
    const std::string &email,
    std::time_t commitTime,
    const std::vector<FileSnapshot> &files)
    : id(commitId),
      parentId(firstParentId),
      secondParentId(secondParentId),
      message(commitMessage),
      authorName(author),
      authorEmail(email),
      timestamp(commitTime),
      snapshot(files)
{
}

int Commit::getId() const
{
    return id;
}

int Commit::getParentId() const
{
    return parentId;
}

int Commit::getSecondParentId() const
{
    return secondParentId;
}

const std::string &Commit::getMessage() const
{
    return message;
}

const std::string &Commit::getAuthorName() const
{
    return authorName;
}

const std::string &Commit::getAuthorEmail() const
{
    return authorEmail;
}

std::time_t Commit::getTimestamp() const
{
    return timestamp;
}

const std::vector<FileSnapshot> &Commit::getSnapshot() const
{
    return snapshot;
}