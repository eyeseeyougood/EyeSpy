#include"StringViews.h"

#define CHARSIZE sizeof(char)


static int GetAllocation(const char* data)
{
	return (strlen(data) + 1) * CHARSIZE;
}

String* NewString(const char* data)
{
	String* result = malloc(sizeof(String));

	result->data = malloc(GetAllocation(data));
	strcpy(result->data, data);

	return result;
}

void DisposeString(String* string)
{
	free(string->data);
	free(string);
}

static void ExpandString(String* s, int extraSpace)
{
	char* newData = realloc(s->data, GetAllocation(s->data)+extraSpace);

	if (newData == NULL)
	{
		printf("FAILED TO REALLOC DATA PTR!!!!");
		return;
	}

	s->data = newData;
}

static int GetLength(String* s)
{
	return strlen(GetData(s));
}

static char GetChar(String* s, int index)
{
	return GetData(s)[index];
}

char* GetData(String* s)
{
	if (s->data[GetAllocation(s->data)/CHARSIZE-1] == 0)
	{
		return s->data;
	}

	int allocation = GetAllocation(s->data+1*CHARSIZE);
	char* result = malloc(allocation);
	memset(result, 0, allocation);
	strcpy(result, s->data);

	return result;
}

void AddData(String* s, const char* newData)
{
	ExpandString(s, GetAllocation(newData));

	strcat(s->data, newData);
}

void AddStrings(String* s1, String* s2)
{
	AddData(s1, s2->data);
}

bool EquateStrings(String* s1, String* s2)
{
	if (GetLength(s1) != GetLength(s2))
		return false;

	for (int i = 0; i < GetLength(s1); i++)
	{
		if (GetChar(s1, i) != GetChar(s2, i))
			return false;
	}

	return true;
}