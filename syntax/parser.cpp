#include "parser.h"

const int ID = 0;
const int HEXNUM = 1;
const int DECNUM = 2;
const int SEP = 3;
const int KEYWORD = 4;

Token Parser::currentToken() const {
    return current < tokens.size() ? tokens[current] : Token(-1, "", "");
}

void Parser::advance() {
    if (current < tokens.size()) current++;
}

bool Parser::match(int expectedTypeCode, const string& expectedValue) const {
    if (current >= tokens.size()) return false;

    const Token& token = tokens[current];

    string expectedTypeStr = to_string(expectedTypeCode);
    if (token.type != expectedTypeStr) {
        return false;
    }

    if (!expectedValue.empty() && token.value != expectedValue) {
        return false;
    }

    return true;
}

void Parser::consume(int expectedTypeCode, const string& expectedValue) {
    if (!match(expectedTypeCode, expectedValue)) {
        string typeStr;
        switch (expectedTypeCode) {
        case ID: typeStr = "identifier"; break;
        case HEXNUM: typeStr = "hex number"; break;
        case DECNUM: typeStr = "decimal number"; break;
        case SEP: typeStr = "separator"; break;
        case KEYWORD: typeStr = "keyword"; break;
        default: typeStr = "token type " + to_string(expectedTypeCode);
        }

        string error = "Syntax error: expected " + typeStr;
        if (!expectedValue.empty()) error += " '" + expectedValue + "'";
        error += ", but found ";

        if (current < tokens.size()) {
            const Token& token = tokens[current];
            error += "token type " + token.type;
            if (!token.value.empty()) error += " value '" + token.value + "'";
        }
        else {
            error += "EOF";
        }

        throw runtime_error(error);
    }
    advance();
}

bool Parser::isVariableDeclaration() {
    int save = current;

    if (!match(ID) && !match(KEYWORD, "result")) return false;
    advance();

    if (!match(SEP, ":")) {
        current = save;
        return false;
    }
    advance();

    if (!match(KEYWORD, "integer")) {
        current = save;
        return false;
    }
    advance();

    if (!match(SEP, ";")) {
        current = save;
        return false;
    }

    current = save;
    return true;
}

Parser::Parser(const TokenArray& tokenArray) : tokens(tokenArray), current(0), stTree(new BinTree()) {}

Parser::~Parser() {
    delete stTree;
}

void Parser::parse() {
    try {
        STNode* rootNode = Program();
        stTree->setRoot(rootNode);
        cout << "Parsing completed successfully!" << endl;
    }
    catch (const exception& e) {
        delete stTree;
        stTree = nullptr;
        throw runtime_error(string("Parsing failed: ") + e.what());
    }
}

BinTree* Parser::getST() {
    return stTree;
}

void Parser::print() const {
    if (stTree) {
        stTree->printST();
    }
    else {
        cout << "Syntax Tree is empty" << endl;
    }
}

void Parser::saveTreeToFile(const string& filename) const {
    if (stTree) {
        stTree->saveSTToFile(filename);
        cout << "Syntax tree saved to '" << filename << "'" << endl;
    }
}

STNode* Parser::createNode(const string& type, const string& value, int line) {
    if (line == -1 && current < tokens.size()) {
        line = tokens[current].line;
    }
    return new STNode(STData(type, value, line));
}

STNode* Parser::Program() {
    STNode* programNode = createNode("PROGRAM", "");

    if (match(KEYWORD, "program")) {
        consume(KEYWORD, "program");
        STNode* progName = Name();
        programNode->setLeft(progName);
        consume(SEP, ";");
    }

    STNode* firstDecl = nullptr;
    STNode* lastDecl = nullptr;
    
    while (!match(KEYWORD, "begin")) {
        STNode* decl = nullptr;

        if (match(KEYWORD, "const")) {
            decl = ConstDec();
        }
        else if (match(KEYWORD, "var")) {
            decl = VarDec();
        }
        else if (match(KEYWORD, "function")) {
            decl = FunctionDec();
        }
        else if (isVariableDeclaration()) {
            decl = SingleVarDec();
        }
        else {
            throw runtime_error("Unexpected token in declaration section");
        }

        if (decl) {
            if (!firstDecl) {
                firstDecl = decl;
                lastDecl = decl;
            } else {
                lastDecl->setRight(decl);
                lastDecl = decl;
            }
        }
    }

    consume(KEYWORD, "begin");
    
    STNode* firstStmt = nullptr;
    STNode* lastStmt = nullptr;
    
    while (!match(KEYWORD, "end") && !match(SEP, ".")) {
        if (match(SEP, ";")) {
            consume(SEP, ";");
            continue;
        }
        
        STNode* stmt = Stmnt();
        if (stmt) {
            if (!firstStmt) {
                firstStmt = stmt;
                lastStmt = stmt;
            } else {
                lastStmt->setRight(stmt);
                lastStmt = stmt;
            }
        } else {
            break;
        }
    }

    if (match(KEYWORD, "end")) {
        consume(KEYWORD, "end");
    }
    consume(SEP, ".");

    if (firstDecl) {
        programNode->setLeft(firstDecl);
    }

    if (firstStmt) {
        programNode->setRight(firstStmt);
    }

    return programNode;
}

STNode* Parser::SingleVarDec() {
    STNode* idNode;
    if (match(KEYWORD, "result")) {
        idNode = createNode("KEYWORD", "result");
        consume(KEYWORD, "result");
    }
    else {
        idNode = Id();
    }

    consume(SEP, ":");
    STNode* typeNode = Type();
    consume(SEP, ";");

    STNode* varDecl = createNode("VAR_DECL_SINGLE", "");
    varDecl->setLeft(idNode);
    varDecl->setRight(typeNode);
    
    return varDecl;
}

STNode* Parser::ConstDec() {
    STNode* constDecl = createNode("CONST_DECL", "");
    consume(KEYWORD, "const");
    
    STNode* firstConst = nullptr;
    STNode* lastConst = nullptr;
    
    while (match(ID)) {
        STNode* constItem = createNode("CONST_ITEM", "");
        
        STNode* idNode = Id();
        constItem->setLeft(idNode);
        
        consume(SEP, "=");
        STNode* constVal = Const();
        constItem->setRight(constVal);
        
        consume(SEP, ";");
        
        if (!firstConst) {
            firstConst = constItem;
            lastConst = constItem;
        } else {
            lastConst->setRight(constItem);
            lastConst = constItem;
        }
    }
    
    if (firstConst) {
        constDecl->setLeft(firstConst);
    }
    
    return constDecl;
}

STNode* Parser::VarDec() {
    STNode* varDecl = createNode("VAR_DECL", "");
    consume(KEYWORD, "var");
    
    STNode* firstGroup = nullptr;
    STNode* lastGroup = nullptr;
    
    while (!match(KEYWORD, "begin") && (match(ID) || match(KEYWORD, "result"))) {
        STNode* varGroup = createNode("VAR_GROUP", "");

        STNode* firstId = nullptr;
        STNode* lastId = nullptr;
        
        do {
            STNode* idNode;
            if (match(KEYWORD, "result")) {
                idNode = createNode("KEYWORD", "result");
                consume(KEYWORD, "result");
            }
            else {
                idNode = Id();
            }
            
            if (!firstId) {
                firstId = idNode;
                lastId = idNode;
            } else {
                lastId->setRight(idNode);
                lastId = idNode;
            }
            
        } while (match(SEP, ",") && (consume(SEP, ","), true));
        
        consume(SEP, ":");
        STNode* typeNode = Type();
        consume(SEP, ";");
        
        varGroup->setLeft(firstId);
        varGroup->setRight(typeNode);
        
        if (!firstGroup) {
            firstGroup = varGroup;
            lastGroup = varGroup;
        } else {
            lastGroup->setRight(varGroup);
            lastGroup = varGroup;
        }
    }

    if (firstGroup) {
        varDecl->setLeft(firstGroup);
    }
    
    return varDecl;
}

STNode* Parser::FunctionDec() {
    STNode* funcDecl = createNode("FUNCTION_DECL", "");
    consume(KEYWORD, "function");

    STNode* funcName = Id();
    funcDecl->setLeft(funcName);
    
    consume(SEP, "(");
    
    STNode* firstParam = nullptr;
    STNode* lastParam = nullptr;
    
    if (!match(SEP, ")")) {
        STNode* param = ParamGroup();
        firstParam = param;
        lastParam = param;
        
        while (match(SEP, ";")) {
            consume(SEP, ";");
            param = ParamGroup();
            lastParam->setRight(param);
            lastParam = param;
        }
    }
    
    consume(SEP, ")");

    consume(SEP, ":");
    STNode* returnType = Type();
    consume(SEP, ";");

    if (firstParam) {
        funcName->setRight(firstParam);
        lastParam->setRight(returnType);
    } else {
        funcName->setRight(returnType);
    }

    STNode* localVars = nullptr;
    STNode* lastLocalVar = nullptr;
    
    while (match(KEYWORD, "var")) {
        STNode* varDecl = VarDec();
        if (!localVars) {
            localVars = varDecl;
            lastLocalVar = varDecl;
        } else {
            lastLocalVar->setRight(varDecl);
            lastLocalVar = varDecl;
        }
    }
    
    consume(KEYWORD, "begin");
    
    STNode* firstStmt = nullptr;
    STNode* lastStmt = nullptr;
    
    while (!match(KEYWORD, "end")) {
        if (match(SEP, ";")) {
            consume(SEP, ";");
            continue;
        }
        
        STNode* stmt = Stmnt();
        if (stmt) {
            if (!firstStmt) {
                firstStmt = stmt;
                lastStmt = stmt;
            } else {
                lastStmt->setRight(stmt);
                lastStmt = stmt;
            }
        } else {
            break;
        }
    }
    
    consume(KEYWORD, "end");
    consume(SEP, ";");
    
    if (localVars) {
        localVars->setRight(firstStmt);
        funcDecl->setRight(localVars);
    } else if (firstStmt) {
        funcDecl->setRight(firstStmt);
    }
    
    return funcDecl;
}

STNode* Parser::FormalParam() {
    return ParamGroup();
}

STNode* Parser::ParamGroup() {
    STNode* paramGroup = createNode("PARAM_GROUP", "");

    STNode* modifier = nullptr;
    if (match(KEYWORD, "var") || match(KEYWORD, "const")) {
        modifier = createNode("MODIFIER", currentToken().value);
        consume(KEYWORD);
        paramGroup->setLeft(modifier);
    }
    
    STNode* firstId = nullptr;
    STNode* lastId = nullptr;
    
    STNode* id = Id();
    firstId = id;
    lastId = id;
    
    while (match(SEP, ",")) {
        consume(SEP, ",");
        id = Id();
        lastId->setRight(id);
        lastId = id;
    }
    
    consume(SEP, ":");
    STNode* typeNode = Type();

    if (modifier) {
        modifier->setRight(firstId);
        paramGroup->setRight(typeNode);
    } else {
        paramGroup->setLeft(firstId);
        paramGroup->setRight(typeNode);
    }
    
    return paramGroup;
}

STNode* Parser::CompoundState() {
    STNode* compound = createNode("COMPOUND_STMT", "");
    consume(KEYWORD, "begin");
    
    STNode* firstStmt = nullptr;
    STNode* lastStmt = nullptr;
    
    while (!match(KEYWORD, "end")) {
        if (match(SEP, ";")) {
            consume(SEP, ";");
            continue;
        }
        
        STNode* stmt = Stmnt();
        if (stmt) {
            if (!firstStmt) {
                firstStmt = stmt;
                lastStmt = stmt;
            } else {
                lastStmt->setRight(stmt);
                lastStmt = stmt;
            }
        } else {
            break;
        }
    }
    
    consume(KEYWORD, "end");
    
    if (match(SEP, ";")) {
        consume(SEP, ";");
    }
    
    if (firstStmt) {
        compound->setLeft(firstStmt);
    }
    
    return compound;
}

STNode* Parser::Stmnt() {
    if (current >= tokens.size()) return nullptr;

    if (match(KEYWORD, "writeln")) {
        return WriteLnStmnt();
    }
    else if (match(ID) || match(KEYWORD, "result")) {
        return AssignOrCall();
    }
    else if (match(KEYWORD, "begin")) {
        return CompoundState();
    }

    return nullptr;
}

STNode* Parser::AssignOrCall() {
    STNode* identifier;

    if (match(KEYWORD, "result")) {
        identifier = createNode("KEYWORD", "result");
        consume(KEYWORD, "result");
    }
    else {
        identifier = Id();
    }

    if (match(SEP, ":=")) {
        consume(SEP, ":=");
        STNode* expr = Expression();
        
        STNode* assignNode = createNode("ASSIGN", ":=");
        assignNode->setLeft(identifier);
        assignNode->setRight(expr);
        
        return assignNode;
    }
    else if (match(SEP, "(")) {
        consume(SEP, "(");
        
        STNode* firstArg = nullptr;
        STNode* lastArg = nullptr;
        
        if (!match(SEP, ")")) {
            STNode* arg;
            if (match(KEYWORD, "result")) {
                arg = createNode("KEYWORD", "result");
                consume(KEYWORD, "result");
            }
            else {
                arg = Expression();
            }
            
            firstArg = arg;
            lastArg = arg;
            
            while (match(SEP, ",")) {
                consume(SEP, ",");
                
                if (match(KEYWORD, "result")) {
                    arg = createNode("KEYWORD", "result");
                    consume(KEYWORD, "result");
                }
                else {
                    arg = Expression();
                }
                
                lastArg->setRight(arg);
                lastArg = arg;
            }
        }
        
        consume(SEP, ")");
        
        STNode* callNode = createNode("FUNC_CALL", "");
        callNode->setLeft(identifier);
        
        if (firstArg) {
            callNode->setRight(firstArg);
        }
        
        return callNode;
    }
    else {
        throw runtime_error("Expected ':=' or '(' after identifier");
    }
}

STNode* Parser::WriteLnStmnt() {
    STNode* writeln = createNode("WRITELN", "");
    consume(KEYWORD, "writeln");
    
    consume(SEP, "(");
    
    STNode* firstArg = nullptr;
    STNode* lastArg = nullptr;
    
    if (!match(SEP, ")")) {
        STNode* arg;
        if (match(KEYWORD, "result")) {
            arg = createNode("KEYWORD", "result");
            consume(KEYWORD, "result");
        }
        else {
            arg = Expression();
        }
        
        firstArg = arg;
        lastArg = arg;
        
        while (match(SEP, ",")) {
            consume(SEP, ",");
            
            if (match(KEYWORD, "result")) {
                arg = createNode("KEYWORD", "result");
                consume(KEYWORD, "result");
            }
            else {
                arg = Expression();
            }
            
            lastArg->setRight(arg);
            lastArg = arg;
        }
    }
    
    consume(SEP, ")");
    
    if (match(SEP, ";")) {
        consume(SEP, ";");
    }
    
    if (firstArg) {
        writeln->setLeft(firstArg);
    }
    
    return writeln;
}

STNode* Parser::Expression() {
    return SimpleExpr();
}

STNode* Parser::SimpleExpr() {
    STNode* left = Term();
    
    while (match(SEP, "+") || match(SEP, "-")) {
        string op = currentToken().value;
        advance();
        
        STNode* right = Term();
        
        STNode* binOp = createNode("BIN_OP", op);
        binOp->setLeft(left);
        binOp->setRight(right);
        
        left = binOp;
    }
    
    return left;
}

STNode* Parser::Term() {
    STNode* left = Factor();
    
    while (match(SEP, "*") || match(ID, "div")) {
        string op = currentToken().value;
        advance();
        
        STNode* right = Factor();
        
        STNode* binOp = createNode("BIN_OP", op);
        binOp->setLeft(left);
        binOp->setRight(right);
        
        left = binOp;
    }
    
    return left;
}

STNode* Parser::Factor() {
    if (match(KEYWORD, "result")) {
        STNode* resultNode = createNode("KEYWORD", "result");
        consume(KEYWORD, "result");
        return resultNode;
    }
    else if (match(ID)) {
        STNode* idNode = Id();
        
        if (match(SEP, "(")) {
            // Вызов функции в выражении
            consume(SEP, "(");
            
            STNode* firstArg = nullptr;
            STNode* lastArg = nullptr;
            
            if (!match(SEP, ")")) {
                STNode* arg = Expression();
                firstArg = arg;
                lastArg = arg;
                
                while (match(SEP, ",")) {
                    consume(SEP, ",");
                    arg = Expression();
                    lastArg->setRight(arg);
                    lastArg = arg;
                }
            }
            
            consume(SEP, ")");
            
            STNode* callNode = createNode("FUNC_CALL", "");
            callNode->setLeft(idNode);
            
            if (firstArg) {
                callNode->setRight(firstArg);
            }
            
            return callNode;
        }
        
        return idNode;
    }
    else if (match(DECNUM) || match(HEXNUM)) {
        return Numbers();
    }
    else if (match(SEP, "(")) {
        consume(SEP, "(");
        STNode* expr = Expression();
        consume(SEP, ")");
        return expr;
    }
    
    throw runtime_error("Expected factor");
}

STNode* Parser::ActualParams() {
    STNode* argsNode = createNode("ARGUMENTS", "");
    
    STNode* firstArg = nullptr;
    STNode* lastArg = nullptr;
    
    if (!match(SEP, ")")) {
        STNode* arg;
        if (match(KEYWORD, "result")) {
            arg = createNode("KEYWORD", "result");
            consume(KEYWORD, "result");
        }
        else {
            arg = Expression();
        }
        
        firstArg = arg;
        lastArg = arg;
        
        while (match(SEP, ",")) {
            consume(SEP, ",");
            
            if (match(KEYWORD, "result")) {
                arg = createNode("KEYWORD", "result");
                consume(KEYWORD, "result");
            }
            else {
                arg = Expression();
            }
            
            lastArg->setRight(arg);
            lastArg = arg;
        }
    }
    
    if (firstArg) {
        argsNode->setLeft(firstArg);
        return argsNode;
    }
    
    delete argsNode;
    return nullptr;
}

STNode* Parser::Id() {
    STNode* idNode = createNode("ID", currentToken().value);
    consume(ID);
    return idNode;
}

STNode* Parser::IdList() {
    STNode* idList = createNode("ID_LIST", "");
    STNode* firstId = Id();
    idList->setLeft(firstId);
    
    STNode* currentId = firstId;
    while (match(SEP, ",")) {
        consume(SEP, ",");
        STNode* nextId = Id();
        currentId->setRight(nextId);
        currentId = nextId;
    }

    return idList;
}

STNode* Parser::Type() {
    STNode* typeNode = createNode("TYPE", "integer");
    consume(KEYWORD, "integer");
    return typeNode;
}

STNode* Parser::Const() {
    return Numbers();
}

STNode* Parser::Numbers() {
    if (match(DECNUM)) {
        STNode* numNode = createNode("DECNUM", currentToken().value);
        consume(DECNUM);
        return numNode;
    }
    else if (match(HEXNUM)) {
        STNode* numNode = createNode("HEXNUM", currentToken().value);
        consume(HEXNUM);
        return numNode;
    }
    
    throw runtime_error("Expected number");
}

STNode* Parser::Name() {
    return Id();
}