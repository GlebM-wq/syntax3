#pragma once
#include <iostream>
#include <string>
#include <fstream>

using namespace std;

struct STData {
    string type;
    string value;
    int line;

    STData(const string& t = "", const string& v = "", int l = 0)
        : type(t), value(v), line(l) {
    }

    string toString() const {
        if (value.empty() || value == type) {
            return type;
        }
        return type + ":" + value;
    }
};

class STNode {
private:
    STData data;
    STNode* left; 
    STNode* right;

public:
    STNode(const STData& value) : data(value), left(nullptr), right(nullptr) {}
    ~STNode();

    const STData& getData() const { return data; }
    STNode* getLeft() const { return left; }
    STNode* getRight() const { return right; }

    void setLeft(STNode* node) { left = node; }
    void setRight(STNode* node) { right = node; }

    bool isLeaf() const { return !left && !right; }
};

class BinTree {
private:
    STNode* root;

    void printBinaryTree(STNode* node, int depth, ostream& out) const;
    void saveTreeToFile(STNode* node, ofstream& file) const;
    STNode* copyTree(STNode* otherNode);

public:
    BinTree();
    ~BinTree();

    void setRoot(STNode* node) { root = node; }
    STNode* getRoot() const { return root; }
    bool isEmpty() const { return root == nullptr; }

    void printST() const;
    void saveSTToFile(const string& filename) const;
};