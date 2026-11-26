#include "Branch.h"

Branch::Branch(
    const std::string &branchName,
    int targetCommitId)
    : name(branchName), commitId(targetCommitId)
{
}

const std::string &Branch::getName() const
{
    return name;
}

int Branch::getCommitId() const
{
    return commitId;
}

void Branch::setCommitId(int newCommitId)
{
    commitId = newCommitId;
}