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
};

class NodeStack {
private:
    STNode** data;
    int capacity;
    int top;

    void resize(int newCapacity);

public:
    NodeStack();
    ~NodeStack();

    void push(STNode* node);
    STNode* pop();
    STNode* peek() const;
    bool isEmpty() const;
    int size() const;
    void clear();
};

class BinTree {
private:
    STNode* root;

    void printBinaryTree(STNode* node, int depth, ostream& out) const;
    void writeNode(STNode* node, ofstream& file) const;

public:
    BinTree();
    ~BinTree();

    void setRoot(STNode* node) { root = node; }
    STNode* getRoot() const { return root; }
    bool isEmpty() const { return root == nullptr; }

    void printST() const;
    void saveToFile(const string& filename) const;
};
