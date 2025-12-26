#pragma once

#include "AnymUtil.h"
#include "Array.h"

struct TreeNode
{
    I64 value;
    virtual I64 Eval() = 0;
};

struct AddNode : TreeNode
{
    TreeNode* left;
    TreeNode* right;

    I64 Eval() override
};

TreeNode* CreateTreeNode(MemoryArena* arena, const char* name);