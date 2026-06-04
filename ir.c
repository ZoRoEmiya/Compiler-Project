#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
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

void emit(const char* format, ...)
{
    va_list args;

    if (ir_output == NULL)
    {
        return;
    }

    va_start(args, format);
    vfprintf(ir_output, format, args);
    fprintf(ir_output, "\n");
    va_end(args);
}

void generate3AC(node* root)
{
    if (root == NULL)
    {
        return;
    }

    emit("# 3AC generation started");
}