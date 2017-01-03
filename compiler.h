/*
TODO:
-reading again same variable should read to same memory position

*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

#define regAmount 5
#define memoryLength 100

#define DEBUG true

typedef unsigned long long ull;

struct symrec
{
	char * name;
	bool initialized;
	bool declared;
	bool isTable;
	bool isVariable;
	bool isValue;
	int tabLength;
	int regNumber;
	ull memoryPosition;
	ull value;
	struct symrec * next;
};

struct reg
{
	ull value;
	bool isInitialized;
	bool isUsed;
};

struct mem
{
	ull value;
	bool isUsed;
};

typedef struct symrec symrec;
typedef struct reg reg;
typedef struct mem mem;

extern symrec * sym_table;
extern reg * registers;
extern mem * memory;

//////////////////////////////
/////POMOCNICZE METODY////////
//////////////////////////////
/*
inicjuje rejestry, pamięć oraz tworzy plik do, którego zapisywany jest kod wynikowy
*/
void initCompiler();

/*
czyści rejestry, pamięć, zamyka pliki
*/
void clearAll();

/*
sprawdza czy wartość jest liczbą
*/

bool isNumber(char * value);

/*
wypisuje tekst błędu
error - opis błędu
name  - nazwa zmiennej której błąd dotyczy
*/
void printVarError(char * error, char * name);

/*
zpisuje podany tekst do pliku wyjściowego
*/
void writeToFile(char * text);

//////////////////////////////
//KONIEC METOD POMOCNICZYCH///
//////////////////////////////


symrec * addVariable(char * name);
symrec * createValue(ull value);
symrec * getVariable(char * name);
ull getVariableValue(char * name);
symrec * getVariableFromTable(char * tableName, int position);
symrec * readVariable(symrec * var);
void writeVariable(symrec * var);