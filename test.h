#define testAmount 10

struct test
{
	char * title;
	char * input;
	char * output;
};

typedef struct test test;

extern test * unittest;

void initTests();
void clearTests();