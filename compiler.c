#include "compiler.h"

symrec * sym_table = (symrec*)0;
reg * registers = (reg*)0;
mem * memory = (mem*)0;
FILE * output = NULL;

void initRegisters()
{
  registers = (reg*)malloc(sizeof(reg) * regAmount);
  memory = (mem*)malloc(sizeof(mem) * memoryLength);  

  output = fopen("output.txt", "w");

  writeToFile("ZERO 0");
  registers[0].value = 0;
  registers[0].isInitialized = true;
  registers[0].isUsed = true;

  int i = 1;
  for (i = 1; i < 5; ++i)
  {
    registers[i].isInitialized = false;
    registers[i].isUsed = true;
  }

  for (i = 0; i < 100; ++i)
  {
    memory[i].isUsed = false;
  }
}

void clearAll()
{
  writeToFile("HALT");
  fclose(output);
  free(memory);
  free(registers);
  free(sym_table);
}

symrec * addVariable(char * name)
{
  printf("ADDING %s :)\n", name);
  symrec * var = getVariable(name);
  if (var != 0)
  {
    printVarError("Ponowne zadeklarowanie zmiennej", name);
    return var;
  }

  var = (symrec*)malloc(sizeof(symrec));
  var->name = name;
  var->initialized = false;
  var->declared = true;
  var->isTable = false;
  var->isVariable = true;
  var->isValue = false;
  var->tabLength = -1;
  var->value = 0;
  var->regNumber = -1;
  var->memoryPosition = -1;
  var->next = (struct symrec *)sym_table;
  sym_table = var;

  return var; 
}

symrec * createValue(ull value)
{
  
  symrec * var = (symrec*)malloc(sizeof(symrec));
  var->name = "";
  var->initialized = true;
  var->declared = true;
  var->isTable = false;
  var->isVariable = false;
  var->isValue = true;
  var->tabLength = -1;
  var->value = value;
  var->regNumber = -1;
  var->memoryPosition = -1;
  return var;
}

symrec * getVariable(char * name)
{
  symrec * ptr;
  for (ptr = sym_table; ptr != (symrec*)0; (symrec*)ptr->next)
  {
    if (ptr->name == name)
      return ptr;
  }
  return 0;
}

ull getVariableValue(char * name)
{
  symrec * var = getVariable(name);
  if (var == 0)
  {
    printVarError("Użycie niezadeklarowanej zmiennej", name);
    var = addVariable(name);
  }
  else if (!var->initialized)
  {
    printVarError("Użycie niezainicjalizowanej zmiennej", name);
    var->value = 0;
    var->initialized = true;
  }
  return var->value;
}

symrec * getVariableFromTable(char * tableName, int position)
{
  return 0;
}

symrec * readVariable(symrec * var)
{
  printf("READING %s\n", var->name);
  if (var == 0)
  {
    printVarError("Uzycie niezadeklarowanej zmiennej", var->name);
    var = addVariable(var->name);
  }

  writeToFile("GET 1");

  if (memory[registers[0].value].isUsed)
  {
    writeToFile("STORE 1");
    memory[registers[0].value].isUsed = true;
    var->initialized = true;
    var->memoryPosition = registers[0].value;
  }
  else 
  {
    int currentLength = sizeof(memory) / sizeof(memory[0]);
    while (memory[registers[0].value].isUsed)
    {
      writeToFile("INC 0\n");
      registers[0].value++;
      if (currentLength <= registers[0].value)
      {
        mem * moreMemory = realloc(memory, 2 * currentLength * sizeof(mem));
        memory = moreMemory;
        int i;
        for (i = currentLength; i < 2 * currentLength; ++i)
          memory[i].isUsed = false;
      }
    }
    memory[registers[0].value].isUsed = true;
    writeToFile("STORE 1");
    var->initialized = true;
    var->memoryPosition = registers[0].value;
  }

  return var;
}

void writeVariable(symrec * var)
{
  ull val = 0;
  if (var->isValue)
  {
    val = var->value;
  } 
  else if (var->isVariable)
  {
    if (!var->initialized)
      printVarError("Użycie niezainicjalizowanej zmiennej", var->name);
    val = var->value;
  }

  writeToFile("ZERO 1");

  ull * tab = (ull*)malloc(sizeof(ull) * 20);
  int i = 0;
  while (val > 0)
  {
    tab[i++] = val;
    if (val % 2 == 0)
      val /= 2;
    else 
      --val;
  }

  while (i > 0)
  {
    if (tab[i] * 2 == tab[i - 1])
      writeToFile("SHL 1");
    else writeToFile("INC 1");
    --i;
  }
  writeToFile("PUT 1");
}

void printVarError(char * error, char * name)
{
  char * str = (char *)malloc(2 + strlen(error) + strlen(name));
  strcpy(str, error);
  strcat(str, " ");
  strcat(str, name);
  yyerror(str);
}

void writeToFile(char * text)
{
  char * str = (char *)malloc(2 + strlen(text));
  strcpy(str, text);
  strcat(str, "\n");
  fprintf(output, str);
}

bool isNumber(char * value)
{
  ull val;
  char * next;
  val = strtoull(value, &next, 10);
  printf("%c\n", *next);
  if ((next == value) || (*next != '\0'))
    return false;
  return true;
}