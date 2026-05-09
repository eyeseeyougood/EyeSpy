#include"StringFuncs.h"

#define CHARSIZE sizeof(char)

static int GetAllocation(const char* data)
{
	return strlen(data) + 1;
}

static char* ExpandString(char* s, int extraSpace)
{
	char* newData = realloc(s, GetAllocation(s)+extraSpace);

	if (newData == NULL)
	{
		printf("FAILED TO REALLOC DATA PTR!!!!");
		return NULL;
	}

	return newData;
}

static int GetLength(const char* s)
{
	return strlen(s);
}

char* CopyString(const char* s)
{
	int allocation = GetAllocation(s);

	char* result = malloc(allocation);

	memcpy(result, s, allocation);

	return result;
}

char* AddData(char* s, const char* newData)
{
	char* n = ExpandString(s, GetAllocation(newData));

	strcat(n, newData);

	return n;
}

bool EquateStrings(const char* s1, const char* s2)
{
	if (GetLength(s1) != GetLength(s2))
		return false;

	for (int i = 0; i < GetLength(s1); i++)
	{
		if (s1[i] != s2[i])
			return false;
	}

	return true;
}