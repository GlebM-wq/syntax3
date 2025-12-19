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

bool Parser::isVariableDeclaration() {
    int save = current;
    if (!match(ID)) {
        return false;
    }
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

STNode* Parser::Program() {
    STNode* progName = nullptr;
    if (match(KEYWORD, "program")) {
        consume(KEYWORD, "program");
        progName = Name();
        consume(SEP, ";");
    }

    STNode* decls = nullptr;
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
            break;
        }
        decls = makeSeq(decls, decl);
    }

    consume(KEYWORD, "begin");

    STNode* stmts = nullptr;
    while (!match(KEYWORD, "end") && !match(SEP, ".")) {
        if (match(SEP, ";")) {
            advance();
            continue;
        }
        STNode* stmt = Stmnt();
        if (stmt) {
            stmts = makeSeq(stmts, stmt);
        }
    }

    if (match(KEYWORD, "end")) {
        consume(KEYWORD, "end");
    }
    consume(SEP, ".");

    STNode* programNode = createNode("PROGRAM", "");
    programNode->setLeft(progName);
    programNode->setRight(makeSeq(decls, stmts));
    return programNode;
}

STNode* Parser::SingleVarDec() {
    STNode* idNode = Id();
    consume(SEP, ":");
    consume(KEYWORD, "integer");
    consume(SEP, ";");

    STNode* varDecl = createNode("VAR_DECL", "");
    varDecl->setLeft(idNode);
    return varDecl;
}

STNode* Parser::ConstDec() {
    consume(KEYWORD, "const");
    STNode* result = nullptr;

    while (match(ID)) {
        STNode* idNode = Id();
        consume(SEP, "=");
        STNode* valueNode = Const();
        consume(SEP, ";");

        STNode* constDecl = createNode("CONST_DECL", "");
        constDecl->setLeft(idNode);
        constDecl->setRight(valueNode);

        result = makeSeq(result, constDecl);
    }
    return result;
}

STNode* Parser::VarDec() {
    consume(KEYWORD, "var");
    STNode* result = nullptr;

    while (match(ID)) {
        STNode* idList = nullptr;
        STNode* lastId = nullptr;

        do {
            STNode* idNode = Id();
            if (!idList) {
                idList = idNode;
            }
            else {
                lastId->setRight(idNode);
            }
            lastId = idNode;
        } while (match(SEP, ",") && (consume(SEP, ","), true));

        consume(SEP, ":");
        consume(KEYWORD, "integer");
        consume(SEP, ";");

        STNode* currentId = idList;
        while (currentId) {
            STNode* next = currentId->getRight();
            currentId->setRight(nullptr);

            STNode* varDecl = createNode("VAR_DECL", "");
            varDecl->setLeft(currentId);
            result = makeSeq(result, varDecl);

            currentId = next;
        }
    }
    return result;
}

STNode* Parser::FunctionDec() {
    if (!match(KEYWORD, "function")) {
        return nullptr;
    }

    consume(KEYWORD, "function");

    STNode* name = Id();
    consume(SEP, "(");

    STNode* params = nullptr;
    if (!match(SEP, ")")) {
        params = ParamList();
    }
    consume(SEP, ")");

    consume(SEP, ":");
    STNode* returnType = Type();

    consume(SEP, ";");

    STNode* body = CompoundState();

    STNode* funcNode = createNode("FUNCTION", "");
    funcNode->setLeft(name);

    STNode* meta = createNode("FUNC_PARAMS", "");
    meta->setLeft(params);
    meta->setRight(returnType);

    funcNode->setRight(makeSeq(meta, body));
    return funcNode;
}

STNode* Parser::ParamList() {
    STNode* result = nullptr;
    do {
        STNode* param = Param();
        result = makeSeq(result, param);
    } while (match(SEP, ";") && (consume(SEP, ";"), true));
    return result;
}

STNode* Parser::Param() {
    bool isVarParam = false;
    if (match(KEYWORD, "var")) {
        consume(KEYWORD, "var");
        isVarParam = true;
    }

    STNode* idList = nullptr;
    STNode* last = nullptr;
    do {
        STNode* id = Id();
        if (!idList) idList = id;
        else last->setRight(id);
        last = id;
    } while (match(SEP, ",") && (consume(SEP, ","), true));

    consume(SEP, ":");
    STNode* typeNode = Type();

    STNode* result = nullptr;
    STNode* current = idList;
    while (current) {
        STNode* next = current->getRight();
        current->setRight(nullptr);

        STNode* paramNode = createNode(isVarParam ? "PARAM_VAR" : "PARAM_VAL", "");
        paramNode->setLeft(current);
        paramNode->setRight(typeNode);

        result = makeSeq(result, paramNode);
        current = next;
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
    else if (match(ID)) {
        return AssignOrCall();
    }
    else if (match(KEYWORD, "begin")) {
        return CompoundState();
    }
    return nullptr;
}

STNode* Parser::AssignOrCall() {
    STNode* identifier = Id();

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
    STNode* writeln = createNode("FUNC_CALL", "writeln");
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
    if (match(SEP, ";")) consume(SEP, ";");
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
    while (match(SEP, "*") || match(KEYWORD, "div")) {
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
    if (match(ID)) {
        STNode* idNode = Id();
        if (match(SEP, "(")) {
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
    STNode* idNode = createNode("ID", currentToken().value);
    consume(ID);
    return idNode;
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
