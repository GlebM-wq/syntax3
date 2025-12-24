#include "stnode.h"

STNode::~STNode() {
    delete left;
    delete right;
}

NodeStack::NodeStack() : data(nullptr), capacity(10), top(-1) {
    data = new STNode * [capacity];
    for (int i = 0; i < capacity; i++) {
        data[i] = nullptr;
    }
}

NodeStack::~NodeStack() {
    clear();
    delete[] data;
}

void NodeStack::resize(int newCapacity) {
    if (newCapacity <= capacity) return;

    STNode** newData = new STNode * [newCapacity];

    for (int i = 0; i <= top; i++) {
        newData[i] = data[i];
    }

    for (int i = top + 1; i < newCapacity; i++) {
        newData[i] = nullptr;
    }

    delete[] data;
    data = newData;
    capacity = newCapacity;
}

void NodeStack::push(STNode* node) {
    if (!node) return;

    if (top + 1 >= capacity) {
        resize(capacity * 2);
    }

    top++;
    data[top] = node;
}

STNode* NodeStack::pop() {
    if (isEmpty()) {
        return nullptr;
    }

    STNode* node = data[top];
    data[top] = nullptr;
    top--;

    return node;
}

STNode* NodeStack::peek() const {
    if (isEmpty()) {
        return nullptr;
    }
    return data[top];
}

bool NodeStack::isEmpty() const {
    return top == -1;
}

int NodeStack::size() const {
    return top + 1;
}

void NodeStack::clear() {
    for (int i = 0; i <= top; i++) {
        data[i] = nullptr;
    }
    top = -1;
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

void BinTree::writeNode(STNode* node, ofstream& file) const {
    if (!node) {
        return;
    }

    file << "(" << node->getData().toString();

    STNode* left = node->getLeft();
    STNode* right = node->getRight();

    if (left) {
        writeNode(left, file);
    }

    if (right) {
        writeNode(right, file);
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

void BinTree::saveToFile(const string& filename) const {
    ofstream file(filename);
    if (!file.is_open()) {
        throw runtime_error("Cannot open file: " + filename);
    }
    if (root) {
        writeNode(root, file);
        file << endl;
    }
    else {
        file << "(empty)" << endl;
    }
    file.close();
}
