#ifndef __StringViews_h__
#define __StringViews_h__

#include<string.h>
#include<stdlib.h>
#include<stdbool.h>
#include<stdio.h>

typedef struct
{
	char* data;
} String;

void DisposeString(String* string);
String* NewString(const char* data);
char* GetData(String* s);
void AddData(String* s, const char* newData);
void AddStrings(String* s1, String* s2);
bool EquateStrings(String* s1, String* s2);

#endif