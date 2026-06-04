#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ir.h"

static FILE* ir_output = NULL;
static int temp_counter = 0;
static int label_counter = 1;

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

    emitLine("# 3AC generation started");
}