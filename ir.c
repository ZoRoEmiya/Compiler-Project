#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ir.h"

static FILE* ir_output = NULL;
static int temp_counter = 0;
static int label_counter = 1;

static char* copyString(const char* str);
static int isEmptyNode(node* tree);
static int isLeaf(node* tree);
static int isLiteralToken(const char* token);
static int isBinaryOperator(const char* token);

static void generateNode(node* tree);
static void generateFunction(node* tree);
static void generateProcedure(node* tree);
static void generateBody(node* tree);
static void generateStatementList(node* tree);
static void generateStatement(node* tree);

static char* generateExpression(node* tree);
static char* generateLogicalExpression(node* tree);
static char* generateLValue(node* tree);
static void generateAssignment(node* tree);
static void generateReturn(node* tree);
static int generateArguments(node* tree);
static char* generateCall(node* tree);


static void generateIf(node* tree);
static void generateIfElse(node* tree);
static void generateWhile(node* tree);
static void generateFor(node* tree);

static void emitAssign(const char* left, const char* right);
static void emitBinary(const char* result, const char* left, const char* op, const char* right);
static void emitUnary(const char* result, const char* op, const char* value);
static void emitReturnValue(const char* value);
static void emitGoto(const char* label);
static void emitIfGoto(const char* condition, const char* label);

static void emitPushParam(const char* value)
{
    if (ir_output == NULL || value == NULL)
    {
        return;
    }

    fprintf(ir_output, "PushParam %s\n", value);
}

static void emitCallResult(const char* result, const char* name)
{
    if (ir_output == NULL || result == NULL || name == NULL)
    {
        return;
    }

    fprintf(ir_output, "%s = LCall %s\n", result, name);
}

static void emitPopParams(int size)
{
    if (ir_output == NULL)
    {
        return;
    }

    fprintf(ir_output, "PopParams %d\n", size);
}

static void emitGoto(const char* label)
{
    if (ir_output == NULL || label == NULL)
    {
        return;
    }

    fprintf(ir_output, "Goto %s\n", label);
}

static void emitIfGoto(const char* condition, const char* label)
{
    if (ir_output == NULL || condition == NULL || label == NULL)
    {
        return;
    }

    fprintf(ir_output, "if %s Goto %s\n", condition, label);
}

void initIR(const char* outputFileName)
{
    ir_output = fopen(outputFileName, "w");

    if (ir_output == NULL)
    {
        printf("Error: cannot open output file '%s'\n", outputFileName);
        exit(1);
    }

    temp_counter = 0;
    label_counter = 1;
}

void closeIR()
{
    if (ir_output != NULL)
    {
        fclose(ir_output);
        ir_output = NULL;
    }
}

char* newTemp()
{
    char buffer[32];
    char* temp;

    sprintf(buffer, "t%d", temp_counter);
    temp_counter++;

    temp = (char*)malloc(strlen(buffer) + 1);
    strcpy(temp, buffer);

    return temp;
}

char* newLabel()
{
    char buffer[32];
    char* label;

    sprintf(buffer, "L%d", label_counter);
    label_counter++;

    label = (char*)malloc(strlen(buffer) + 1);
    strcpy(label, buffer);

    return label;
}

void emitLine(const char* line)
{
    if (ir_output == NULL || line == NULL)
    {
        return;
    }

    fprintf(ir_output, "%s\n", line);
}

void emitLabel(const char* label)
{
    if (ir_output == NULL || label == NULL)
    {
        return;
    }

    fprintf(ir_output, "%s:\n", label);
}

void emitFunctionHeader(const char* name)
{
    if (ir_output == NULL || name == NULL)
    {
        return;
    }

    fprintf(ir_output, "%s:\n", name);
}

void emitBeginFunc(int size)
{
    if (ir_output == NULL)
    {
        return;
    }

    fprintf(ir_output, "BeginFunc %d\n", size);
}

void emitEndFunc()
{
    if (ir_output == NULL)
    {
        return;
    }

    fprintf(ir_output, "EndFunc\n");
}

static void emitAssign(const char* left, const char* right)
{
    if (ir_output == NULL || left == NULL || right == NULL)
    {
        return;
    }

    fprintf(ir_output, "%s = %s\n", left, right);
}

static void emitBinary(const char* result, const char* left, const char* op, const char* right)
{
    if (ir_output == NULL || result == NULL || left == NULL || op == NULL || right == NULL)
    {
        return;
    }

    fprintf(ir_output, "%s = %s %s %s\n", result, left, op, right);
}

static void emitUnary(const char* result, const char* op, const char* value)
{
    if (ir_output == NULL || result == NULL || op == NULL || value == NULL)
    {
        return;
    }

    fprintf(ir_output, "%s = %s%s\n", result, op, value);
}

static void emitReturnValue(const char* value)
{
    if (ir_output == NULL || value == NULL)
    {
        return;
    }

    fprintf(ir_output, "Return %s\n", value);
}

void generate3AC(node* root)
{
    if (root == NULL)
    {
        return;
    }

    generateNode(root);
}

static char* copyString(const char* str)
{
    char* result;

    if (str == NULL)
    {
        return NULL;
    }

    result = (char*)malloc(strlen(str) + 1);
    strcpy(result, str);

    return result;
}

static int isEmptyNode(node* tree)
{
    return tree != NULL && strcmp(tree->token, "") == 0;
}

static int isLeaf(node* tree)
{
    return tree != NULL && tree->left == NULL && tree->right == NULL;
}

static int isLiteralToken(const char* token)
{
    if (token == NULL)
    {
        return 0;
    }

    if (strcmp(token, "true") == 0 || strcmp(token, "false") == 0 || strcmp(token, "null") == 0)
    {
        return 1;
    }

    if (token[0] >= '0' && token[0] <= '9')
    {
        return 1;
    }

    if (token[0] == '\'' || token[0] == '"')
    {
        return 1;
    }

    return 0;
}

static int isBinaryOperator(const char* token)
{
    if (token == NULL)
    {
        return 0;
    }

    return strcmp(token, "+") == 0 ||
           strcmp(token, "-") == 0 ||
           strcmp(token, "*") == 0 ||
           strcmp(token, "/") == 0 ||
           strcmp(token, "==") == 0 ||
           strcmp(token, "!=") == 0 ||
           strcmp(token, ">") == 0 ||
           strcmp(token, ">=") == 0 ||
           strcmp(token, "<") == 0 ||
           strcmp(token, "<=") == 0;
}

static void generateNode(node* tree)
{
    if (tree == NULL)
    {
        return;
    }

    if (strcmp(tree->token, "CODE") == 0)
    {
        generateNode(tree->left);
        return;
    }

    if (isEmptyNode(tree))
    {
        generateNode(tree->left);
        generateNode(tree->right);
        return;
    }

    if (strcmp(tree->token, "FUNC") == 0)
    {
        generateFunction(tree);
        return;
    }

    if (strcmp(tree->token, "PROC") == 0)
    {
        generateProcedure(tree);
        return;
    }
}

static void generateFunction(node* tree)
{
    node* nameNode;
    node* bodyNode;

    if (tree == NULL)
    {
        return;
    }

    nameNode = tree->left;
    bodyNode = tree->right->right->right;

    emitFunctionHeader(nameNode->token);
    emitBeginFunc(0);

    generateBody(bodyNode);

    emitEndFunc();
    emitLine("");
}

static void generateProcedure(node* tree)
{
    node* nameNode;
    node* bodyNode;

    if (tree == NULL)
    {
        return;
    }

    nameNode = tree->left;
    bodyNode = tree->right->right;

    if (strcmp(nameNode->token, "Main") == 0)
    {
        emitFunctionHeader("main");
    }
    else
    {
        emitFunctionHeader(nameNode->token);
    }

    emitBeginFunc(0);

    generateBody(bodyNode);

    emitEndFunc();
    emitLine("");
}

static void generateBody(node* tree)
{
    if (tree == NULL)
    {
        return;
    }

    if (strcmp(tree->token, "BODY") != 0)
    {
        return;
    }

    if (tree->right != NULL)
    {
        generateStatementList(tree->right->right);
    }
}

static void generateStatementList(node* tree)
{
    if (tree == NULL)
    {
        return;
    }

    if (isEmptyNode(tree))
    {
        generateStatementList(tree->left);
        generateStatementList(tree->right);
        return;
    }

    generateStatement(tree);
}

static void generateStatement(node* tree)
{
    if (tree == NULL)
    {
        return;
    }

    if (strcmp(tree->token, "=") == 0)
    {
        generateAssignment(tree);
        return;
    }

    if (strcmp(tree->token, "RET") == 0)
    {
        generateReturn(tree);
        return;
    }
    if (strcmp(tree->token, "CALL") == 0)
    {
        generateCall(tree);
        return;
    }
    if (strcmp(tree->token, "IF") == 0)
    {
        generateIf(tree);
        return;
    }

    if (strcmp(tree->token, "IF-ELSE") == 0)
    {
        generateIfElse(tree);
        return;
    }
    if (strcmp(tree->token, "WHILE") == 0)
    {
        generateWhile(tree);
        return;
    }
    if (strcmp(tree->token, "FOR") == 0)
    {
        generateFor(tree);
        return;
    }
    if (strcmp(tree->token, "BODY") == 0)
    {
        generateBody(tree);
        return;
    }

}

static char* generateExpression(node* tree)
{
    char* leftPlace;
    char* rightPlace;
    char* result;

    if (tree == NULL)
    {
        return copyString("");
    }

    if (strcmp(tree->token, "&&") == 0 || strcmp(tree->token, "||") == 0)
    {
        return generateLogicalExpression(tree);
    }

    if (isLeaf(tree))
    {
        if (isLiteralToken(tree->token))
        {
            result = newTemp();
            emitAssign(result, tree->token);
            return result;
        }

        return copyString(tree->token);
    }

    if (isBinaryOperator(tree->token))
    {
        leftPlace = generateExpression(tree->left);
        rightPlace = generateExpression(tree->right);

        result = newTemp();
        emitBinary(result, leftPlace, tree->token, rightPlace);

        return result;
    }
    if (strcmp(tree->token, "CALL") == 0)
{
    return generateCall(tree);
}

    if (strcmp(tree->token, "!") == 0)
    {
        leftPlace = generateExpression(tree->left);

        result = newTemp();
        emitUnary(result, "!", leftPlace);

        return result;
    }

    if (strcmp(tree->token, "^") == 0)
    {
        leftPlace = generateExpression(tree->left);

        result = newTemp();
        emitUnary(result, "*", leftPlace);

        return result;
    }

    if (strcmp(tree->token, "&") == 0)
    {
        leftPlace = generateExpression(tree->left);

        result = newTemp();
        emitUnary(result, "&", leftPlace);

        return result;
    }

    return copyString(tree->token);
}
static int generateArguments(node* tree)
{
    char* argPlace;
    int count;

    if (tree == NULL)
    {
        return 0;
    }

    if (isEmptyNode(tree))
    {
        count = generateArguments(tree->left);
        count += generateArguments(tree->right);
        return count;
    }

    argPlace = generateExpression(tree);
    emitPushParam(argPlace);

    return 1;
}

static char* generateCall(node* tree)
{
    char* result;
    int paramCount;

    if (tree == NULL)
    {
        return copyString("");
    }

    paramCount = generateArguments(tree->right);

    result = newTemp();

    if (tree->left != NULL)
    {
        emitCallResult(result, tree->left->token);
        emitPopParams(paramCount * 8);
    }

    return result;
}

static char* generateLogicalExpression(node* tree)
{
    char* leftPlace;
    char* rightPlace;
    char* result;

    char* firstLabel;
    char* secondLabel;
    char* thirdLabel;
    char* endLabel;

    if (tree == NULL)
    {
        return copyString("");
    }

    if (strcmp(tree->token, "&&") == 0)
    {
        firstLabel = newLabel();
        secondLabel = newLabel();
        thirdLabel = newLabel();
        endLabel = newLabel();

        leftPlace = generateExpression(tree->left);

        emitIfGoto(leftPlace, firstLabel);
        emitGoto(secondLabel);

        emitLabel(firstLabel);

        rightPlace = generateExpression(tree->right);

        emitIfGoto(rightPlace, thirdLabel);
        emitGoto(secondLabel);

        result = newTemp();

        emitLabel(secondLabel);
        emitAssign(result, "false");
        emitGoto(endLabel);

        emitLabel(thirdLabel);
        emitAssign(result, "true");

        emitLabel(endLabel);

        return result;
    }

    if (strcmp(tree->token, "||") == 0)
    {
        firstLabel = newLabel();
        secondLabel = newLabel();
        thirdLabel = newLabel();
        endLabel = newLabel();

        leftPlace = generateExpression(tree->left);

        emitIfGoto(leftPlace, thirdLabel);
        emitGoto(firstLabel);

        emitLabel(firstLabel);

        rightPlace = generateExpression(tree->right);

        emitIfGoto(rightPlace, thirdLabel);
        emitGoto(secondLabel);

        result = newTemp();

        emitLabel(secondLabel);
        emitAssign(result, "false");
        emitGoto(endLabel);

        emitLabel(thirdLabel);
        emitAssign(result, "true");

        emitLabel(endLabel);

        return result;
    }

    return generateExpression(tree);
}

static char* generateLValue(node* tree)
{
    char* base;
    char* indexPlace;
    char* result;

    if (tree == NULL)
    {
        return copyString("");
    }

    if (isLeaf(tree))
    {
        return copyString(tree->token);
    }

    if (strcmp(tree->token, "^") == 0)
    {
        base = generateExpression(tree->left);

        result = (char*)malloc(strlen(base) + 2);
        strcpy(result, "*");
        strcat(result, base);

        return result;
    }

    if (strcmp(tree->token, "INDEX") == 0)
    {
        indexPlace = generateExpression(tree->right);

        result = (char*)malloc(strlen(tree->left->token) + strlen(indexPlace) + 4);
        strcpy(result, tree->left->token);
        strcat(result, "[");
        strcat(result, indexPlace);
        strcat(result, "]");

        return result;
    }

    return generateExpression(tree);
}

static void generateAssignment(node* tree)
{
    char* leftPlace;
    char* rightPlace;

    if (tree == NULL)
    {
        return;
    }

    leftPlace = generateLValue(tree->left);
    rightPlace = generateExpression(tree->right);

    emitAssign(leftPlace, rightPlace);
}

static void generateReturn(node* tree)
{
    char* returnPlace;

    if (tree == NULL)
    {
        return;
    }

    returnPlace = generateExpression(tree->left);

    emitReturnValue(returnPlace);
}

static void generateIf(node* tree)
{
    char* conditionPlace;
    char* trueLabel;
    char* endLabel;

    if (tree == NULL)
    {
        return;
    }

    conditionPlace = generateExpression(tree->left);
    trueLabel = newLabel();
    endLabel = newLabel();

    emitIfGoto(conditionPlace, trueLabel);
    emitGoto(endLabel);

    emitLabel(trueLabel);
    generateStatement(tree->right);

    emitLabel(endLabel);
}

static void generateIfElse(node* tree)
{
    char* conditionPlace;
    char* trueLabel;
    char* falseLabel;
    char* endLabel;

    if (tree == NULL)
    {
        return;
    }

    conditionPlace = generateExpression(tree->left);
    trueLabel = newLabel();
    falseLabel = newLabel();
    endLabel = newLabel();

    emitIfGoto(conditionPlace, trueLabel);
    emitGoto(falseLabel);

    emitLabel(trueLabel);
    generateStatement(tree->right->left);
    emitGoto(endLabel);

    emitLabel(falseLabel);
    generateStatement(tree->right->right);

    emitLabel(endLabel);
}

static void generateWhile(node* tree)
{
    char* startLabel;
    char* bodyLabel;
    char* endLabel;
    char* conditionPlace;

    if (tree == NULL)
    {
        return;
    }

    startLabel = newLabel();
    bodyLabel = newLabel();
    endLabel = newLabel();

    emitLabel(startLabel);

    conditionPlace = generateExpression(tree->left);

    emitIfGoto(conditionPlace, bodyLabel);
    emitGoto(endLabel);

    emitLabel(bodyLabel);
    generateStatement(tree->right);
    emitGoto(startLabel);

    emitLabel(endLabel);
}

static void generateFor(node* tree)
{
    node* initNode;
    node* conditionNode;
    node* updateNode;
    node* bodyNode;

    char* startLabel;
    char* bodyLabel;
    char* endLabel;
    char* conditionPlace;

    if (tree == NULL)
    {
        return;
    }

    initNode = tree->left;
    conditionNode = tree->right->left;
    updateNode = tree->right->right->left;
    bodyNode = tree->right->right->right;

    generateAssignment(initNode);

    startLabel = newLabel();
    bodyLabel = newLabel();
    endLabel = newLabel();

    emitLabel(startLabel);

    conditionPlace = generateExpression(conditionNode);

    emitIfGoto(conditionPlace, bodyLabel);
    emitGoto(endLabel);

    emitLabel(bodyLabel);
    generateStatement(bodyNode);
    generateAssignment(updateNode);
    emitGoto(startLabel);

    emitLabel(endLabel);
}