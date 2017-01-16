#include "compiler.h"

symrec * sym_table = (symrec*)0;
reg * registers = (reg*)0;
mem * memory = (mem*)0;
command * commands = (command*)0;
FILE * output = NULL;

int currentMemoryLength = 0;
int currentCommandsLength = 0;
int k = 0;

int multCount = 0;
int divCount = 0;
int ifCount = 0;

bool isIf = false;
bool isWhile = false;

void initRegisters()
{
  registers = malloc(sizeof(reg) * regAmount);
  memory = malloc(sizeof(mem) * memoryLength);  
  commands = malloc(sizeof(command) * commandsAmount);
  currentMemoryLength = memoryLength;
  currentCommandsLength = commandsAmount;

  //writeToFile("ZERO 0");
  saveCommand("ZERO", 0, -1, 0, 0, 0);
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
  //writeToFile("HALT");
  saveCommand("HALT", -1, -1, 0, 0, 0);
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

void removeVariable(char * name)
{
  symrec * ptr = sym_table;
  while (ptr->next != (symrec*)0)
  {
    if (strcmp(ptr->next->name, name) == 0)
    {
      symrec * toRemove = ptr->next;
      ptr->next = toRemove->next;
      free(toRemove);
      break;
    }
  }
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

  //writeToFile("GET 1");
  saveCommand("GET", 1, -1, 0, 0, 0);

  if (!memory[registers[0].value].isUsed)
  {
   // writeToFile("STORE 1");
    saveCommand("STORE", 1, -1, 0, 0, 0);
    memory[registers[0].value].isUsed = true;
    var->initialized = true;
    var->memoryPosition = registers[0].value;
  }
  else 
  {
    while (memory[registers[0].value].isUsed)
    {
     // writeToFile("INC 0");
      saveCommand("INC", 0, -1, 0, 0, 0);
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
   // writeToFile("STORE 1");
    saveCommand("STORE", 1, -1, 0, 0, 0);
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
     // writeToFile("LOAD 1");
      saveCommand("LOAD", 1, -1, 0, 0, 0);
     // writeToFile("PUT 1");
      saveCommand("PUT", 1, -1, 0, 0, 0);
      return;
    }
  }

  changeRegValueTo(1, val);
 // writeToFile("PUT 1");
  saveCommand("PUT", 1, -1, 0, 0, 0);
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
    printf("ASSIGN VARIABLE AS %llu\n", from->value);
    changeRegValueTo(2, from->value);
  }
  else
  {
    changeAccumlatorPositionToVar(from);
   // writeToFile("LOAD 2");
    saveCommand("LOAD", 2, -1, 0, 0, 0);
  } 

  if (!to->initialized)
  {
    saveVariableToMemory(to);
  }
  changeAccumlatorPositionToVar(to);
  //writeToFile("STORE 2");
  saveCommand("STORE", 2, -1, 0, 0, 0);
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
  val1->value += val2->value;
  return val1;
}

symrec * addVariableAndNumber(symrec * var, symrec * val)
{
  symrec * result = getVariable("result");
  if (result == 0)
  {
    result = addVariable("result");
    saveVariableToMemory(result);
  }
  changeAccumlatorPositionToVar(var);
  changeRegValueTo(1, val->value);
//  writeToFile("ADD 1");
  saveCommand("ADD", 1, -1, 0, 0, 0);
  changeAccumlatorPositionToVar(result);
//  writeToFile("STORE 1");
  saveCommand("STORE", 1, -1, 0, 0, 0);
  return result;
}

symrec * addVariables(symrec * var1, symrec * var2)
{
  symrec * result = getVariable("result");
  if (result == 0)
  {
    result = addVariable("result");
    saveVariableToMemory(result);
  }
  changeAccumlatorPositionToVar(var1);
  //writeToFile("LOAD 1");
  saveCommand("LOAD", 1, -1, 0, 0, 0);
  changeAccumlatorPositionToVar(var2);
  //writeToFile("ADD 1");
  saveCommand("ADD", 1, -1, 0, 0, 0);
  changeAccumlatorPositionToVar(result);
 // writeToFile("STORE 1");
  saveCommand("STORE", 1, -1, 0, 0, 0);
  return result;
}

symrec * performSubtraction(symrec * a, symrec * b)
{
  if (a->isValue && b->isValue)
    return subtractNumbers(a, b);

  if (a->isVariable && b->isVariable)
    return subtractVariables(a, b);

  if (a->isValue && b->isVariable)
    return subtractNumberAndVariable(a, b);

  if (a->isVariable && b->isValue)
    return subtractVariableAndNumber(a, b);

  return 0;
}

symrec * subtractNumbers(symrec * val1, symrec * val2)
{
  val1->value -= val2->value;
  return val1;
}

symrec * subtractVariableAndNumber(symrec * var, symrec * val)
{
  symrec * result = getVariable("result");
  if (result == 0)
  {
    result = addVariable("result");
    saveVariableToMemory(result);
  }
  changeRegValueTo(1, val->value);
  changeAccumlatorPositionToVar(result);
  //writeToFile("STORE 1");
  saveCommand("STORE", 1, -1, 0, 0, 0);
  changeAccumlatorPositionToVar(var);
 // writeToFile("LOAD 1");
  saveCommand("LOAD", 1, -1, 0, 0, 0);
  changeAccumlatorPositionToVar(result);
  //writeToFile("SUB 1");
  saveCommand("SUB", 1, -1, 0, 0, 0);
 // writeToFile("STORE 1");
  saveCommand("STORE", 1, -1, 0, 0, 0);
  return result;
}

symrec * subtractNumberAndVariable(symrec * val, symrec * var)
{
  symrec * result = getVariable("result");
  if (result == 0)
  {
    result = addVariable("result");
    saveVariableToMemory(result);
  }
  changeAccumlatorPositionToVar(var);
  changeRegValueTo(1, val->value);
  //writeToFile("SUB 1");
  saveCommand("SUB", 1, -1, 0, 0, 0);
  changeAccumlatorPositionToVar(result);
 // writeToFile("STORE 1");
  saveCommand("STORE", 1, -1, 0, 0, 0);
  return result;
}

symrec * subtractVariables(symrec * var1, symrec * var2)
{
  symrec * result = getVariable("result");
  if (result == 0)
  {
    result = addVariable("result");
    saveVariableToMemory(result);
  }
  changeAccumlatorPositionToVar(var1);
 // writeToFile("LOAD 1");
  saveCommand("LOAD", 1, -1, 0, 0, 0);
  changeAccumlatorPositionToVar(var2);
 // writeToFile("SUB 1");
  saveCommand("SUB", 1, -1, 0, 0, 0);
  changeAccumlatorPositionToVar(result);
 // writeToFile("STORE 1");
  saveCommand("STORE", 1, -1, 0, 0, 0);
  return result;
}

symrec * performMultiplication(symrec * a, symrec * b)
{
  if (a->isValue && b->isValue)
    return multNumbers(a, b);

  if (a->isVariable && b->isVariable)
    return multVariables(a, b);

  if (a->isValue && b->isVariable)
    return multVariableAndNumber(b, a);

  if (a->isVariable && b->isValue)
    return multVariableAndNumber(a, b);

  return 0;
}

symrec * multNumbers(symrec * val1, symrec * val2)
{
  val1->value *= val2->value;
  return val1;
}

symrec * multVariableAndNumber(symrec * var, symrec * val)
{
  symrec * result = getVariable("result");
  if (result == 0)
  {
    result = addVariable("result");
    saveVariableToMemory(result);
  }
  changeRegValueTo(1, val->value);
  saveCommand("LOAD", 3, -1, 0, 0, 0);
  saveCommand("ZERO", 3, -1, 0, 0, 0);

  changeAccumlatorPositionToVar(var);
  saveCommand("LOAD", 2, -1, 0, 0, 0);

  char * buf = malloc(25);
  sprintf(buf, "startmult%d", multCount);
  char * buf2 = malloc(25);
  sprintf(buf2, "endmult%d", multCount);
  saveCommand("JZERO", 1, -1, buf, buf2, 0);
  sprintf(buf, "addToResult%d", multCount);
  saveCommand("JODD", 1, -1, 0, buf, 0);
  saveCommand("SHR", 1, -1, 0, 0, 0);
  saveCommand("SHL", 2, -1, 0, 0, 0);
  saveCommand("STORE", 2, -1, 0, 0, 0);
  sprintf(buf, "startmult%d", multCount);
  saveCommand("JUMP", -1, -1, 0, buf, 0);
  sprintf(buf, "addToResult%d", multCount);
  saveCommand("ADD", 3, -1, buf, 0, 0);
  saveCommand("SHR", 1, -1, 0, 0, 0);
  saveCommand("SHL", 2, -1, 0, 0, 0);
  saveCommand("STORE", 2, -1, 0, 0, 0);
  sprintf(buf, "startmult%d", multCount);
  sprintf(buf2, "endmult%d", multCount);
  saveCommand("JUMP", -1, -1, 0, buf, buf2);

  changeAccumlatorPositionToVar(result);
  saveCommand("STORE", 3, -1, 0, 0, 0);
  multCount++;
  return result;
}

symrec * multVariables(symrec * var1, symrec * var2)
{
  changeAccumlatorPositionToVar(var1);
  saveCommand("LOAD", 1, -1, 0, 0, 0);

  symrec * result = getVariable("result");
  if (result == 0)
  {
    result = addVariable("result");
    saveVariableToMemory(result);
  }
  changeAccumlatorPositionToVar(result);
  saveCommand("LOAD", 3, -1, 0, 0, 0);
  saveCommand("ZERO", 3, -1, 0, 0, 0);

  changeAccumlatorPositionToVar(var2);
  saveCommand("LOAD", 2, -1, 0, 0, 0);

  char * buf = malloc(25);
  sprintf(buf, "startmult%d", multCount);
  char * buf2 = malloc(25);
  sprintf(buf2, "endmult%d", multCount);
  saveCommand("JZERO", 1, -1, buf, buf2, 0);
  sprintf(buf, "addToResult%d", multCount);
  saveCommand("JODD", 1, -1, 0, buf, 0);
  saveCommand("SHR", 1, -1, 0, 0, 0);
  saveCommand("SHL", 2, -1, 0, 0, 0);
  saveCommand("STORE", 2, -1, 0, 0, 0);
  sprintf(buf, "startmult%d", multCount);
  saveCommand("JUMP", -1, -1, 0, buf, 0);
  sprintf(buf, "addToResult%d", multCount);
  saveCommand("ADD", 3, -1, buf, 0, 0);
  saveCommand("SHR", 1, -1, 0, 0, 0);
  saveCommand("SHL", 2, -1, 0, 0, 0);
  saveCommand("STORE", 2, -1, 0, 0, 0);
  sprintf(buf, "startmult%d", multCount);
  sprintf(buf2, "endmult%d", multCount);
  saveCommand("JUMP", -1, -1, 0, buf, buf2);

  changeAccumlatorPositionToVar(result);
  saveCommand("STORE", 3, -1, 0, 0, 0);
  multCount++;
  return result;
}

symrec * performDivision(symrec * a, symrec * b, bool getModulo)
{
  if (a->isValue && b->isValue)
    return divideNumbers(a, b, getModulo);

  if (a->isVariable && b->isVariable)
    return divideVariables(a, b, getModulo);

  if (a->isValue && b->isVariable)
    return divideNumberAndVariable(a, b, getModulo);

  if (a->isVariable && b->isValue)
    return divideVariableAndNumber(a, b, getModulo);

  return 0;
}

symrec * divideNumbers(symrec * val1, symrec * val2, bool getModulo)
{
  if (getModulo)
    val1->value %= val2->value;
  else
    val1->value /= val2->value;
  return val1;
}

symrec * divideVariableAndNumber(symrec * var, symrec * val, bool getModulo)
{
  symrec * temp = getVariable("temp1");
  if (temp == 0)
  {
    temp = addVariable("temp1");
    saveVariableToMemory(temp);
  }
  changeRegValueTo(1, val->value);
  changeAccumlatorPositionToVar(temp);
  saveCommand("STORE", 1, -1, 0, 0, 0);
  return divideVariables(var, temp, getModulo);
}

symrec * divideNumberAndVariable(symrec * val, symrec * var, bool getModulo)
{
  symrec * temp = getVariable("temp1");
  if (temp == 0)
  {
    temp = addVariable("temp1");
    saveVariableToMemory(temp);
  }
  changeRegValueTo(1, val->value);
  changeAccumlatorPositionToVar(temp);
  saveCommand("STORE", 1, -1, 0, 0, 0);
  return divideVariables(temp, var, getModulo);
}

symrec * divideVariables(symrec * var1, symrec * var2, bool getModulo)
{
  symrec * result = getVariable("result");
  if (result == 0)
  {
    result = addVariable("result");
    saveVariableToMemory(result);
  }

  symrec * mult = getVariable("mult");
  if (mult == 0)
  {
    mult = addVariable("mult");
    saveVariableToMemory(mult);
  }

  symrec * dividend = getVariable("var1");
  if (dividend == 0)
  {
    dividend = addVariable("var1");
    saveVariableToMemory(dividend);
  }

  symrec * divisor = getVariable("var2");
  if (divisor == 0)
  {
    divisor = addVariable("var2");
    saveVariableToMemory(divisor);
  }

  changeAccumlatorPositionToVar(var1);
  saveCommand("LOAD", 1, -1, 0, 0, 0);
  changeAccumlatorPositionToVar(dividend);
  saveCommand("STORE", 1, -1, 0, 0, 0);
  changeAccumlatorPositionToVar(var2);
  saveCommand("LOAD", 1, -1, 0, 0, 0);
  changeAccumlatorPositionToVar(divisor);
  saveCommand("STORE", 1, -1, 0, 0, 0);
  /*
  regs:
  1 - a
  2 - b 
  3 - mult
  4 - result, addition and subtraction results
  */
  changeAccumlatorPositionToVar(result);
  saveCommand("LOAD", 4, -1, 0, 0, 0);
  saveCommand("ZERO", 4, -1, 0, 0, 0);
  saveCommand("STORE", 4, -1, 0, 0, 0);
  changeAccumlatorPositionToVar(dividend);
  saveCommand("LOAD", 1, -1, 0, 0, 0);
  saveCommand("LOAD", 4, -1, 0, 0, 0);
  changeAccumlatorPositionToVar(mult);
  saveCommand("LOAD", 3, -1, 0, 0, 0);
  saveCommand("ZERO", 3, -1, 0, 0, 0);
  saveCommand("INC", 3, -1, 0, 0, 0);
  changeAccumlatorPositionToVar(divisor);
  saveCommand("LOAD", 2, -1, 0, 0, 0);

  /*
  while (a < b)
  {
    a <<= 1;
    mult <<= 1;
  }
  */
  char * buf = malloc(15);
  sprintf(buf, "end%d", divCount);
  saveCommand("JZERO", 2, -1, 0, buf, 0);
  sprintf(buf, "step1%d", divCount);
  saveCommand("SUB", 4, -1, buf, 0, 0);
  sprintf(buf, "step2%d", divCount);
  saveCommand("JZERO", 4, -1, 0, buf, 0);
  saveCommand("SHL", 2, -1, 0, 0, 0);
  changeAccumlatorPositionToVar(dividend);
  saveCommand("LOAD", 4, -1, 0, 0, 0);
  changeAccumlatorPositionToVar(divisor);
  saveCommand("STORE", 2, -1, 0, 0, 0);
  saveCommand("SHL", 3, -1, 0, 0, 0);
  changeAccumlatorPositionToVar(mult);
  saveCommand("STORE", 3, -1, 0, 0, 0);
  changeAccumlatorPositionToVar(divisor);
  sprintf(buf, "step1%d", divCount);
  saveCommand("JUMP", -1, -1, 0, buf, 0);
  /*
  do
  {
    if (a >= b)
    {
      a -= b
      result += mult
    }
    b >>= 1
    mult >>= 1
  } while (mult != 0)
  */
  sprintf(buf, "step2%d", divCount);
  saveCommand("LOAD", 4, -1, buf, 0, 0);
  changeAccumlatorPositionToVar(dividend);
  saveCommand("SUB", 4, -1, 0, 0, 0);
  changeAccumlatorPositionToVar(result);  //it needs to be here 
  sprintf(buf, "calcNew%d", divCount);
  saveCommand("JZERO", 4, -1, 0, buf, 0);
  sprintf(buf, "inc%d", divCount);
  char * buf2 = malloc(15);
  sprintf(buf2, "calcNew%d", divCount);
  saveCommand("JUMP", -1, -1, 0, buf, buf2);
  changeAccumlatorPositionToVar(divisor);
  saveCommand("SUB", 1, -1, 0, 0, 0);
  changeAccumlatorPositionToVar(dividend);
  saveCommand("STORE", 1, -1, 0, 0, 0);
  changeAccumlatorPositionToVar(result);
  saveCommand("LOAD", 4, -1, 0, 0, 0);
  changeAccumlatorPositionToVar(mult);
  saveCommand("ADD", 4, -1, 0, 0, 0);
  changeAccumlatorPositionToVar(result);
  saveCommand("STORE", 4, -1, 0, 0, 0);
  sprintf(buf, "inc%d", divCount);
  saveCommand("SHR", 2, -1, buf, 0, 0);
  saveCommand("SHR", 3, -1, 0, 0, 0);
  changeAccumlatorPositionToVar(mult);
  saveCommand("STORE", 3, -1, 0, 0, 0);
  changeAccumlatorPositionToVar(divisor);
  saveCommand("STORE", 2, -1, 0, 0, 0);
  sprintf(buf, "end%d", divCount);
  saveCommand("JZERO", 3, -1, 0, buf, 0);
  sprintf(buf, "step2%d", divCount);
  sprintf(buf2, "end%d", divCount);
  saveCommand("JUMP", -1, -1, 0, buf, buf2);
  divCount++;
  if (getModulo)
    return dividend;
  return result;
}

void afterFirstCond()
{
  char * buf = malloc(15);
  sprintf(buf, "endcond", ifCount);
  char * buf2 = malloc(15);
  sprintf(buf2, "elseif%d", ifCount);
  printf("WRITING JUUMP INS LABEL %s\n", buf);
  saveCommand("JUMP", -1, -1, 0, buf, buf2);
}

void afterSecondCond()
{
  command * temp = commands;
  while (temp->next != (command*)0)
  {
      if (strcmp(temp->label, "endcond") == 0)
          strcpy(temp->label, "\0");
      temp = temp->next;
  }
  temp->nextlineLabel = malloc(8);
  char * buf = malloc(15);
  sprintf(buf, "endcond", ifCount);
  strcpy(temp->nextlineLabel, buf);
  printf("WRITING AFTER %s INS LABEL %s\n", temp->name, temp->nextlineLabel);
}

void greaterOrEqual(symrec * a, symrec * b)
{
  changeAccumlatorPositionToVar(b);
  saveCommand("LOAD", 1, -1, 0, 0, 0);
  changeAccumlatorPositionToVar(a);
  saveCommand("SUB", 1, -1, 0, 0, 0);
  char * buf = malloc(15);
  char * buf2 = malloc(15);
  if (isIf)
  {
    sprintf(buf, "operations%d", ifCount);
    sprintf(buf2, "elseif%d", ifCount);
  }
  else 
  {
    //while
  }
  saveCommand("JZERO", 1, -1, 0, buf, 0);
  saveCommand("JUMP", -1, -1, 0, buf2, buf);
}

void greater(symrec * a, symrec * b)
{
  changeAccumlatorPositionToVar(a);
  saveCommand("LOAD", 1, -1, 0, 0, 0);
  changeAccumlatorPositionToVar(b);
  saveCommand("SUB", 1, -1, 0, 0, 0);
  char * buf = malloc(15);
  if (isIf)
  {
    sprintf(buf, "elseif%d", ifCount);
  }
  else 
  {

  }
  saveCommand("JZERO", 1, -1, 0, buf, 0);
}

void different(symrec * a, symrec * b)
{
  //a - b != 0 || b - a != 0
  changeAccumlatorPositionToVar(a);
  saveCommand("LOAD", 1, -1, 0, 0, 0);
  changeAccumlatorPositionToVar(b);
  saveCommand("SUB", 1, -1, 0, 0, 0);
  char * buf = malloc(15);
  char * buf2 = malloc(15);
  if (isIf)
  {
    sprintf(buf, "orcond%d", ifCount);
    sprintf(buf2, "operations%d", ifCount);
  }
  else 
  {

  }
  saveCommand("JZERO", 1, -1, 0, buf, 0);
  saveCommand("JUMP", -1, -1, 0, buf2, 0);

  saveCommand("LOAD", 1, -1, buf, 0, 0);
  changeAccumlatorPositionToVar(a);
  saveCommand("SUB", 1, -1, 0, 0, 0);
  if (isIf)
  {
    sprintf(buf, "elseif%d", ifCount);
  }
  else 
  {

  }
  saveCommand("JZERO", 1, -1, 0, buf, buf2);
}

void equal(symrec * a, symrec * b)
{
  changeAccumlatorPositionToVar(a);
  saveCommand("LOAD", 1, -1, 0, 0, 0);
  changeAccumlatorPositionToVar(b);
  saveCommand("SUB", 1, -1, 0, 0, 0);
  char * buf = malloc(15);
  char * buf2 = malloc(15);
  if (isIf)
  {
    sprintf(buf, "andcond%d", ifCount);
    sprintf(buf2, "elseif%d", ifCount);
  }
  else 
  {

  }
  saveCommand("JZERO", 1, -1, 0, buf, 0);
  saveCommand("JUMP", -1, -1, 0, buf2, 0);
  saveCommand("LOAD", 1, -1, buf, 0, 0);
  changeAccumlatorPositionToVar(a);
  if (isIf)
  {
    sprintf(buf, "operations%d", ifCount);
  }
  else 
  {

  }
  saveCommand("SUB", 1, -1, 0, 0, 0);
  saveCommand("JZERO", 1, -1, 0, buf, 0);
  saveCommand("JUMP", -1, -1, 0, buf2, buf);
}

void printVarError(char * error, char * name)
{
  char * str = malloc(2 + strlen(error) + strlen(name));
  strcpy(str, error);
  strcat(str, " ");
  strcat(str, name);
  yyerror(str);
}

void printValueError(char * error, ull value)
{
  int errorLength = strlen(error);
  char * buf = malloc(22 + errorLength);
  sprintf(buf, "%s %llu", error, value);
  yyerror(buf);
}

void writeToFile(char * text)
{
  char * str = malloc(2 + strlen(text));
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

void saveVariableToMemory(symrec * var)
{
  ull reg0Value = registers[0].value;
  if (!memory[registers[0].value].isUsed)
  {
    memory[registers[0].value].isUsed = true;
    var->initialized = true;
    var->memoryPosition = registers[0].value;
  }
  else 
  {
    while (memory[registers[0].value].isUsed)
    {
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
    var->initialized = true;
    var->memoryPosition = registers[0].value;
  }
  registers[0].value = reg0Value;
}

void changeAccumlatorPositionToVar(symrec * var)
{
  saveCommand("ZERO", 0, -1, 0, 0, 0);
  registers[0].value = 0;
  printf("CHANGING ACCUMLATOR FROM %llu TO %d\n", registers[0].value, var->memoryPosition);
  while (registers[0].value != var->memoryPosition)
  {
    if (registers[0].value > var->memoryPosition)
    {
     // writeToFile("DEC 0");
      saveCommand("DEC", 0, -1, 0, 0, 0);
      registers[0].value--;
    }
    else 
    {
     // writeToFile("INC 0");
      saveCommand("INC", 0, -1, 0, 0, 0);
      registers[0].value++;
    }
  }
}

void changeRegValueTo(int reg, ull value)
{
  //char buf[7];
  //sprintf(buf, "ZERO %d", reg);
  //writeToFile(buf);
  saveCommand("ZERO", reg, -1, 0, 0, 0);

  int currentSize = 20;
  ull * tab = (ull*)malloc(sizeof(ull) * currentSize);
  int i = 0;
  while (value > 0)
  {
    if (i == currentSize - 1)
    {
        ull * moreSpace = realloc(tab, 2 * currentSize * sizeof(ull));
        tab = moreSpace;
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
     // sprintf(buf, "SHL %d", reg);
     // writeToFile(buf);
      saveCommand("SHL", reg, -1, 0, 0, 0);
    }
    else 
    {
     // sprintf(buf, "INC %d", reg);
     // writeToFile(buf);
      saveCommand("INC", reg, -1, 0, 0, 0);
    }
    --i;
  }
}

void saveCommand(char * name, int arg1, int arg2, char * label, char * tolabel, char * nextlineLabel)
{
  if (k == currentCommandsLength)
  {
    command * moreSpace = realloc(commands, 2 * currentCommandsLength * sizeof(command));
    commands = moreSpace;
    currentCommandsLength = 2 * currentCommandsLength;
  }
  command * com = malloc(sizeof(command));
  com->name = malloc(sizeof(name) + 1);
  strcpy(com->name, name);
  com->arg1 = arg1;
  com->arg2 = arg2;
  if (label != 0)
  {
    com->label = malloc(sizeof(label) + 1);
    strcpy(com->label, label);
  } 
  else 
  {
    com->label = malloc(1);
    strcpy(com->label, "\0");
  }
  if (tolabel != 0)
  {
    com->tolabel = malloc(sizeof(tolabel) + 1);
    strcpy(com->tolabel, tolabel);
  }
  else 
  {
    com->tolabel = malloc(1);
    strcpy(com->tolabel, "\0");
  }
  if (nextlineLabel != 0)
  {
    com->nextlineLabel = malloc(sizeof(nextlineLabel) + 1);
    strcpy(com->nextlineLabel, nextlineLabel);
  }
  else 
  {
    com->nextlineLabel = malloc(1);
    strcpy(com->nextlineLabel, "\0");
  }
  com->k = k;
  printf("%s %d\n", name, k);
  if (commands->name != 0)
  {
    command * temp = commands;
    while (temp->next != (command*)0)
      temp = temp->next;
    if (strcmp(temp->nextlineLabel, "") != 0)
    {
      if (strcmp(com->label, "") != 0)
        printf("WARNING!! OVERRIDING LABEL AT %d COMMAND %s\n", k, com->name);
      com->label = malloc(sizeof(temp->nextlineLabel) + 1);
      strcpy(com->label, temp->nextlineLabel);
    }
    com->next = 0;
    temp->next = com;
  }
  else 
  {
    com->next = 0;
    commands = com;
  }
  ++k;
}

void changeLabels()
{
  command * current = commands;
  while (current != (command*)0)
  {
    if (strcmp(current->label, "") != 0) 
    {
      int k = current->k;
      command * temp = commands;
      while (temp != (command*)0)
      {
        if (strcmp(temp->tolabel, current->label) == 0)
        {
          printf("Changing label %s at command %s to %d\n", temp->tolabel, temp->name, k);
          temp->tolabel = "";
          if (temp->arg1 == -1)
            temp->arg1 = k;
          else temp->arg2 = k;
        }
        temp = temp->next;
      }
      strcpy(current->label, "\0");
    }
    current = current->next;
  }
}

void writeCommands()
{
  output = fopen("output.txt", "w");
  command * temp = commands;
  while (temp != (command*)0)
  {
    char buf[10];
    if (temp->arg1 != -1 && temp->arg2 != -1)
      sprintf(buf, "%s %d %d", temp->name, temp->arg1, temp->arg2);
    else if (temp->arg1 != -1)
      sprintf(buf, "%s %d", temp->name, temp->arg1);
    else 
      sprintf(buf, "%s", temp->name);
    writeToFile(buf);
    temp = temp->next;

  }
  fclose(output);
  free(commands);
}