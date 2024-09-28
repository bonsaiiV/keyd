#include "strutil.h"
#include "string.h"
#include <stdio.h>

int streq(char* str1, char* str2){
	if (str1 == NULL) return str2 == NULL;
	if (str2 == NULL) return 0;
	do{
		if (*str1 != *str2) return 0;
	} while (*str1++ != '\0' && *str2++ != '\0');
	return 1;
}


int run_test(char* tmp, char* expected_value[]){
	char* str = calloc(strlen(tmp) + 1, sizeof(char));
	char* mem = str;
	strcpy(str, tmp);
	char* tok;
	int i = 0;
	do {
		tok = nexttoken(&str, ' ');
		if(!streq(tok, expected_value[i])){
			free(mem);
			return 1;
		}
	} while (expected_value[i++]);
	free(mem);
	return 0;
}

int basic_test(void) {
	char* str = " 1 one 2 ";
	char* expected_value[] = {"1", "one", "2", NULL};
	return run_test(str, expected_value );
}

struct a_case{
	char* name;
	int (*call)(void);
};

struct a_case tests[] = {
	{"basic", &basic_test}
};

int main(int argc, char* argv[]) {
	if (argc < 2) return 1;
	int test_count = sizeof(tests)/sizeof(*tests);
	for (int i = 0; i < test_count; i++) {
		if (streq(argv[1], tests[i].name)) return tests[i].call();
	}
	return 1;
}
