#include <stdio.h>
#include "xxhash.h"

#ifdef _WIN32
#include <stdio.h>
#include <io.h>
#include <string.h>
#pragma warning(disable:4996)
#define snprintf _snprintf
#endif

static unsigned long long calcul_hash(const void *buffer, size_t length)
{
	unsigned long long const seed = 0; /* or any other value */
	unsigned long long const hash = XXH64(buffer, length, seed);
	return hash;
}

int main()
{
	const char *bookuuid = "tidestopcalcul_hash*&^%$281927~!@!zbx14571twking"; //"booklist";
	char has128[64] = {0};
	{
		unsigned long long hash_value = calcul_hash((void *)bookuuid, strlen(bookuuid));
		snprintf(has128, 64, "%016llX", hash_value);
		printf("booklist = %s,hash_value=%llu,has128=%016llx\n", bookuuid, hash_value, hash_value);
	}
	return 0;
}
