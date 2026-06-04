#ifndef SEMANTIC_H
#define SEMANTIC_H

#include "ast.h"

typedef struct symbol
{
    char* name;
    char* kind;
    char* type;

    int param_count;
    char** param_types;

    struct symbol* next;

} symbol;

typedef struct scope
{
    int id;
    struct scope* parent;
    symbol* symbols;
} scope;

extern scope* current_scope;
extern char* current_func_return_type;
extern int main_found;

int isLiteral(char* token);
int isTypeName(char* token);
int isPointerType(char* type);
int isOperator(char* token);

scope* enter_scope();
void exit_scope();

symbol* insert_symbol(char* name, char* kind, char* type);
symbol* lookup_current(char* name);
symbol* lookup_all(char* name);

void insertIdList(node* ids, char* type);
void analyzeVarDecl(node* tree);
void extractParams(node* tree, symbol* func_sym);
void analyzeParams(node* tree);
void analyzeIdentifier(node* tree);

char* evaluateType(node* tree);
void analyzeAssignment(node* tree);
void analyzeCall(node* tree);
void analyzeReturn(node* tree);
void analyzeAST(node* tree);
void checkMainValidity();

#endif