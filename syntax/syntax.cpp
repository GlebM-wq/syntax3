#include "token.h"
#include "parser.h" 
#include <iostream>

using namespace std;

int main() {
    try {
        TokenArray tokens = loadTokens("lexer.txt");

        if (tokens.empty()) {
            cerr << "ERROR: No tokens loaded!" << endl;
            return 1;
        }

        Parser parser(tokens);
        parser.parse();
        parser.print();
        parser.saveTreeToFile("syntax_tree.txt");

    }
    catch (const exception& e) {
        cerr << "Error: " << e.what() << endl;
        return 1;
    }

    return 0;
}