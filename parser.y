%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ast.h"
#include "semantic.h"
#include "ir.h"

int yylex(void);
int yyerror(const char* str);

node* root;
#define YYERROR_VERBOSE 1
#define YYSTYPE struct node*
%}

%token PROC FUNC RETURN VAR
%token TYPE STRING_TYPE
%token ID
%token INT_LITERAL REAL_LITERAL CHAR_LITERAL STRING_LITERAL BOOL_LITERAL NULL_LITERAL
%token LPAREN RPAREN LBRACE RBRACE LBRACKET RBRACKET
%token COLON SEMICOLON COMMA ASSIGN
%token PLUS MINUS STAR DIV NOT
%token IF ELSE WHILE FOR
%token AND OR DEREF ADDRESS BAR
%token EQ NE GT GE LT LE

%left OR
%left AND
%left EQ NE GT GE LT LE
%left PLUS MINUS
%left STAR DIV
%right NOT DEREF ADDRESS
%nonassoc LOWER_THAN_ELSE
%nonassoc ELSE


%%

program: decl_list { root = mknode("CODE", $1, NULL); };

decl_list: decl { $$ = $1; }
    | decl_list decl { $$ = mknode("", $1, $2); };

decl: proc_decl { $$ = $1; }
    | func_decl { $$ = $1; };

proc_decl: PROC ID LPAREN params RPAREN body { 
    $$ = mknode("PROC", $2, mknode("", $4, $6)); 
};

func_decl: FUNC ID LPAREN params RPAREN RETURN type body { 
    node* ret_node = mknode("RET", $7, NULL);
    $$ = mknode("FUNC", $2, mknode("", $4, mknode("", ret_node, $8))); 
};

params: { $$ = mknode("(ARGS NONE)", NULL, NULL); }
    | param_list { $$ = mknode("ARGS", $1, NULL); };

typed_ids: id_list COLON type { $$ = mknode($3->token, $1, NULL); };

id_list: ID { $$ = $1; }
    | ID COMMA id_list { $$ = mknode("", $1, $3); };

param_list: typed_ids { $$ = $1; }
    | param_list SEMICOLON typed_ids { $$ = mknode("", $1, $3); };

type: TYPE { $$ = $1; }
    | STRING_TYPE LBRACKET INT_LITERAL RBRACKET { $$ = mknode("STRING", $3, NULL); };

body: LBRACE opt_decls opt_vars opt_stmts RBRACE { $$ = mknode("BODY", $2, mknode("", $3, $4)); };

opt_decls: { $$ = NULL; }
    | decl_list  { $$ = $1; };

opt_vars: { $$ = NULL; }
    | var_decls  { $$ = $1; };

opt_stmts: { $$ = NULL; }
    | stmt_list  { $$ = $1; };

var_decls: var_decl { $$ = $1; }
    | var_decls var_decl { $$ = mknode("", $1, $2); };

var_decl: VAR id_list COLON type SEMICOLON { 
    $$ = mknode("VAR", mknode($4->token, $2, NULL), NULL); 
};

arg_list: expr { $$ = $1; }
    | expr COMMA arg_list { $$ = mknode("", $1, $3); };

opt_arg_list: { $$ = NULL; }
    | arg_list { $$ = $1; };

stmt_list: stmt { $$ = $1; }
    | stmt_list stmt { $$ = mknode("", $1, $2); };

stmt: assignment { $$ = $1; }
    | call_stmt { $$ = $1; }
    | return_stmt { $$ = $1; }
    | if_stmt { $$ = $1; }
    | while_stmt { $$ = $1; }
    | for_stmt { $$ = $1; }
    | body { $$ = $1; };

assignment: assign_expr SEMICOLON { $$ = $1; };

assign_expr: lhs ASSIGN expr { $$ = mknode("=", $1, $3); };

lhs: addressable { $$ = $1; }
    | DEREF expr { $$ = mknode("^", $2, NULL); };

return_stmt: RETURN expr SEMICOLON { $$ = mknode("RET", $2, NULL); };

call_stmt: ID LPAREN opt_arg_list RPAREN SEMICOLON { $$ = mknode("CALL", $1, $3); };

if_stmt: IF LPAREN expr RPAREN stmt %prec LOWER_THAN_ELSE { $$ = mknode("IF", $3, $5); }
    | IF LPAREN expr RPAREN stmt ELSE stmt { $$ = mknode("IF-ELSE", $3, mknode("", $5, $7)); };

while_stmt: WHILE LPAREN expr RPAREN stmt { $$ = mknode("WHILE", $3, $5);};

for_stmt: FOR LPAREN assign_expr SEMICOLON expr SEMICOLON assign_expr RPAREN stmt { $$ = mknode("FOR", $3, mknode("COND", $5, mknode("ITER", $7, $9))); };

addressable: ID { $$ = $1; }
    | ID LBRACKET expr RBRACKET { $$ = mknode("INDEX", $1, $3); };

expr: INT_LITERAL { $$ = $1; }
    | REAL_LITERAL { $$ = $1; }
    | CHAR_LITERAL { $$ = $1; }
    | STRING_LITERAL { $$ = $1; }
    | BOOL_LITERAL { $$ = $1; }
    | NULL_LITERAL { $$ = $1; }
    | ID { $$ = $1; }
    | ID LPAREN opt_arg_list RPAREN { $$ = mknode("CALL", $1, $3); }
    | ID LBRACKET expr RBRACKET { $$ = mknode("INDEX", $1, $3); }
    | expr PLUS expr { $$ = mknode("+", $1, $3); }
    | expr MINUS expr { $$ = mknode("-", $1, $3); }
    | expr STAR expr { $$ = mknode("*", $1, $3); }
    | expr DIV expr { $$ = mknode("/", $1, $3); }
    | NOT expr { $$ = mknode("!", $2, NULL); }
    | expr AND expr { $$ = mknode("&&", $1, $3); }
    | expr OR expr { $$ = mknode("||", $1, $3); }
    | DEREF expr { $$ = mknode("^", $2, NULL); }
    | ADDRESS addressable { $$ = mknode("&", $2, NULL); }
    | BAR ID BAR { $$ = mknode("LEN", $2, NULL); }
    | LPAREN expr RPAREN { $$ = $2; }
    | expr EQ expr { $$ = mknode("==", $1, $3); }
    | expr GT expr { $$ = mknode(">", $1, $3); }
    | expr GE expr { $$ = mknode(">=", $1, $3); }
    | expr LT expr { $$ = mknode("<", $1, $3); }
    | expr LE expr { $$ = mknode("<=", $1, $3); }
    | expr NE expr { $$ = mknode("!=", $1, $3); };

%%

node* mknode(char* token, node* left, node* right)
{
    node* newnode = (node*)malloc(sizeof(node));
    char* newstr = (char*)malloc(strlen(token) + 1);
    strcpy(newstr, token);
    newnode->token = newstr;
    newnode->left = left;
    newnode->right = right;
    return newnode;
}

void printtree(node* tree, int level) {
    if (!tree) return;

    if (!strcmp(tree->token, "")) {
        printtree(tree->left, level);
        if (level == -1) printf(" ");
        printtree(tree->right, level);
        return;
    }

    if (level >= 0) for (int i = 0; i < level; i++) printf("  ");

    int is_op = (strchr("+-*/=><!", tree->token[0]) != NULL);
    int kids_are_leaves = (!tree->left || (!tree->left->left && !tree->left->right)) && 
                          (!tree->right || (!tree->right->left && !tree->right->right));

    int should_inline = (level == -1 || 
                         !strcmp(tree->token, "int") || !strcmp(tree->token, "real") || 
                         !strcmp(tree->token, "char") || !strcmp(tree->token, "bool") || 
                         !strcmp(tree->token, "RET") || !strcmp(tree->token, "NONE") ||
                         !strcmp(tree->token, "ARGS NONE") ||
                         (is_op && kids_are_leaves));

    if (!tree->left && !tree->right) {
        printf("%s%s", tree->token, level >= 0 ? "\n" : "");
    } else if (should_inline) {
        printf("(%s ", tree->token);
        printtree(tree->left, -1);
        if (tree->left && tree->right) printf(" ");
        printtree(tree->right, -1);
        printf(")%s", level >= 0 ? "\n" : "");
    } else if (!strcmp(tree->token, "=")) {
        printf("(= ");
        printtree(tree->left, -1);
        printf("\n");
        printtree(tree->right, level + 1);
        for (int i = 0; i < level; i++) printf("  ");
        printf(")\n");
    } else {
        printf("(%s\n", tree->token);
        printtree(tree->left, level + 1);
        printtree(tree->right, level + 1);
        for (int i = 0; i < level; i++) printf("  ");
        printf(")\n");
    }
}

int main()
{
    if (yyparse() == 0)
    {
        printf("Parsing successful\n");
        printtree(root, 0);
        printf("---------------------------------------------\n");
        enter_scope();
        analyzeAST(root);
        checkMainValidity();
        initIR("output.txt");
        generate3AC(root);
        closeIR();

        printf("3AC was written to output.txt\n");
    }
    return 0;
}

extern int yylineno;

int yyerror(const char* str)
{
    printf("Parsing failed at line %d: %s \n", yylineno, str);
    return 0;
}

#include "lex.yy.c"