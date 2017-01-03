#include "test.h"
#include <stdlib.h>

test * unittest = (test*)0;

void initTests()
{
	unittest = (test*)malloc(sizeof(test) * testAmount);
	unittest[0].title = "WRITE NUMBER";
	unittest[0].input = "VAR BEGIN WRITE 5; END";
	unittest[0].output = "ZERO 0 \
						   ZERO 1 \
						   INC 1 \
						   SHL 1 \
						   SHL 1 \
						   INC 1 \
						   PUT 1 \
						   HALT";
}

void clearTests()
{
	free(unittest);
}