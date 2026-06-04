#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ir.h"

static FILE* ir_output = NULL;
static int temp_counter = 0;
static int label_counter = 1;

static int isEmptyNode(node* tree);
static void generateNode(node* tree);
static void generateFunction(node* tree);
static void generateProcedure(node* tree);
static void generateBody(node* tree);

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

void generate3AC(node* root)
{
    if (root == NULL)
    {
        return;
    }

    generateNode(root);
}

static int isEmptyNode(node* tree)
{
    return tree != NULL && strcmp(tree->token, "") == 0;
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

    /* statements here */
}