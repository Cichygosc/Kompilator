#include "compiler.h"

symrec * sym_table = (symrec*)0;
reg * registers = (reg*)0;
mem * memory = (mem*)0;
FILE * output = NULL;

int currentMemoryLength = 0;

void initRegisters()
{
  registers = (reg*)malloc(sizeof(reg) * regAmount);
  memory = (mem*)malloc(sizeof(mem) * memoryLength);  
  currentMemoryLength = memoryLength;

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
  symrec * var = getVariable(name);
  if (var != 0)
  {
    printVarError("Ponowne zadeklarowanie zmiennej", name);
    return var;
  }

  var = (symrec*)malloc(sizeof(symrec));
  var->name = (char*)malloc(strlen(name) + 1);
  strcpy(var->name, name);
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

symrec * addTable(char * name, int size)
{
  symrec * var = getVariable(name);
  if (var != 0)
  {
    printVarError("Ponowne zdefiniowanie zmiennej", name);
    return var;
  }

  var = malloc(sizeof(symrec));
  var->elements = malloc(sizeof(symrec)  * size);
  var->name = (char*)malloc(sizeof(name) + 1);
  strcpy(var->name, name);
  var->initialized = false;
  var->declared = true;
  var->isTable = true;
  var->isVariable = false;
  var->isValue = false;
  var->tabLength = size;
  var->value = 0;
  var->regNumber = -1;
  var->memoryPosition = -1;
  var->next = (struct symrec *)sym_table;
  sym_table = var;

  int i = 0;
  char elemName[1] = "";
  for (i = 0; i < size; ++i)
  {
    var->elements[i].name = malloc(sizeof(elemName));
    strcpy(var->elements[i].name, elemName);
    var->elements[i].initialized = false;
    var->elements[i].declared = true;
    var->elements[i].isTable = false;
    var->elements[i].isVariable = true;
    var->elements[i].isValue = false;
    var->elements[i].tabLength = -1;
    var->elements[i].value = 0;
    var->elements[i].regNumber = -1;
    var->elements[i].memoryPosition = -1;
  }

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
  for (ptr = sym_table; ptr != (symrec*)0; ptr = (symrec*)ptr->next)
  {
    if (strcmp(ptr->name, name) == 0)
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
  symrec * table = getVariable(tableName);
  if (table == 0)
  {
    printVarError("Użycie niezadeklarowanej tablicy", tableName);
    table = addTable(tableName, position);
  }
  else if (!table->isTable)
  {
    printVarError("Niepoprawne użycie zmiennej", tableName);
    return 0;
  }

  return &table->elements[position];
}

symrec * readVariable(symrec * var)
{
  if (var == 0)
  {
    printVarError("Uzycie niezadeklarowanej zmiennej", var->name);
    var = addVariable(var->name);
  }

  writeToFile("GET 1");

  if (!memory[registers[0].value].isUsed)
  {
    writeToFile("STORE 1");
    memory[registers[0].value].isUsed = true;
    var->initialized = true;
    var->memoryPosition = registers[0].value;
  }
  else 
  {
    while (memory[registers[0].value].isUsed)
    {
      writeToFile("INC 0");
      registers[0].value++;
      if (currentMemoryLength <= registers[0].value)
      {
        mem * moreMemory = realloc(memory, 2 * currentMemoryLength * sizeof(mem));
        memory = moreMemory;
        int i;
        for (i = currentMemoryLength; i < 2 * currentMemoryLength; ++i)
          memory[i].isUsed = false;
        currentMemoryLength = 2 * currentMemoryLength;
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
    {
      //its table element
      if (strcmp(var->name, "") == 0)
        printVarError("Użycie niezainicjalizowanego elementu tablicy", "");
      else 
        printVarError("Użycie niezainicjalizowanej zmiennej", var->name);
      return;
    }
    else
    {
      changeAccumlatorPositionToVar(var);
      writeToFile("LOAD 1");
      writeToFile("PUT 1");
      return;
    }
  }

  changeRegValueTo(1, val);
  writeToFile("PUT 1");
}

void assignVariable(symrec * to, symrec * from)
{
  if (!to->declared)
  {
    printVarError("Użycie niezadeklarowanej zmiennej", to->name);
    to = addVariable(to->name);
  }
  if (!from->declared)
  {
    printVarError("Użycie niezadeklarowanej zmiennej", from->name);
    from = addVariable(from->name);
  }
  else if (!from->initialized)
  {
    printVarError("Użycie niezainicjalizowanej zmiennej", from->name);
  }
  if (to->isValue)
  {
    printValueError("Niepoprawne przypisanie", to->value);
  }

  if (from->isValue)
  {
    changeRegValueTo(2, from->value);
  }
  else
  {
    changeAccumlatorPositionToVar(from);
    writeToFile("LOAD 2");
  } 
  changeAccumlatorPositionToVar(to);
  writeToFile("STORE 2");
  to->initialized = true;
}

symrec * performAddition(symrec * a, symrec * b)
{
  if (a->isValue && b->isValue)
    return addNumbers(a, b);

  if (a->isVariable && b->isVariable)
    return addVariables(a, b);

  if (a->isValue && b->isVariable)
    return addVariableAndNumber(b, a);

  if (a->isVariable && b->isValue)
    return addVariableAndNumber(a, b);

  return 0;
}

symrec * addNumbers(symrec * val1, symrec * val2)
{
  return 0;
}

symrec * addVariableAndNumber(symrec * var, symrec * val)
{
  return 0;
}

symrec * addVariables(symrec * var1, symrec * var2)
{
  return 0;
}

void printVarError(char * error, char * name)
{
  char * str = (char *)malloc(2 + strlen(error) + strlen(name));
  strcpy(str, error);
  strcat(str, " ");
  strcat(str, name);
  yyerror(str);
}

void printValueError(char * error, ull value)
{
  int errorLength = strlen(error);
  char * buf = (char *)malloc(22 + errorLength);
  sprintf(buf, "%s %llu", error, value);
  writeToFile(buf);
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

void changeAccumlatorPositionToVar(symrec * var)
{
  while (registers[0].value != var->memoryPosition)
  {
    if (registers[0].value > var->memoryPosition)
    {
      writeToFile("DEC 0");
      registers[0].value--;
    }
    else 
    {
      writeToFile("INC 0");
      registers[0].value++;
    }
  }
}

void changeRegValueTo(int reg, ull value)
{
  char buf[7];
  sprintf(buf, "ZERO %d", reg);
  writeToFile(buf);

  int currentSize = 20;
  ull * tab = (ull*)malloc(sizeof(ull) * currentSize);
  int i = 0;
  while (value > 0)
  {
    if (i == currentSize - 1)
    {
        ull * moreSpace = realloc(tab, 2 * currentSize * sizeof(ull));
        tab = moreSpace;
        int i;
        currentSize = 2 * currentSize;
    }
    tab[i++] = value;
    if (value % 2 == 0)
      value /= 2;
    else 
      --value;
  }

  while (i > 0)
  {
    if (tab[i] * 2 == tab[i - 1])
    {
      sprintf(buf, "SHL %d", reg);
      writeToFile(buf);
    }
    else 
    {
      sprintf(buf, "INC %d", reg);
      writeToFile(buf);
    }
    --i;
  }
}