#ifndef IR_H
#define IR_H

#include <stdio.h>
#include "ast.h"

void initIR(const char* outputFileName);
void closeIR();

char* newTemp();
char* newLabel();

void emitLine(const char* line);
void emitLabel(const char* label);
void emitFunctionHeader(const char* name);
void emitBeginFunc(int size);
void emitEndFunc();

void generate3AC(node* root);

#endif