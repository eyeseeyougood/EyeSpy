#ifndef __StringFuncs_h__
#define __StringFuncs_h__

#include<string.h>
#include<stdlib.h>
#include<stdbool.h>
#include<stdio.h>

char* CopyString(const char* s);
char* AddData(char* s, const char* newData);
bool EquateStrings(const char* s1, const char* s2);

#endif