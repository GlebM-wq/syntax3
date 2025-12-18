#pragma once
#include "stnode.h"
#include "token.h"
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>

using namespace std;

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

public:
    Parser(const TokenArray& tokens);
    ~Parser();

    Parser(const Parser&) = delete;
    Parser& operator=(const Parser&) = delete;

    void parse();
    BinTree* getST();

    void print() const;
    void saveTreeToFile(const string& filename) const;

private:
    STNode* Program();
    STNode* SingleVarDec();
    STNode* ConstDec();
    STNode* VarDec();
    STNode* FunctionDec();
    STNode* FormalParam();
    STNode* ParamGroup();
    STNode* CompoundState();
    STNode* Stmnt();
    STNode* AssignOrCall();
    STNode* WriteLnStmnt();
    STNode* Expression();
    STNode* SimpleExpr();
    STNode* Term();
    STNode* Factor();
    STNode* ActualParams();
    STNode* Id();
    STNode* IdList(); 
    STNode* Type();
    STNode* Const();
    STNode* Numbers();
    STNode* Name();
};