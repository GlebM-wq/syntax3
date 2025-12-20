#include "stnode.h"

STNode::~STNode() {
    delete left;
    delete right;
}

BinTree::BinTree() : root(nullptr) {}

BinTree::~BinTree() {
    delete root;
}

void BinTree::printBinaryTree(STNode* node, int depth, ostream& out) const {
    if (!node) return;

    out << string(depth * 2, ' ') << node->getData().toString() << '\n';

    printBinaryTree(node->getLeft(), depth + 1, out);

    printBinaryTree(node->getRight(), depth + 1, out);
}

void BinTree::saveTreeToFile(STNode* node, ofstream& file) const {
    if (!node) {
        return;
    }

    file << "(" << node->getData().toString();

    STNode* left = node->getLeft();
    STNode* right = node->getRight();

    if (left) {
        saveTreeToFile(left, file);
    }

    if (right) {
        saveTreeToFile(right, file);
    }

    file << ")";
}

void BinTree::printST() const {
    if (root) {
        printBinaryTree(root, 0, cout);
    }
    else {
        cout << "(empty tree)" << endl;
    }
}

void BinTree::saveSTToFile(const string& filename) const {
    ofstream file(filename);
    if (!file.is_open()) {
        throw runtime_error("Cannot open file: " + filename);
    }
    if (root) {
        saveTreeToFile(root, file);
        file << endl;
    }
    else {
        file << "(empty)" << endl;
    }
    file.close();
}
