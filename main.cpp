#include <iostream>
#include <cstdio>
#include <vector>
#include <map>
#include "llvm/ADT/STLExtras.h"

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
static double NumVal;    //will hold the token number value if it is tok_number

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

/*
 * Expression Abastract Syntax Tree
 */
class ExprAST {
public:
    virtual ~ExprAST();
};

/*
 * Number expression for such as '1.0'
 */
class NumberExprAST : public ExprAST {
    double Val;

public:
    NumberExprAST(double val) : Val(val) {}
};

/*
 * Variable expression for such as 'a'
 */
class VariableExprAST : public ExprAST {
    string Name;

public:
    VariableExprAST(const string &name) : Name(name) {}
};

/*
 * Binary expression for such as '+'
 */

class BinaryExprAST : public ExprAST {
    char Op;
    unique_ptr<ExprAST> LHS,RHS;//Left hand side and right hand side op
    //AST will use node to left child and right child

public:
    BinaryExprAST(char op, unique_ptr<ExprAST> lhs, unique_ptr<ExprAST> rhs)
            : Op(op), LHS(std::move(lhs)), RHS(std::move(rhs)) {}
};

/*
 * Call expression for function call such as 'add()'
 */

class CallExprAST : public ExprAST {
    string Callee;  //callee function name
    vector<unique_ptr<ExprAST>> Args;

public:
    CallExprAST(const string &callee, vector<unique_ptr<ExprAST>> args)
            : Callee(callee), Args(std::move(args)) {}
};

/*
 * Prototype of function, include function name and args
 * Just now all function args are all double, so don't need type filed
 */

class PrototypeAST {
    string Name;
    vector<string> Args;

public:
    PrototypeAST(const string &name, vector<string> args)
            : Name(name), Args(std::move(args)) {}
};

/*
 * Body of function, include function prototype and body expression
 */
class FunctionAST {
    unique_ptr<PrototypeAST> Proto;
    unique_ptr<ExprAST> Body;

public:
    FunctionAST(unique_ptr<PrototypeAST> proto, unique_ptr<ExprAST> body)
            : Proto(std::move(proto)), Body(std::move(body)) {}
};





/*
 * Base expression parse
 */
static int CurTok;  //Current token
static int getNextToken() { //Get next token and update CurTok
    return CurTok = gettok();
}

/*
 * Handle error token
 */
unique_ptr<ExprAST> Error(const char *Str) {
    fprintf(stderr, "Error:%s\n", Str);
    return nullptr;
}

unique_ptr<PrototypeAST> ErrorP(const char *Str) {
    Error(Str);
    return nullptr;
}

/*
 * Parse number ::= number
 */
static unique_ptr<ExprAST> ParseNumberExpr() {
    auto Result = llvm::make_unique<NumberExprAST>(NumVal);
    getNextToken();
    return std::move(Result);
}

/*
 * Define parse binary expression, see later
 */
static unique_ptr<ExprAST> ParseExpression();

/*
 * Parse paren ::= '(' expression ')'
 */
static unique_ptr<ExprAST> ParseParenExpr() {
    getNextToken(); //Eat '('
    auto Result = ParseExpression();
    if (!Result) {
        return nullptr;
    }

    if (CurTok != ')') {
        return Error("excepted ')'");
    }

    getNextToken();//Eat ')'
    return Result;
}

/*
 * Parse identifier ::= identifier
 * ::= identifier '(' expression* ')'
 */
static unique_ptr<ExprAST> ParseIndentifierExpr() {
    string IdName = IdentifierStr;

    getNextToken(); //Eat identifier

    if (CurTok != '(') {    //Look ahead, if it's a variable reference return VariableExprAST
        return llvm::make_unique<VariableExprAST>(IdName);
    }

    //Else is a function call and should return CallExprAST
    getNextToken(); //Eat '('
    vector<unique_ptr<ExprAST>> Args;
    if (CurTok != ')') {
        while (true) {  //Parse all the args
            if (auto Arg = ParseExpression()) {
                Args.push_back(std::move(Arg));
            }
            else {
                return nullptr;
            }

            if (CurTok == ')') break;   //End

            if (CurTok != ',') {
                return Error("Excepted ')' or ',' in arguments");
            }

            getNextToken(); //Eat ',' to next arg
        }
    }

    getNextToken(); //Eat ')'

    return llvm::make_unique<CallExprAST>(IdName, std::move(Args));
}

/*
 * Parse primary of expression ::= indentifier
 * ::= numberexpr
 * ::= parenexpr
 */
static unique_ptr<ExprAST> ParsePrimary() {
    switch (CurTok) {
        default:
            return Error("Unkown token when excepting an expression");
        case tok_identifier:
            return ParseIndentifierExpr();
        case tok_number:
            return ParseNumberExpr();
        case '(':   //In order to parse precedence
            return ParseParenExpr();
    }
}




/*
 * Binary expression parse
 */


static map<char ,int> BinoPrecedence;   //Hold precedence for each binary ops

/*
 * Get the precedence of an op
 */
static int GetTokPrecedence() {
    if (!isascii(CurTok)) { //Not a ASCII op, set to -1
        return -1;
    }

    int TokPrec = BinoPrecedence[CurTok];
    if (TokPrec < 0) {  //Unknown binary op, set to -1
        return -1;
    }
    return TokPrec;
}

/*
 * Define parse binary with precedence
 */
static unique_ptr<ExprAST> ParseBinOpRHS(int ,unique_ptr<ExprAST>);

/*
 * Parse binary expression entry
 * For ops like "a+b+(c+d)", we should split ops into pairs like [+ b], [+ (c+d)]
 * So we should firstly parse the LHS use ParsePrimary() to parse 'a'
 * Attention that one op like single 'x' is also valid
 */
static unique_ptr<ExprAST> ParseExpression() {
    auto LHS = ParsePrimary();
    if (!LHS) { //Not a expression
        return nullptr;
    }

    return ParseBinOpRHS(0, std::move(LHS));
}

/*
 * Parse binary expression with precedence
 * ::= ('+' primary)*
 */
static unique_ptr<ExprAST> ParseBinOpRHS(int ExprPrec, unique_ptr<ExprAST> LHS) {
    /*
     * While is hard to understand,take ops exmaple "a+b+(c+d)*e*f+g" to explain
     * First the ops will be split into [a] [+, b] [+, (c+d)] [*, e] [*, f] [+, g]
     * Paren will use ParseParen() to process recursion call
     */
    while (true) {
        int TokPrec = GetTokPrecedence();

        /*
         * Ignore wrong op or the single op and the first op(example [a])
         * At first time the ExprPrec is 0 and only -1 will go into this
         */
        if (TokPrec < ExprPrec) {
            return LHS;
        }

        /*
         * Now the ops are all valid and in pairs
         * Example [+, b] [+, (c+d)] [*, e] [*, f] [+, g]
         */
        int BinOp = CurTok; //Save current token
        getNextToken(); //Eat op and to next token

        auto RHS = ParsePrimary();  //Get RHS
        if (!RHS) {
            return nullptr;
        }

        /*
         * Now we have LHS and RHS, example is [+ b] and [+ op unparsed]
         * First to look ahead to judge "(a+b) op unparsed" or "a+(b op unparsed)"
         */
        int NextPrec = GetTokPrecedence();

        /*
         * RHS > current, like "a+(b op unparsed)"
         * Show [LHS] and [RHS]
         * First time: [a] and [+ b], equal, skip and make AST node
         * Second time: [a+b] and [+ (c+d)], equal, skip and make AST node
         * Third time: [a+b+] and [(c+d)], paren process to recursion call{
         *     First time: [c] [+ d]: equal, skip and make AST node
         *     Second time: [c+d] [* e]: '+' < '*', go into if condition
         *     Third time: [(c+d)*e] [* f]: equal, skip and make AST node
         *     Forth time: [(c+d)*e*f] [end]: end and return
         * }
         * Forth time: [a+b+(c+d)*e*f] [end]: end and return
         */
        if (NextPrec > TokPrec) {
            /*
             * Recursion call to set all rest RHS as current LHS
             * TokPrec + 1 to ignore that "if (TokPrec < ExprPrec) return LHS;"
             * Example: [(c+d)*e*f] as RHS
             */
            RHS = ParseBinOpRHS(TokPrec + 1, std::move(RHS));
            if (!RHS) {
                return nullptr;
            }
        }
        /*
         * RHS  <= current, like "(a+b) op unparsed", create AST and continue
         */

        //Update LHS and use while loop to process next ops pair
        LHS = llvm::make_unique<ExprAST>(BinOp, std::move(LHS), std::move(RHS));
    }
}

static void HandleExtern() {
  if (ParseExtern()) {
    fprintf(stderr, "Parsed an extern\n");
  } else {
    // Skip token for error recovery.
    getNextToken();
  }
}

static void HandleTopLevelExpression() {
  // Evaluate a top-level expression into an anonymous function.
  if (ParseTopLevelExpr()) {
    fprintf(stderr, "Parsed a top-level expr\n");
  } else {
    // Skip token for error recovery.
    getNextToken();
  }
}

/// top ::= definition | external | expression | ';'
static void MainLoop() {
  while (1) {
    fprintf(stderr, "ready> ");
    switch (CurTok) {
    case tok_eof:
      return;
    case ';': // ignore top-level semicolons.
      getNextToken();
      break;
    case tok_def:
      HandleDefinition();
      break;
    case tok_extern:
      HandleExtern();
      break;
    default:
      HandleTopLevelExpression();
      break;
    }
  }
}

int main() {
    BinoPrecedence['<'] = 10;
    BinoPrecedence['>'] = 10;
    BinoPrecedence['+'] = 20;
    BinoPrecedence['-'] = 20;
    BinoPrecedence['*'] = 40;
    BinoPrecedence['/'] = 40;

    fprintf(stdout, "input>> ");
    getNextToken();

//    MainLoop();

    return 0;
}