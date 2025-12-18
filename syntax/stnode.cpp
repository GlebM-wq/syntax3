#include "stnode.h"

STNode::~STNode() {
    delete left;
    delete right;
}

STNode* BinTree::copyTree(STNode* otherNode) {
    if (!otherNode) return nullptr;

    STNode* newNode = new STNode(otherNode->getData());
    newNode->setLeft(copyTree(otherNode->getLeft()));
    newNode->setRight(copyTree(otherNode->getRight()));
    return newNode;
}

BinTree::BinTree() : root(nullptr) {}

BinTree::~BinTree() {
    delete root;
}

static bool shouldSkipNode(const string& nodeType) {
    if (nodeType == "DECLARATIONS") return true;
    if (nodeType == "STATEMENTS") return true;
    if (nodeType == "CONTENT") return true;
    if (nodeType == "LOCAL_VARS") return true;
    if (nodeType == "FUNC_BODY") return true;
    if (nodeType == "ID_LIST") return true;
    if (nodeType == "PARAMS") return true;
    if (nodeType == "ARGUMENTS") return true;
    if (nodeType == "VAR_LIST") return true;
    if (nodeType == "CONST_LIST") return true;
    if (nodeType == "STMT_LIST") return true;
    return false;
}

void BinTree::printBinaryTree(STNode* node, int depth, ostream& out) const {
    if (!node) return;

    string nodeType = node->getData().type;

    if (shouldSkipNode(nodeType)) {
        if (node->getLeft()) {
            printBinaryTree(node->getLeft(), depth, out);
        }
        if (node->getRight()) {
            printBinaryTree(node->getRight(), depth, out);
        }
        return;
    }

    for (int i = 0; i < depth; i++) {
        out << "    ";
    }

    out << node->getData().toString() << endl;

    if (node->getLeft()) {
        printBinaryTree(node->getLeft(), depth + 1, out);
    }

    if (node->getRight()) {
        printBinaryTree(node->getRight(), depth + 1, out);
    }
}

void BinTree::saveTreeToFile(STNode* node, ofstream& file) const {
    if (!node) {
        file << "()";
        return;
    }

    string nodeType = node->getData().type;

    if (shouldSkipNode(nodeType)) {
        if (node->getLeft()) {
            saveTreeToFile(node->getLeft(), file);
        }
        if (node->getRight()) {
            saveTreeToFile(node->getRight(), file);
        }
        return;
    }

    file << "(" << node->getData().toString();

    if (node->getLeft()) {
        file << " ";
        saveTreeToFile(node->getLeft(), file);
    }

    if (node->getRight()) {
        file << " ";
        saveTreeToFile(node->getRight(), file);
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