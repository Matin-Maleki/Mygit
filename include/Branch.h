#ifndef BRANCH_H
#define BRANCH_H

#include <string>

class Branch
{
private:
    std::string name;
    int commitId;

public:
    Branch(
        const std::string &branchName,
        int targetCommitId);

    const std::string &getName() const;

    int getCommitId() const;

    void setCommitId(int newCommitId);
};

#endif