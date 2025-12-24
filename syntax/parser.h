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
            for (int i = 0; i < capacity; i++) {
                names[i] = "";
                kinds[i] = "";
            }
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

    struct FunctionSignature {
        string name;
        int paramCount;
        FunctionSignature* next;

        FunctionSignature(const string& n, int count) : name(n), paramCount(count), next(nullptr) {}
    };

    class FunctionTable {
    private:
        FunctionSignature* head;

    public:
        FunctionTable() : head(nullptr) {}

        ~FunctionTable() {
            FunctionSignature* current = head;
            while (current) {
                FunctionSignature* next = current->next;
                delete current;
                current = next;
            }
        }

        void addFunction(const string& name, int paramCount) {
            FunctionSignature* current = head;
            while (current) {
                if (current->name == name) {
                    throw runtime_error("Function '" + name + "' already declared");
                }
                current = current->next;
            }

            FunctionSignature* newFunc = new FunctionSignature(name, paramCount);
            newFunc->next = head;
            head = newFunc;
        }

        int getParamCount(const string& name) const {
            FunctionSignature* current = head;
            while (current) {
                if (current->name == name) {
                    return current->paramCount;
                }
                current = current->next;
            }
            return -1;
        }
    };

    const TokenArray& tokens;
    int current;
    BinTree* stTree;

    Scope** scopes;
    int scopeCount;
    int scopeCapacity;
    bool inDeclaration;
    FunctionTable* funcTable;

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
    STNode* parseDecls();
    STNode* parseStmts();
    STNode* parseMainBlock();

    int countParams(STNode* paramsNode);
    int countArguments(STNode* argsNode);

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
