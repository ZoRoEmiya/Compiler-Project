#ifndef IR_H
#define IR_H

#include <stdio.h>
#include "ast.h"

void initIR(const char* outputFileName);
void closeIR();

char* newTemp();
char* newLabel();

void emit(const char* format, ...);

void generate3AC(node* root);

#endif