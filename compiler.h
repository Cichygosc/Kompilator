/*
TODO:
-reading again same variable should read to same memory position
!switch methods from == to strcmp and from = to strcpy!
!assign numbers (eg. a := 5;)
!division by zero
-removing variables
-make tab[n] working (currently ASSIGN, WRITE)
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

#define regAmount 5
#define memoryLength 100
#define commandsAmount 64

#define DEBUG false

typedef unsigned long long ull;

struct symrec
{
	char * name;
	char * currentElement;
	bool initialized;
	bool declared;
	bool isTable;
	bool isVariable;
	bool isValue;
	bool knownValue;
	int tabLength;
	int regNumber;
	ull memoryPosition;
	ull value;
	struct symrec * elements;
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

struct command
{
	char * name;
	int arg1;
	int arg2;
	int k;
	char * label;
	char * tolabel;
	char * nextlineLabel;
	struct command * next;
};

typedef struct symrec symrec;
typedef struct reg reg;
typedef struct mem mem;
typedef struct command command;

extern symrec * sym_table;
extern reg * registers;
extern mem * memory;
extern command * commands;
extern int ifCount;
extern int whileCount;
extern int forCount;
extern bool isIf;
extern bool isWhile;
extern bool readTab;

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

void printValueError(char * error, ull value);

/*
zapisuje podany tekst do pliku wyjściowego
*/
void writeToFile(char * text);

/*
zmienia wartość rejestru zerowego na miejsce w pamięci gdzie przechowywana jest wartość zmiennej var
*/
void changeAccumlatorPositionToVar(symrec * var);

/*
zmienia wartość rejestru na podaną wartość
*/
void changeRegValueTo(int reg, ull value);

/*
zapisuje instrukcję do listy instrukcji
*/
void saveCommand(char * name, int arg1, int arg2, char * label, char * toLabel, char * nextlineLabel);

/*
zamienia labele na numery linii
*/
void changeLabels(bool removeLabel);

void changeLabel(char * tolabel, char * label);

/*
zapisuje wszystkie instrukcje to pliku
*/
void writeCommands();

int getRegWithValue(ull value);
//////////////////////////////
//KONIEC METOD POMOCNICZYCH///
//////////////////////////////


symrec * createVariable(char * name);
symrec * addVariable(char * name);
symrec * addTable(char * name, int size);
symrec * createValue(ull value);
symrec * getVariable(char * name);
void removeVariable(char * name);
ull getVariableValue(char * name);
symrec * getVariableFromTable(char * tablename, char * varname);
symrec * getVariableFromTableByValue(char * tableName, int position);
symrec * readVariable(symrec * var);
void writeVariable(symrec * var);
void assignVariable(symrec * to, symrec * from);
void saveVariableToMemory(symrec * var);
void saveTableToMemory(symrec * tab);
void saveVariableToAssemblyMemory(symrec * var);

//////////////////////////////
//////ADDITION OPERATION//////
//////////////////////////////
symrec * performAddition(symrec * a, symrec * b);
symrec * addNumbers(symrec * val1, symrec * val2);
symrec * addVariableAndNumber(symrec * var, symrec * val);
symrec * addVariables(symrec * var1, symrec * var2);
//////////////////////////////
///END OF ADDITION OPERATION//
//////////////////////////////

//////////////////////////////
//////SUBTRACT OPERATION//////
//////////////////////////////
symrec * performSubtraction(symrec * a, symrec * b);
symrec * subtractNumbers(symrec * val1, symrec * val2);
symrec * subtractVariableAndNumber(symrec * var, symrec * val);
symrec * subtractNumberAndVariable(symrec * val, symrec * var);
symrec * subtractVariables(symrec * var1, symrec * var2);
//////////////////////////////
///END OF SUBTRACT OPERATION//
//////////////////////////////

//////////////////////////////
//////ADDITION OPERATION//////
//////////////////////////////
symrec * performMultiplication(symrec * a, symrec * b);
symrec * multNumbers(symrec * val1, symrec * val2);
symrec * multVariableAndNumber(symrec * var, symrec * val);
symrec * multVariables(symrec * var1, symrec * var2);
//////////////////////////////
///END OF ADDITION OPERATION//
//////////////////////////////

//////////////////////////////
//////DIVISION OPERATION//////
//////////////////////////////
symrec * performDivision(symrec * a, symrec * b, bool getModulo);
symrec * divideNumbers(symrec * val1, symrec * val2, bool getModulo);
symrec * divideVariableAndNumber(symrec * var, symrec * val, bool getModulo);
symrec * divideNumberAndVariable(symrec * val, symrec * var, bool getModulo);
symrec * divideVariables(symrec * var1, symrec * var2, bool getModulo);
//////////////////////////////
///END OF DIVISION OPERATION//
//////////////////////////////

void afterFirstCond();
void afterSecondCond();

void onWhile();
void afterWhile();
void forCond(char * identifier, symrec * from, symrec * to, bool downTo);
void afterFor(char * iter, bool downTo);

//////////////////////////////
//////////CONDITIONS//////////
//////////////////////////////
void performCondCheck(symrec * a, symrec * b, int condIndex);

void greaterOrEqual(symrec * a, symrec * b);
void greater(symrec * a, symrec * b);
void different(symrec * a, symrec * b);
void equal(symrec * a, symrec * b);
//////////////////////////////
///////END OF CONDITIONS//////
//////////////////////////////