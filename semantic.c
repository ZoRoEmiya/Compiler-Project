#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "semantic.h"

scope* current_scope = NULL;
int next_scope_id = 0;
char* current_func_return_type = NULL;
int main_found = 0;

int isLiteral(char* token) {
    if (!token) return 0;
    if (strcmp(token, "true") == 0 || strcmp(token, "false") == 0 || strcmp(token, "null") == 0) return 1;
    if (isdigit(token[0]) || token[0] == '\'' || token[0] == '"') return 1;
    return 0;
}

int isTypeName(char* token) {
    if (!token) return 0;
    return strcmp(token, "int") == 0 || strcmp(token, "real") == 0 ||
           strcmp(token, "bool") == 0 || strcmp(token, "char") == 0 ||
           strcmp(token, "int*") == 0 || strcmp(token, "real*") == 0 ||
           strcmp(token, "char*") == 0 || strcmp(token, "STRING") == 0;
}

int isPointerType(char* type) {
    if (!type) return 0;
    return strchr(type, '*') != NULL;
}

int isOperator(char* token) {
    if (!token) return 0;
    return strchr("+-*/=><!^&|", token[0]) != NULL || 
           strcmp(token, "==") == 0 || strcmp(token, "!=") == 0 ||
           strcmp(token, ">=") == 0 || strcmp(token, "<=") == 0 ||
           strcmp(token, "&&") == 0 || strcmp(token, "||") == 0;
}

scope* enter_scope() {
    scope* s = malloc(sizeof(scope));
    s->id = next_scope_id++;
    s->parent = current_scope;
    s->symbols = NULL;
    current_scope = s;
    return s;
}

void exit_scope() {
    if (current_scope != NULL)
        current_scope = current_scope->parent;
}

symbol* insert_symbol(char* name, char* kind, char* type) {
    symbol* s = malloc(sizeof(symbol));
    s->name = strdup(name);
    s->kind = strdup(kind);
    s->type = strdup(type);
    s->param_count = 0;
    s->param_types = NULL;
    s->next = current_scope->symbols;
    current_scope->symbols = s;
    return s;
}

symbol* lookup_current(char* name) {
    if (!current_scope) return NULL;
    symbol* s = current_scope->symbols;
    while (s) {
        if (strcmp(s->name, name) == 0) return s;
        s = s->next;
    }
    return NULL;
}

symbol* lookup_all(char* name) {
    scope* temp = current_scope;
    while (temp) {
        symbol* s = temp->symbols;
        while (s) {
            if (strcmp(s->name, name) == 0) return s;
            s = s->next;
        }
        temp = temp->parent;
    }
    return NULL;
}

void insertIdList(node* ids, char* type) {
    if (!ids) return;
    if (strcmp(ids->token, "") == 0) {
        insertIdList(ids->left, type);
        insertIdList(ids->right, type);
        return;
    }
    if (lookup_current(ids->token) != NULL) {
        printf("Semantic Error: Redeclaration of variable '%s' in the same scope\n", ids->token);
        return;
    }
    insert_symbol(ids->token, "var", type);
}

void analyzeVarDecl(node* tree) {
    if (!tree) return;
    node* typeNode = tree->left;
    char* type = typeNode->token;
    insertIdList(typeNode->left, type);
}

void extractParams(node* tree, symbol* func_sym) {
    if (!tree || !func_sym) return;
    
    if (strcmp(tree->token, "ARGS") == 0 || strcmp(tree->token, "") == 0) {
        extractParams(tree->left, func_sym);
        extractParams(tree->right, func_sym);
        return;
    }
    
    if (isTypeName(tree->token)) {
        char* type = tree->token;
        node* ids = tree->left;
        
        node* temp = ids;
        int count = 0;
        while (temp && strcmp(temp->token, "") == 0) { count++; temp = temp->right; }
        if (temp) count++;
        
        int start_idx = func_sym->param_count;
        func_sym->param_count += count;
        func_sym->param_types = realloc(func_sym->param_types, func_sym->param_count * sizeof(char*));
        
        for (int i = 0; i < count; i++) {
            func_sym->param_types[start_idx + i] = strdup(type);
        }
    }
}

void analyzeParams(node* tree) {
    if (!tree) return;
    if (strcmp(tree->token, "ARGS") == 0 || strcmp(tree->token, "") == 0) {
        analyzeParams(tree->left);
        analyzeParams(tree->right);
        return;
    }
    if (isTypeName(tree->token)) {
        insertIdList(tree->left, tree->token);
        return;
    }
}

char* evaluateType(node* tree) {
    if (!tree) return "unknown";

    if (isLiteral(tree->token)) {
        if (strcmp(tree->token, "true") == 0 || strcmp(tree->token, "false") == 0) return "bool";
        if (strcmp(tree->token, "null") == 0) return "null";
        if (tree->token[0] == '\'') return "char";
        if (tree->token[0] == '"') return "STRING";
        if (strchr(tree->token, '.') != NULL) return "real";
        return "int";
    }

    if (strcmp(tree->token, "LEN") == 0) {
        char* innerType = evaluateType(tree->left);
        if (strcmp(innerType, "STRING") != 0) {
            printf("Semantic Error: Length operator '| |' can only be applied to strings\n");
        }
        return "int";
    }

    if (strcmp(tree->token, "INDEX") == 0) {
        char* arrType = evaluateType(tree->left);
        char* idxType = evaluateType(tree->right);
        
        if (strcmp(arrType, "STRING") != 0) {
            printf("Semantic Error: Operator [] can only be used on strings\n");
        }
        if (strcmp(idxType, "int") != 0) {
            printf("Semantic Error: String index must be of type int\n");
        }
        return "char";
    }

    if (strcmp(tree->token, "CALL") == 0) {
        symbol* s = lookup_all(tree->left->token);
        if (!s) return "unknown";
        if (strcmp(s->kind, "proc") == 0) {
            printf("Semantic Error: Procedure '%s' does not return a value\n", s->name);
            return "unknown";
        }
        return s->type;
    }

    if (strcmp(tree->token, "^") == 0) {
        char* ptrType = evaluateType(tree->left);
        if (!isPointerType(ptrType)) {
            printf("Semantic Error: Dereference operator '^' can only be applied to pointers\n");
            return "unknown";
        }
        char* baseType = malloc(strlen(ptrType));
        strncpy(baseType, ptrType, strlen(ptrType) - 1);
        baseType[strlen(ptrType) - 1] = '\0';
        return baseType;
    }

    if (strcmp(tree->token, "&") == 0) {
        char* varType = evaluateType(tree->left);
        if (strcmp(varType, "int") != 0 && strcmp(varType, "real") != 0 && strcmp(varType, "char") != 0) {
            printf("Semantic Error: Address operator '&' can only be applied to int, real, char, or string cells\n");
            return "unknown";
        }
        char* ptrType = malloc(strlen(varType) + 2);
        sprintf(ptrType, "%s*", varType);
        return ptrType;
    }

    symbol* s = lookup_all(tree->token);
    if (s != NULL) {
        if (strcmp(s->kind, "func") == 0 || strcmp(s->kind, "proc") == 0) {
            printf("Semantic Error: Cannot use function/procedure '%s' as a variable\n", s->name);
            return "unknown";
        }
        return s->type;
    }

    if (strchr("+-*/", tree->token[0]) != NULL && strlen(tree->token) == 1) {
        char* leftType = evaluateType(tree->left);
        char* rightType = evaluateType(tree->right);

        if (isPointerType(leftType)) {
            if ((strcmp(tree->token, "+") == 0 || strcmp(tree->token, "-") == 0) && strcmp(rightType, "int") == 0) {
                return leftType;
            }
            printf("Semantic Error: Pointer arithmetic allows only adding/subtracting an int\n");
            return "unknown";
        }

        if ((strcmp(leftType, "int") != 0 && strcmp(leftType, "real") != 0) ||
            (strcmp(rightType, "int") != 0 && strcmp(rightType, "real") != 0)) {
            printf("Semantic Error: Arithmetic operators require int or real operands\n");
            return "unknown";
        }
        
        if (strcmp(leftType, "real") == 0 || strcmp(rightType, "real") == 0) return "real";
        return "int";
    }

    if (strcmp(tree->token, "&&") == 0 || strcmp(tree->token, "||") == 0) {
        if (strcmp(evaluateType(tree->left), "bool") != 0 || strcmp(evaluateType(tree->right), "bool") != 0) {
            printf("Semantic Error: Logical operators && and || require bool operands\n");
        }
        return "bool";
    }
    
    if (strcmp(tree->token, "!") == 0) {
        if (strcmp(evaluateType(tree->left), "bool") != 0) {
            printf("Semantic Error: Operator ! requires a bool operand\n");
        }
        return "bool";
    }

    if (strcmp(tree->token, ">") == 0 || strcmp(tree->token, "<") == 0 || 
        strcmp(tree->token, ">=") == 0 || strcmp(tree->token, "<=") == 0) {
        char* leftType = evaluateType(tree->left);
        char* rightType = evaluateType(tree->right);
        if ((strcmp(leftType, "int") != 0 && strcmp(leftType, "real") != 0) ||
            (strcmp(rightType, "int") != 0 && strcmp(rightType, "real") != 0)) {
            printf("Semantic Error: Relational operators <, >, <=, >= require int or real operands\n");
        }
        return "bool";
    }

    if (strcmp(tree->token, "==") == 0 || strcmp(tree->token, "!=") == 0) {
        char* leftType = evaluateType(tree->left);
        char* rightType = evaluateType(tree->right);
        
        if (strcmp(leftType, rightType) != 0) {
             printf("Semantic Error: Operators == and != require operands of the exact same type\n");
        } else if (strcmp(leftType, "STRING") == 0) {
             printf("Semantic Error: Operators == and != cannot be used directly on strings\n");
        }
        return "bool";
    }

    return "unknown";
}

void analyzeAssignment(node* tree) {
    char* leftType = evaluateType(tree->left);
    char* rightType = evaluateType(tree->right);

    if (strcmp(rightType, "null") == 0) {
        if (!isPointerType(leftType)) {
            printf("Semantic Error: NULL can only be assigned to a pointer\n");
        }
        return;
    }

    if (strcmp(tree->left->token, "INDEX") == 0 && strcmp(leftType, "char") == 0) {
        if (strcmp(rightType, "char") != 0) {
            printf("Semantic Error: String cells can only be assigned characters\n");
        }
        return;
    }

    if (strcmp(leftType, rightType) != 0) {
        if (!(strcmp(leftType, "real") == 0 && strcmp(rightType, "int") == 0)) {
            printf("Semantic Error: Type mismatch in assignment\n");
        }
    }
}

void analyzeCall(node* tree) {
    if (!tree) return;
    symbol* s = lookup_all(tree->left->token);
    
    if (!s || (strcmp(s->kind, "func") != 0 && strcmp(s->kind, "proc") != 0)) {
        printf("Semantic Error: Undeclared function/procedure '%s'\n", tree->left->token);
        return;
    }

    int arg_count = 0;
    node* arg_list = tree->right;
    
    while (arg_list && strcmp(arg_list->token, "") == 0) {
        char* arg_type = evaluateType(arg_list->left);
        if (arg_count < s->param_count) {
            if (strcmp(arg_type, s->param_types[arg_count]) != 0 && 
                !(strcmp(s->param_types[arg_count], "real") == 0 && strcmp(arg_type, "int") == 0)) {
                printf("Semantic Error: Argument %d type mismatch in call to '%s'\n", arg_count + 1, s->name);
            }
        }
        arg_count++;
        arg_list = arg_list->right;
    }
    
    if (arg_list && strcmp(arg_list->token, "") != 0) {
        char* arg_type = evaluateType(arg_list);
        if (arg_count < s->param_count) {
            if (strcmp(arg_type, s->param_types[arg_count]) != 0 && 
                !(strcmp(s->param_types[arg_count], "real") == 0 && strcmp(arg_type, "int") == 0)) {
                printf("Semantic Error: Argument %d type mismatch in call to '%s'\n", arg_count + 1, s->name);
            }
        }
        arg_count++;
    }

    if (arg_count != s->param_count) {
        printf("Semantic Error: Incorrect number of arguments for '%s'\n", s->name);
    }
}

void analyzeReturn(node* tree) {
    if (!current_func_return_type) {
        printf("Semantic Error: Return statement outside of a function\n");
        return;
    }
    
    char* ret_type = evaluateType(tree->left);
    
    if (strcmp(current_func_return_type, "STRING") == 0) {
        printf("Semantic Error: Function cannot return a string\n");
    } else if (strcmp(current_func_return_type, ret_type) != 0) {
        if (!(strcmp(current_func_return_type, "real") == 0 && strcmp(ret_type, "int") == 0)) {
            printf("Semantic Error: Return type mismatch\n");
        }
    }
}

void checkMainValidity() {
    symbol* main_sym = lookup_all("Main");
    if (!main_sym || strcmp(main_sym->kind, "proc") != 0) {
        printf("Semantic Error: Procedure 'Main' must exist and be unique.\n");
    } else if (main_sym->param_count > 0) {
        printf("Semantic Error: Procedure 'Main' cannot take arguments.\n");
    }
}

void analyzeAST(node* tree) {
    if (!tree) return;

    if (strcmp(tree->token, "BODY") == 0) {
        enter_scope();
        analyzeAST(tree->left);
        analyzeAST(tree->right);
        exit_scope();
        return;
    }

    if (strcmp(tree->token, "VAR") == 0) {
        analyzeVarDecl(tree);
        return;
    }

    if (strcmp(tree->token, "PROC") == 0) {
        if (strcmp(tree->left->token, "Main") == 0) main_found = 1;

        if (lookup_current(tree->left->token) != NULL) {
            printf("Semantic Error: Procedure '%s' already declared\n", tree->left->token);
        } else {
            symbol* s = insert_symbol(tree->left->token, "proc", "void");
            extractParams(tree->right->left, s);
        }
        current_func_return_type = NULL;
        enter_scope();
        analyzeParams(tree->right->left);
        analyzeAST(tree->right->right);
        exit_scope();
        return;
    }

    if (strcmp(tree->token, "FUNC") == 0) {
        node* ret_node = tree->right->right->left;
        char* return_type = ret_node->left->token;
        
        if (strcmp(return_type, "STRING") == 0) {
            printf("Semantic Error: Function '%s' cannot have a string return type\n", tree->left->token);
        }

        if (lookup_current(tree->left->token) != NULL) {
            printf("Semantic Error: Function '%s' already declared\n", tree->left->token);
        } else {
            symbol* s = insert_symbol(tree->left->token, "func", return_type);
            extractParams(tree->right->left, s);
        }
        
        current_func_return_type = return_type;
        enter_scope();
        analyzeParams(tree->right->left);
        analyzeAST(tree->right->right->right);
        exit_scope();
        return;
    }

    if (strcmp(tree->token, "=") == 0) {
        analyzeAssignment(tree);
    } else if (strcmp(tree->token, "CALL") == 0) {
        analyzeCall(tree);
    } else if (strcmp(tree->token, "RET") == 0) {
        analyzeReturn(tree);
    } else if (strcmp(tree->token, "IF") == 0 || strcmp(tree->token, "IF-ELSE") == 0) {
        if (strcmp(evaluateType(tree->left), "bool") != 0) {
            printf("Semantic Error: 'if' condition must be of type bool\n");
        }
    } else if (strcmp(tree->token, "WHILE") == 0) {
        if (strcmp(evaluateType(tree->left), "bool") != 0) {
            printf("Semantic Error: 'while' condition must be of type bool\n");
        }
    } else if (strcmp(tree->token, "FOR") == 0) {
        if (strcmp(evaluateType(tree->right->left), "bool") != 0) {
            printf("Semantic Error: 'for' condition must be of type bool\n");
        }
    }

    if (!isOperator(tree->token) && !isLiteral(tree->token) && !isTypeName(tree->token) &&
        strcmp(tree->token, "BODY") != 0 && strcmp(tree->token, "ARGS") != 0 && 
        strcmp(tree->token, "RET") != 0 && strcmp(tree->token, "INDEX") != 0 && 
        strcmp(tree->token, "CALL") != 0 && strcmp(tree->token, "CODE") != 0 && strcmp(tree->token, "") != 0) {
        if (tree->left == NULL && tree->right == NULL) {
            if (lookup_all(tree->token) == NULL) {
                printf("Semantic Error: Undeclared identifier '%s'\n", tree->token);
            }
        }
    }

    analyzeAST(tree->left);
    analyzeAST(tree->right);
}