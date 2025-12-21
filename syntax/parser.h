#pragma once
#include "stnode.h"
#include "token.h"
#include <iostream>
#include <string>
#include <stdexcept>

using namespace std;

extern const int ID;
extern const int HEXNUM;
extern const int DECNUM;
extern const int SEP;
extern const int KEYWORD;

class Parser {
private:
    struct Scope {
        string* names;
        string* kinds;
        int count;
        int capacity;

        Scope() {
            capacity = 4;
            names = new string[capacity];
            kinds = new string[capacity];
            count = 0;
        }

        ~Scope() {
            delete[] names;
            delete[] kinds;
        }

        void add(const string& name, const string& kind) {
            for (int i = 0; i < count; ++i) {
                if (names[i] == name) {
                    throw runtime_error("Identifier '" + name + "' already declared");
                }
            }

            if (count >= capacity) {
                int newCap = capacity * 2;
                string* newNames = new string[newCap];
                string* newKinds = new string[newCap];

                for (int i = 0; i < count; ++i) {
                    newNames[i] = names[i];
                    newKinds[i] = kinds[i];
                }

                delete[] names;
                delete[] kinds;
                names = newNames;
                kinds = newKinds;
                capacity = newCap;
            }

            names[count] = name;
            kinds[count] = kind;
            count++;
        }

        string getKind(const string& name) const {
            for (int i = 0; i < count; ++i) {
                if (names[i] == name) {
                    return kinds[i];
                }
            }
            return "";
        }

        bool contains(const string& name) const {
            return !getKind(name).empty();
        }
    };

    const TokenArray& tokens;
    int current;
    BinTree* stTree;

    Scope** scopes;
    int scopeCount;
    int scopeCapacity;
    bool inDeclaration;

    Token currentToken() const;
    void advance();
    bool match(int expectedTypeCode, const string& expectedValue = "") const;
    void consume(int expectedTypeCode, const string& expectedValue = "");

    STNode* createNode(const string& type, const string& value = "", int line = -1);
    STNode* makeSeq(STNode* left, STNode* right);

    void enterScope();
    void exitScope();
    void addToCurrentScope(const string& name, const string& kind);
    string getIdentifierKind(const string& name) const;
    bool isDeclaredInScopes(const string& name) const;

    STNode* Program();
    STNode* ConstDec();
    STNode* VarDec();
    STNode* FunctionDec();
    STNode* ParamList();
    STNode* Param();
    STNode* CompoundState();
    STNode* Stmnt();
    STNode* AssignOrCall();
    STNode* WriteLnStmnt();
    STNode* Expression();
    STNode* SimpleExpr();
    STNode* Term();
    STNode* Factor();
    STNode* Id();
    STNode* Type();
    STNode* Numbers();
    STNode* Name();
    STNode* parseDecls();
    STNode* parseStmts();

public:
    Parser(const TokenArray& tokens);
    ~Parser();

    Parser(const Parser&) = delete;
    Parser& operator=(const Parser&) = delete;

    void parse();
    BinTree* getST();

    void print() const;
    void saveTreeToFile(const string& filename) const;
};
