#include <iostream>
#include <cstdio>

using namespace std;


/*
 * Lexer will return token in [0-255](ASCII code) if unknown, otherwise one of enum
 */
enum Token {
    tok_eof = -1,

    tok_def = -2,
    tok_extern = -3,

    tok_identifier = -4,
    tok_number = -5,
};

static string IdentifierStr;    //will hold the token name if it is tok_identifier
static double NumberVal;    //will hold the token number value if it is tok_number

//return the next token from stdin
static int gettok(){
    static int LastChar = ' ';

    //Skip any whitespace
    while (isspace(LastChar)) {
        LastChar = getchar();
    }

    //Identifier: [a-zA-Z][a-zA-Z0-9]*
    if (isalpha(LastChar)) {
        IdentifierStr = LastChar;
        while (isalnum(LastChar = getchar())) {
            IdentifierStr += LastChar;
        }

        if (IdentifierStr == "def") {
            return tok_def;
        }
        if (IdentifierStr == "extern") {
            return tok_extern;
        }

        return tok_identifier;
    }

    //Number: [0-9.]+
    if (isdigit(LastChar) || LastChar == '.') {
        string NumStr;
        do {
            NumStr += LastChar;
            LastChar = getchar();
        } while (isdigit(LastChar) || LastChar == '.');

        NumStr = strtod(NumStr.c_str(), 0);//Just ignore any word after '.'
        return tok_number;
    }

    //Comment: start with '#', ignore any word after # to the end of line
    if (LastChar == '#') {
        do {
            LastChar = getchar();
        } while (LastChar != EOF && LastChar != '\n' && LastChar != '\r');

        if (LastChar != EOF) {
            return gettok();
        }
    }

    //End: EOF, don't eat this token
    if (LastChar == EOF) {
        return tok_eof;
    }

    //Unknown: for such as 'a', just return its ASCII value
    int ThisChar = LastChar;
    LastChar = getchar();
    return ThisChar;
}



int main() {
    cout << "Hello, World!" << endl;
    return 0;
}