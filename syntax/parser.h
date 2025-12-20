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
    const TokenArray& tokens;
    int current;
    BinTree* stTree;

    Token currentToken() const;
    void advance();

    bool match(int expectedTypeCode, const string& expectedValue = "") const;
    void consume(int expectedTypeCode, const string& expectedValue = "");

    bool isVariableDeclaration();
    STNode* createNode(const string& type, const string& value = "", int line = -1);
    STNode* makeSeq(STNode* left, STNode* right);
    STNode* Program();
    STNode* SingleVarDec();
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
    STNode* Const();
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
