#pragma once
#include <string>
#include <fstream>
#include <iostream>
#include <stdexcept>

using namespace std;

struct Token {
    int line;
    string type;
    string value;

    Token() : line(-1), type(""), value("") {}

    Token(int l, const string& t, const string& v)
        : line(l), type(t), value(v) {
    }

    string toString() const {
        return "Line " + to_string(line) + ": " + type + " '" + value + "'";
    }
};

class StringArray {
private:
    string* data;
    int capacity;
    int length;

    void resize(int newCapacity) {
        if (newCapacity == 0) newCapacity = 1;

        string* newData = new string[newCapacity];
        int copyCount = (length < newCapacity) ? length : newCapacity;

        for (int i = 0; i < copyCount; i++) {
            newData[i] = data[i];
        }

        delete[] data;
        data = newData;
        capacity = newCapacity;

        if (length > capacity) {
            length = capacity; 
        }
    }

public:
    StringArray() : data(nullptr), capacity(0), length(0) {
        resize(10);
    }

    StringArray(const StringArray& other) : data(nullptr), capacity(0), length(0) {
        resize(other.capacity);
        length = other.length;
        for (int i = 0; i < length; i++) {
            data[i] = other.data[i];
        }
    }

    ~StringArray() {
        delete[] data;
    }

    StringArray& operator=(const StringArray& other) {
        if (this != &other) {
            delete[] data;
            data = nullptr;
            capacity = 0;
            length = 0;
            resize(other.capacity);
            length = other.length;
            for (int i = 0; i < length; i++) {
                data[i] = other.data[i];
            }
        }
        return *this;
    }

    void push_back(const string& str) {
        if (length >= capacity) {
            resize(capacity * 2);
        }
        data[length] = str;
        length++;
    }

    string& operator[](int index) {
        if (index >= length) {
            throw out_of_range("StringArray index out of range");
        }
        return data[index];
    }

    const string& operator[](int index) const {
        if (index >= length) {
            throw out_of_range("StringArray index out of range");
        }
        return data[index];
    }

    int size() const {
        return length;
    }

    bool empty() const {
        return length == 0;
    }

    void clear() {
        length = 0;
    }
};

class TokenArray {
private:
    Token* data;
    int capacity;
    int length;

    void resize(int newCapacity) {
        if (newCapacity == 0) newCapacity = 1; 

        Token* newData = new Token[newCapacity];
        int copyCount = (length < newCapacity) ? length : newCapacity;

        for (int i = 0; i < copyCount; i++) {
            newData[i] = data[i];
        }

        delete[] data;
        data = newData;
        capacity = newCapacity;

        if (length > capacity) {
            length = capacity;
        }
    }

public:
    TokenArray() : data(nullptr), capacity(0), length(0) {
        resize(10);
    }

    TokenArray(const TokenArray& other) : data(nullptr), capacity(0), length(0) {
        resize(other.capacity);
        length = other.length;
        for (int i = 0; i < length; i++) {
            data[i] = other.data[i];
        }
    }

    ~TokenArray() {
        delete[] data;
    }

    TokenArray& operator=(const TokenArray& other) {
        if (this != &other) {
            delete[] data;
            data = nullptr;
            capacity = 0;
            length = 0;
            resize(other.capacity);
            length = other.length;
            for (int i = 0; i < length; i++) {
                data[i] = other.data[i];
            }
        }
        return *this;
    }

    void push_back(const Token& token) {
        if (length >= capacity) {
            resize(capacity * 2);
        }
        data[length] = token;
        length++;
    }

    void emplace_back(int line, const string& type, const string& value) {
        push_back(Token(line, type, value));
    }

    Token& operator[](int index) {
        if (index >= length) {
            throw out_of_range("TokenArray index out of range");
        }
        return data[index];
    }

    const Token& operator[](int index) const {
        if (index >= length) {
            throw out_of_range("TokenArray index out of range");
        }
        return data[index];
    }

    Token& at(int index) {
        return operator[](index);
    }

    const Token& at(int index) const {
        return operator[](index);
    }

    int size() const {
        return length;
    }

    bool empty() const {
        return length == 0;
    }

    void clear() {
        length = 0;
    }

    Token* begin() {
        return data;
    }

    Token* end() {
        return data + length;
    }

    const Token* begin() const {
        return data;
    }

    const Token* end() const {
        return data + length;
    }
};

inline bool isNumberString(const string& str) {
    if (str.empty()) return false;

    for (char c : str) {
        if (c < '0' || c > '9') {
            return false;
        }
    }
    return true;
}

inline int stringToInt(const string& str) {
    int result = 0;
    for (char c : str) {
        result = result * 10 + (c - '0');
    }
    return result;
}

inline StringArray splitString(const string& str) {
    StringArray result;
    string current;
    bool inSpace = true;

    for (char c : str) {
        if (c == ' ' || c == '\t') {
            if (!inSpace) {
                if (!current.empty()) {
                    result.push_back(current);
                    current.clear();
                }
                inSpace = true;
            }
        }
        else {
            current += c;
            inSpace = false;
        }
    }

    if (!current.empty()) {
        result.push_back(current);
    }

    return result;
}

inline TokenArray loadTokens(const string& filename) {
    TokenArray tokens;
    ifstream file(filename);
    if (!file.is_open()) {
        throw runtime_error("Cannot open file: " + filename);
    }

    string line;
    while (getline(file, line)) {
        if (line.empty()) continue;

        StringArray parts = splitString(line);
        if (parts.size() < 2) {
            continue;
        }

        int lineNum = 0;
        const string& numStr = parts[0];

        if (isNumberString(numStr)) {
            lineNum = stringToInt(numStr);
        }
        else {
            continue;
        }

        string type = parts[1];
        string value;

        for (int i = 2; i < parts.size(); i++) {
            if (!value.empty()) {
                value += " ";
            }
            value += parts[i];
        }

        tokens.emplace_back(lineNum, type, value);
    }
    file.close();

    cout << "Loaded " << tokens.size() << " tokens" << endl;
    return tokens;
}