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

class TokenArray {
private:
    Token* data;
    int capacity;
    int length;

    void resize(int newCapacity) {
        if (newCapacity <= 0) newCapacity = 1;
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
        if (index < 0 || index >= length) {
            throw out_of_range("TokenArray index out of range");
        }
        return data[index];
    }

    const Token& operator[](int index) const {
        if (index < 0 || index >= length) {
            throw out_of_range("TokenArray index out of range");
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

inline int tokenTypeCode(const string& typeStr) {
    if (typeStr == "ID") return 0;
    if (typeStr == "HEXNUM") return 1;
    if (typeStr == "DECNUM") return 2;
    if (typeStr == "SEP") return 3;
    if (typeStr == "KEYWORD") return 4;
    return -1;
}

inline bool isNumberString(const string& str) {
    if (str.empty()) return false;
    for (int i = 0; i < (int)str.length(); i++) {
        if (str[i] < '0' || str[i] > '9') return false;
    }
    return true;
}

inline int stringToInt(const string& str) {
    int result = 0;
    for (int i = 0; i < (int)str.length(); i++) {
        result = result * 10 + (str[i] - '0');
    }
    return result;
}

inline bool parseTokenLine(const string& line, int& outLine, string& outType, string& outValue) {
    if (line.empty()) return false;

    int pos = 0;
    while (pos < line.length() && line[pos] == ' ') pos++;
    if (pos == line.length()) return false;

    int pos1 = line.find(' ', pos);
    if (pos1 == string::npos) return false;
    string linePart = line.substr(pos, pos1 - pos);
    if (!isNumberString(linePart)) return false;
    outLine = stringToInt(linePart);

    pos = pos1 + 1;
    while (pos < line.length() && line[pos] == ' ') pos++;
    if (pos >= line.length()) return false;

    int pos2 = line.find(' ', pos);
    string typeCodeStr = (pos2 == string::npos) ? line.substr(pos) : line.substr(pos, pos2 - pos);
    if (!isNumberString(typeCodeStr)) return false;
    int typeCode = stringToInt(typeCodeStr);

    switch (typeCode) {
    case 0: outType = "ID"; break;
    case 1: outType = "HEXNUM"; break;
    case 2: outType = "DECNUM"; break;
    case 3: outType = "SEP"; break;
    case 4: outType = "KEYWORD"; break;
    default: return false;
    }

    if (pos2 == string::npos) {
        outValue = "";
    }
    else {
        int valStart = pos2 + 1;
        while (valStart < line.length() && line[valStart] == ' ') valStart;
        if (valStart >= line.length()) {
            outValue = "";
        }
        else {
            outValue = line.substr(valStart);
        }
    }

    return true;
}

inline TokenArray loadTokens(const string& filename) {
    TokenArray tokens;
    ifstream file(filename);
    if (!file.is_open()) {
        throw runtime_error("Cannot open file: " + filename);
    }
    string line;
    int count = 0;
    while (getline(file, line)) {
        if (line.empty()) continue;
        int lineNum;
        string type, value;
        if (parseTokenLine(line, lineNum, type, value)) {
            tokens.emplace_back(lineNum, type, value);
            count++;
        }
    }
    file.close();
    cout << "Loaded " << count << " tokens" << endl;
    return tokens;
}
