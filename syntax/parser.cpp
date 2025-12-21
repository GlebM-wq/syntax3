#include "parser.h"

const int ID = 0;
const int HEXNUM = 1;
const int DECNUM = 2;
const int SEP = 3;
const int KEYWORD = 4;

static STNode* cloneNode(const STNode* node) {
    if (!node) return nullptr;
    STNode* copy = new STNode(node->getData());
    copy->setLeft(cloneNode(node->getLeft()));
    copy->setRight(cloneNode(node->getRight()));
    return copy;
}

void Parser::enterScope() {
    if (scopeCount >= scopeCapacity) {
        int newCap = scopeCapacity * 2;
        Scope** newScopes = new Scope * [newCap];
        for (int i = 0; i < scopeCount; ++i) {
            newScopes[i] = scopes[i];
        }
        delete[] scopes;
        scopes = newScopes;
        scopeCapacity = newCap;
    }
    scopes[scopeCount++] = new Scope();
}

void Parser::exitScope() {
    if (scopeCount <= 1) return;
    delete scopes[--scopeCount];
}

void Parser::addToCurrentScope(const string& name, const string& kind) {
    scopes[scopeCount - 1]->add(name, kind);
}

string Parser::getIdentifierKind(const string& name) const {
    for (int i = scopeCount - 1; i >= 0; --i) {
        string kind = scopes[i]->getKind(name);
        if (!kind.empty()) return kind;
    }
    return "";
}

bool Parser::isDeclaredInScopes(const string& name) const {
    return !getIdentifierKind(name).empty();
}

Token Parser::currentToken() const {
    return current < tokens.size() ? tokens[current] : Token(-1, "", "");
}

void Parser::advance() {
    if (current < tokens.size()) current++;
}

bool Parser::match(int expectedTypeCode, const string& expectedValue) const {
    if (current >= tokens.size()) return false;
    const Token& token = tokens[current];
    int tokenType = tokenTypeCode(token.type);
    if (tokenType != expectedTypeCode) return false;
    if (!expectedValue.empty() && token.value != expectedValue) return false;
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

STNode* Parser::createNode(const string& type, const string& value, int line) {
    if (line == -1 && current < tokens.size()) {
        line = tokens[current].line;
    }
    return new STNode(STData(type, value, line));
}

STNode* Parser::makeSeq(STNode* left, STNode* right) {
    if (!left) return right;
    if (!right) return left;
    STNode* seq = createNode("SEQ", "");
    seq->setLeft(left);
    seq->setRight(right);
    return seq;
}

Parser::Parser(const TokenArray& tokenArray)
    : tokens(tokenArray), current(0), stTree(new BinTree()),
    scopes(nullptr), scopeCount(0), scopeCapacity(0), inDeclaration(false) {
    scopeCapacity = 4;
    scopes = new Scope * [scopeCapacity];
    scopes[scopeCount++] = new Scope();
}

Parser::~Parser() {
    for (int i = 0; i < scopeCount; ++i) {
        delete scopes[i];
    }
    delete[] scopes;
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

STNode* Parser::Program() {
    STNode* progName = nullptr;
    if (match(KEYWORD, "program")) {
        consume(KEYWORD, "program");
        inDeclaration = true;
        progName = Id();
        inDeclaration = false;
        addToCurrentScope(progName->getData().value, "var");
        consume(SEP, ";");
    }

    STNode* body = parseDecls();
    consume(SEP, ".");

    STNode* programNode = createNode("PROGRAM", "");
    programNode->setLeft(progName);
    programNode->setRight(body);
    return programNode;
}

STNode* Parser::ConstDec() {
    consume(KEYWORD, "const");
    STNode* result = nullptr;

    while (match(ID)) {
        inDeclaration = true;
        STNode* idNode = Id();
        inDeclaration = false;
        addToCurrentScope(idNode->getData().value, "const");

        consume(SEP, "=");
        STNode* valueNode = Numbers();
        consume(SEP, ";");

        STNode* constDecl = createNode("CONST_DECL", "");
        constDecl->setLeft(idNode);
        constDecl->setRight(valueNode);

        if (!result) {
            result = constDecl;
        }
        else {
            result = makeSeq(result, constDecl);
        }
    }
    return result;
}

STNode* Parser::VarDec() {
    consume(KEYWORD, "var");
    STNode* result = nullptr;

    while (match(ID)) {
        const int MAX_IDS = 100;
        STNode* ids[MAX_IDS];
        int count = 0;

        do {
            inDeclaration = true;
            STNode* id = Id();
            inDeclaration = false;
            addToCurrentScope(id->getData().value, "var");

            if (count < MAX_IDS) {
                id->setLeft(nullptr);
                id->setRight(nullptr);
                ids[count++] = id;
            }
        } while (match(SEP, ",") && (consume(SEP, ","), true));

        consume(SEP, ":");
        consume(KEYWORD, "integer");
        consume(SEP, ";");

        STNode* currentResult = nullptr;
        for (int i = count - 1; i >= 0; i--) {
            STNode* varDecl = createNode("VAR_DECL", "");
            varDecl->setLeft(cloneNode(ids[i]));
            varDecl->setRight(createNode("TYPE", "INTEGER"));

            if (!currentResult) {
                currentResult = varDecl;
            }
            else {
                currentResult = makeSeq(varDecl, currentResult);
            }
        }

        if (!result) {
            result = currentResult;
        }
        else {
            result = makeSeq(result, currentResult);
        }
    }
    return result;
}

STNode* Parser::FunctionDec() {
    consume(KEYWORD, "function");

    inDeclaration = true;
    STNode* name = Id();
    inDeclaration = false;
    addToCurrentScope(name->getData().value, "func");

    STNode* params = nullptr;
    if (match(SEP, "(")) {
        consume(SEP, "(");
        enterScope();
        if (!match(SEP, ")")) {
            params = ParamList();\
        }
        consume(SEP, ")");
    }
    else {
        enterScope();
    }

    consume(SEP, ":");
    STNode* returnType = Type();
    consume(SEP, ";");

    STNode* localDecls = nullptr;
    while (match(KEYWORD, "var") || match(KEYWORD, "const")) {
        if (match(KEYWORD, "var")) {
            localDecls = makeSeq(localDecls, VarDec());
        }
        else if (match(KEYWORD, "const")) {
            localDecls = makeSeq(localDecls, ConstDec());
        }
    }

    STNode* body = CompoundState();
    exitScope();

    STNode* fullBody = localDecls ? makeSeq(localDecls, body) : body;

    STNode* funcNode = createNode("FUNCTION", "");
    funcNode->setLeft(name);

    STNode* rightPart = nullptr;
    if (params) {
        STNode* typeAndBody = makeSeq(cloneNode(returnType), fullBody);
        rightPart = makeSeq(params, typeAndBody);
    }
    else {
        rightPart = makeSeq(cloneNode(returnType), fullBody);
    }
    funcNode->setRight(rightPart);

    return funcNode;
}

STNode* Parser::ParamList() {
    STNode* first = Param();
    if (match(SEP, ";")) {
        consume(SEP, ";");
        STNode* rest = ParamList();
        STNode* seq = createNode("SEQ", "");
        seq->setLeft(first);
        seq->setRight(rest);
        return seq;
    }
    return first;
}

STNode* Parser::Param() {
    bool isVarParam = false;
    bool isConstParam = false;
    if (match(KEYWORD, "var")) {
        consume(KEYWORD, "var");
        isVarParam = true;
    }
    else if (match(KEYWORD, "const")) {
        consume(KEYWORD, "const");
        isConstParam = true;
    }

    const int MAX = 100;
    STNode* ids[MAX];
    int count = 0;
    do {
        inDeclaration = true;
        STNode* id = Id();
        inDeclaration = false;
        addToCurrentScope(id->getData().value, "var");
        if (count < MAX) {
            id->setLeft(nullptr);
            id->setRight(nullptr);
            ids[count++] = id;
        }
    } while (match(SEP, ",") && (consume(SEP, ","), true));

    consume(SEP, ":");
    STNode* typeNode = Type();

    STNode* result = nullptr;
    for (int i = count - 1; i >= 0; i--) {
        string paramType = isVarParam ? "PARAM_VAR" :
            isConstParam ? "PARAM_CONST" : "PARAM_VAL";
        STNode* paramNode = createNode(paramType, "");
        paramNode->setLeft(cloneNode(ids[i]));
        paramNode->setRight(cloneNode(typeNode));
        if (!result) {
            result = paramNode;
        }
        else {
            result = makeSeq(paramNode, result);
        }
    }
    return result;
}

STNode* Parser::CompoundState() {
    STNode* compound = createNode("COMPOUND_STMT", "");
    consume(KEYWORD, "begin");
    STNode* stmts = nullptr;
    while (!match(KEYWORD, "end")) {
        if (match(SEP, ";")) { advance(); continue; }
        STNode* stmt = Stmnt();
        if (stmt) stmts = makeSeq(stmts, stmt);
    }
    consume(KEYWORD, "end");
    if (match(SEP, ";")) consume(SEP, ";");
    compound->setLeft(stmts);
    return compound;
}

STNode* Parser::Stmnt() {
    if (current >= tokens.size()) return nullptr;
    if (match(KEYWORD, "writeln")) {
        return WriteLnStmnt();
    }
    else if (match(KEYWORD, "begin")) {
        return CompoundState();
    }
    else if (match(ID)) {
        return AssignOrCall();
    }
    return nullptr;
}

STNode* Parser::AssignOrCall() {
    inDeclaration = false;
    STNode* identifier = Id();
    string idName = identifier->getData().value;

    if (match(SEP, ":=")) {
        string kind = getIdentifierKind(idName);
        if (kind == "const") {
            throw runtime_error("Cannot assign to constant '" + idName + "'");
        }

        consume(SEP, ":=");
        STNode* expr = Expression();
        STNode* assignNode = createNode("ASSIGN", ":=");
        assignNode->setLeft(identifier);
        assignNode->setRight(expr);
        return assignNode;
    }
    else if (match(SEP, "(")) {
        consume(SEP, "(");
        STNode* args = nullptr;
        if (!match(SEP, ")")) {
            args = Expression();
            while (match(SEP, ",")) {
                consume(SEP, ",");
                STNode* nextArg = Expression();
                args = makeSeq(args, nextArg);
            }
        }
        consume(SEP, ")");
        STNode* callNode = createNode("FUNC_CALL", "");
        callNode->setLeft(identifier);
        callNode->setRight(args);
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
    STNode* args = nullptr;
    if (!match(SEP, ")")) {
        args = Expression();
        STNode* lastArg = args;
        while (match(SEP, ",")) {
            consume(SEP, ",");
            STNode* nextArg = Expression();
            lastArg->setRight(nextArg);
            lastArg = nextArg;
        }
    }
    consume(SEP, ")");
    if (match(SEP, ";")) {
        consume(SEP, ";");
    }
    writeln->setRight(args);
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
    while (true) {
        if (match(SEP, "*")) {
            consume(SEP, "*");
            STNode* right = Factor();
            STNode* binOp = createNode("BIN_OP", "*");
            binOp->setLeft(left);
            binOp->setRight(right);
            left = binOp;
        }
        else if (match(SEP, "/")) {
            consume(SEP, "/");
            STNode* right = Factor();
            STNode* binOp = createNode("BIN_OP", "/"); 
            binOp->setLeft(left);
            binOp->setRight(right);
            left = binOp;
        }
        else if (match(KEYWORD, "div")) {
            consume(KEYWORD, "div");
            STNode* right = Factor();
            STNode* binOp = createNode("BIN_OP", "div");
            binOp->setLeft(left);
            binOp->setRight(right);
            left = binOp;
        }
        else {
            break;
        }
    }
    return left;
}

STNode* Parser::Factor() {
    if (match(ID)) {
        inDeclaration = false;
        STNode* idNode = Id();

        if (match(SEP, "(")) {
            string idName = idNode->getData().value;
            string kind = getIdentifierKind(idName);
            if (kind != "func") {
                throw runtime_error("Identifier '" + idName + "' is not a function");
            }

            consume(SEP, "(");
            STNode* args = nullptr;
            if (!match(SEP, ")")) {
                args = Expression();
                while (match(SEP, ",")) {
                    consume(SEP, ",");
                    STNode* nextArg = Expression();
                    args = makeSeq(args, nextArg);
                }
            }
            consume(SEP, ")");
            STNode* callNode = createNode("FUNC_CALL", "");
            callNode->setLeft(idNode);
            callNode->setRight(args);
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

STNode* Parser::Id() {
    string idName = currentToken().value;
    consume(ID);

    if (!inDeclaration && !isDeclaredInScopes(idName)) {
        throw runtime_error("Undeclared identifier: '" + idName + "'");
    }

    return createNode("ID", idName);
}

STNode* Parser::Type() {
    STNode* typeNode = createNode("TYPE", "integer");
    consume(KEYWORD, "integer");
    return typeNode;
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

STNode* Parser::parseDecls() {
    const int MAX_DECLS = 200;
    STNode* decls[MAX_DECLS];
    int count = 0;

    auto addDecl = [&](STNode* node) {
        if (!node || count >= MAX_DECLS) return;
        STNode* stack[100];
        int stackTop = 0;
        stack[stackTop++] = node;

        while (stackTop > 0 && count < MAX_DECLS) {
            STNode* current = stack[--stackTop];
            if (!current) continue;

            if (current->getData().type == "SEQ") {
                if (current->getRight() && stackTop < 100) stack[stackTop++] = current->getRight();
                if (current->getLeft() && stackTop < 100) stack[stackTop++] = current->getLeft();
            }
            else {
                decls[count++] = current;
            }
        }
        };

    while (match(KEYWORD, "const") || match(KEYWORD, "var") || match(KEYWORD, "function")) {
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
        if (decl) {
            addDecl(decl);
        }
    }

    STNode* result = nullptr;
    for (int i = count - 1; i >= 0; i--) {
        if (!result) {
            result = decls[i];
        }
        else {
            result = makeSeq(decls[i], result);
        }
    }

    if (match(KEYWORD, "begin")) {
        consume(KEYWORD, "begin");
        STNode* body = parseStmts();
        consume(KEYWORD, "end");
        if (result) {
            result = makeSeq(result, body);
        }
        else {
            result = body;
        }
    }

    return result;
}

STNode* Parser::parseStmts() {
    if (match(KEYWORD, "end") || match(SEP, ".")) {
        return nullptr;
    }
    if (match(SEP, ";")) {
        advance();
        return parseStmts();
    }

    STNode* currentStmt = Stmnt();
    if (!currentStmt) {
        return nullptr;
    }

    if (match(SEP, ";")) {
        advance();
    }

    STNode* rest = parseStmts();
    if (!rest) {
        return currentStmt;
    }

    STNode* seq = createNode("SEQ", "");
    seq->setLeft(currentStmt);
    seq->setRight(rest);
    return seq;
}
