#include "compiler.h"

symrec * sym_table = (symrec*)0;
reg * registers = (reg*)0;
mem * memory = (mem*)0;
command * commands = (command*)0;

int multCount = 0;
int divCount = 0;
int ifCount = 0;
int whileCount = 0;
int forCount = 0;

bool isIf = false;
bool isWhile = false;
bool readTab = true;

extern int currentMemoryLength;
extern int currentCommandsLength;

void initRegisters()
{
  registers = malloc(sizeof(reg) * regAmount);
  memory = malloc(sizeof(mem) * memoryLength);  
  commands = malloc(sizeof(command) * commandsAmount);
  currentMemoryLength = memoryLength;
  currentCommandsLength = commandsAmount;

  //writeToFile("ZERO 0");

  int i = 0;
  for (i = 0; i < 5; ++i)
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

symrec * createVariable(char * name)
{
  symrec * var = (symrec*)malloc(sizeof(symrec));
  var->name = (char*)malloc(strlen(name) + 1);
  strcpy(var->name, name);
  var->initialized = false;
  var->declared = true;
  var->isTable = false;
  var->isVariable = true;
  var->isValue = false;
  var->knownValue = false;
  var->tabLength = -1;
  var->value = 0;
  var->regNumber = -1;
  var->memoryPosition = -1;
  return var;
}

symrec * addVariable(char * name)
{
  symrec * var = getVariable(name);
  if (var != 0)
  {
    printVarError("Ponowne zadeklarowanie zmiennej", name);
    return var;
  }

  var = createVariable(name);
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
  var->next = (struct symrec *)sym_table;
  var->memoryPosition = -1;
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
      memory[toRemove->memoryPosition].isUsed = false;
      free(toRemove);
      break;
    }
    ptr = ptr->next;
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

symrec * getVariableFromTable(char * tablename, char * varname)
{
  symrec * var = getVariable(varname);
  if (var == 0)
  {
    printVarError("Użycie niezadeklarowanej zmiennej", varname);
    var = addVariable(varname);
    saveVariableToMemory(var);
  }
  else if (!var->initialized)
  {
    printVarError("Użycie niezainicjalizowanej zmiennej", varname);
  }

  symrec * tab = getVariable(tablename);
  if (tab == 0)
  {
    printVarError("Użycie niezadeklarowanej tablicy", tablename);
    tab = addTable(tablename, 100);
  }
  else if (!tab->isTable)
  {
    printVarError("Niepoprawne użycie zmiennej", tablename);
    return 0;
  }

  if (tab->memoryPosition == -1)
  {
    saveTableToMemory(tab);
  }

  symrec * element = createVariable(tab->name);
  element->initialized = true;
  element->currentElement = malloc(strlen(varname) + 1);
  strcpy(element->currentElement, varname);
  return element;
}

symrec * getVariableFromTableByValue(char * tableName, int position)
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

  if (table->memoryPosition == -1)
  {
    saveTableToMemory(table);
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

  if (var->initialized)
  {
    changeAccumlatorPositionToVar(var);
    saveCommand("STORE", 1, -1, 0, 0, 0);
  }
  else
  {
    saveVariableToAssemblyMemory(var);
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

  bool assignKnownValue = false;
  ull value = 0;
  if (from->isValue)
  {
    int reg = getRegWithValue(from->value);
    changeRegValueTo(2, from->value);
    assignKnownValue = true;
    value = from->value;
  }
  else if (from->isVariable)
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
  to->knownValue = assignKnownValue;
  to->value = value;
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
  symrec * result = getVariable("result1");
  if (result == 0)
  {
    result = addVariable("result1");
    saveVariableToMemory(result);
  }

  changeAccumlatorPositionToVar(var);
  changeRegValueTo(1, val->value);
  saveCommand("ADD", 1, -1, 0, 0, 0);
  changeAccumlatorPositionToVar(result);
  saveCommand("STORE", 1, -1, 0, 0, 0);
  return result;
}

symrec * addVariables(symrec * var1, symrec * var2)
{
  symrec * result = getVariable("result1");
  if (result == 0)
  {
    result = addVariable("result1");
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
  symrec * result = getVariable("result1");
  if (result == 0)
  {
    result = addVariable("result1");
    saveVariableToMemory(result);
  }

  if (val->value == 1)
  {
    changeAccumlatorPositionToVar(var);
    saveCommand("LOAD", 1, -1, 0, 0, 0);
    saveCommand("DEC", 1, -1, 0, 0, 0);
    changeAccumlatorPositionToVar(result);
    saveCommand("STORE", 1, -1, 0, 0, 0);
    return result;
  }

  changeRegValueTo(1, val->value);
  changeAccumlatorPositionToVar(result);
  saveCommand("STORE", 1, -1, 0, 0, 0);
  changeAccumlatorPositionToVar(var);
  saveCommand("LOAD", 1, -1, 0, 0, 0);
  changeAccumlatorPositionToVar(result);
  saveCommand("SUB", 1, -1, 0, 0, 0);
  saveCommand("STORE", 1, -1, 0, 0, 0);
  return result;
}

symrec * subtractNumberAndVariable(symrec * val, symrec * var)
{
  symrec * result = getVariable("result1");
  if (result == 0)
  {
    result = addVariable("result1");
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
  symrec * result = getVariable("result1");
  if (result == 0)
  {
    result = addVariable("result1");
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

  /*if (a->isTable && b->isVariable)
  {
    changeRegValueTo(4, a->memoryPosition);
    symrec * temp = getVariable(a->currentElement);
    changeAccumlatorPositionToVar(temp);
    saveCommand("ADD", 4, -1, 0, 0, 0);
    saveCommand("COPY", 4, -1, 0, 0, 0);
    saveCommand("LOAD", 1, -1, 0, 0, 0);
    symrec * temp1 = getVariable("temp11");
    if (temp1 == 0)
    {
      temp1 = addVariable("temp11");
      saveVariableToMemory(temp1);
    }
    changeAccumlatorPositionToVar(temp1);
    saveCommand("STORE", 1, -1, 0, 0, 0);
    return multVariables(temp1, b);
  }

  if (a->isVariable && b->isTable)
  {
    changeRegValueTo(4, b->memoryPosition);
    symrec * temp = getVariable(b->currentElement);
    changeAccumlatorPositionToVar(temp);
    saveCommand("ADD", 4, -1, 0, 0, 0);
    saveCommand("COPY", 4, -1, 0, 0, 0);
    saveCommand("LOAD", 1, -1, 0, 0, 0);
    symrec * temp1 = getVariable("temp11");
    if (temp1 == 0)
    {
      temp1 = addVariable("temp11");
      saveVariableToMemory(temp1);
    }
    changeAccumlatorPositionToVar(temp1);
    saveCommand("STORE", 1, -1, 0, 0, 0);
    return multVariables(a, temp1);
  }*/

  return 0;
}

symrec * multNumbers(symrec * val1, symrec * val2)
{
  val1->value *= val2->value;
  return val1;
}

symrec * multVariableAndNumber(symrec * var, symrec * val)
{
  symrec * result = getVariable("result1");
  if (result == 0)
  {
    result = addVariable("result1");
    saveVariableToMemory(result);
  }

  if (val->value == 2)
  {
    changeAccumlatorPositionToVar(var);
    saveCommand("LOAD", 2, -1, 0, 0, 0);
    saveCommand("SHL", 2, -1, 0, 0, 0);
    changeAccumlatorPositionToVar(result);
    saveCommand("STORE", 2, -1, 0, 0, 0);
    return result;
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

  symrec * result = getVariable("result1");
  if (result == 0)
  {
    result = addVariable("result1");
    saveVariableToMemory(result);
  }

  symrec * temp1 = getVariable("temp1");
  if (temp1 == 0)
  {
    temp1 = addVariable("temp1");
    saveVariableToMemory(temp1);
  }

  changeAccumlatorPositionToVar(result);
  saveCommand("LOAD", 3, -1, 0, 0, 0);
  saveCommand("ZERO", 3, -1, 0, 0, 0);

  changeAccumlatorPositionToVar(var2);
  saveCommand("LOAD", 2, -1, 0, 0, 0);
  changeAccumlatorPositionToVar(temp1);
  saveCommand("STORE", 2, -1, 0, 0, 0);

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

  if (val->value == 2 && !getModulo)
  {
    changeAccumlatorPositionToVar(var);
    saveCommand("LOAD", 2, -1, 0, 0, 0);
    saveCommand("SHR", 2, -1, 0, 0, 0);
    changeAccumlatorPositionToVar(temp);
    saveCommand("STORE", 2, -1, 0, 0, 0);
    return temp;
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
  symrec * result = getVariable("result1");
  if (result == 0)
  {
    result = addVariable("result1");
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
  saveCommand("ZERO", 4, -1, 0, 0, 0);    //make sure result is 0
  saveCommand("STORE", 4, -1, 0, 0, 0);
  changeAccumlatorPositionToVar(dividend);
  saveCommand("LOAD", 1, -1, 0, 0, 0);
  saveCommand("LOAD", 4, -1, 0, 0, 0);
  changeAccumlatorPositionToVar(mult);
  saveCommand("LOAD", 3, -1, 0, 0, 0);
  saveCommand("ZERO", 3, -1, 0, 0, 0);
  saveCommand("INC", 3, -1, 0, 0, 0);
  saveCommand("STORE", 3, -1, 0, 0, 0);
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
 // printf("WRITING JUUMP INS LABEL %s\n", buf);
 // printf("WRITING AFTER JUMP INS LABEL %s\n", buf2);
  saveCommand("JUMP", -1, -1, 0, buf, buf2);

  if (registers[0].value != 0)
  {
    registers[0].value = 0;
    saveCommand("ZERO", 0, -1, 0, 0, 0);
  }
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
  char * buf = malloc(15);
  sprintf(buf, "endcond", ifCount);
  if (strcmp(temp->nextlineLabel, "") != 0)
  {
   // printf("WARNING!! OVERRIDING NEXTLINELABEL OF %s FROM %s TO %s AT LINE %d\n", temp->name, temp->nextlineLabel, buf, k);
    changeLabel(temp->nextlineLabel, buf);
  }
  temp->nextlineLabel = malloc(8);
  strcpy(temp->nextlineLabel, buf);

  registers[0].value = 0;
  saveCommand("ZERO", 0, -1, 0, 0, 0);

 // printf("WRITING AFTER %s INS LABEL %s\n", temp->name, temp->nextlineLabel);
}

void performCondCheck(symrec * a, symrec * b, int condIndex)
{
  //1 - greater
  //2 - different
  //3 - equal
  //4 - greateOrEqual
  if (a->isValue && b->isValue)
  {
    symrec * cond1 = getVariable("cond1");
    symrec * cond2 = getVariable("cond2");
    if (cond1 == 0)
    {
      cond1 = addVariable("cond1");
      saveVariableToMemory(cond1);
    }
    if (cond2 == 0)
    {
      cond2 = addVariable("cond2");
      saveVariableToMemory(cond2);
    }
    changeRegValueTo(1, a->value);
    changeAccumlatorPositionToVar(cond1);
    saveCommand("STORE", 1, -1, 0, 0, 0);
    changeRegValueTo(1, b->value);
    changeAccumlatorPositionToVar(cond2);
    saveCommand("STORE", 1, -1, 0, 0, 0);
    if (condIndex == 1)
      greater(cond1, cond2);
    else if (condIndex == 2)
      different(cond1, cond2);
    else if (condIndex == 3)
      equal(cond1, cond2);
    else greaterOrEqual(cond1, cond2);
  }
  else if (a->isVariable && b->isVariable)
  {
    if (condIndex == 1)
      greater(a, b);
    else if (condIndex == 2)
      different(a, b);
    else if (condIndex == 3)
      equal(a, b);
    else greaterOrEqual(a, b);
  }
  else if (a->isValue && b->isVariable)
  {
    symrec * cond1 = getVariable("cond1");
    if (cond1 == 0)
    {
      cond1 = addVariable("cond1");
      saveVariableToMemory(cond1);
    }
    changeRegValueTo(1, a->value);
    changeAccumlatorPositionToVar(cond1);
    saveCommand("STORE", 1, -1, 0, 0, 0);
    if (condIndex == 1)
      greater(cond1, b);
    else if (condIndex == 2)
      different(cond1, b);
    else if (condIndex == 3)
      equal(cond1, b);
    else greaterOrEqual(cond1, b);
  }
  else if (a->isVariable && b->isValue)
  {
    symrec * cond1 = getVariable("cond1");
    if (cond1 == 0)
    {
      cond1 = addVariable("cond1");
      saveVariableToMemory(cond1);
    }
    changeRegValueTo(1, b->value);
    changeAccumlatorPositionToVar(cond1);
    saveCommand("STORE", 1, -1, 0, 0, 0);
    if (condIndex == 1)
      greater(a, cond1);
    else if (condIndex == 2)
      different(a, cond1);
    else if (condIndex == 3)
      equal(a, cond1);
    else greaterOrEqual(a, cond1);
  }
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
    sprintf(buf, "whileoperations%d", whileCount);
    sprintf(buf2, "endwhile%d", whileCount);
  }

  if (registers[0].value != 0)
  {
    registers[0].value = 0;
    saveCommand("ZERO", 0, -1, 0, 0, 0);
  }

  saveCommand("JZERO", 1, -1, 0, buf, 0);

  registers[0].value = 0;
  saveCommand("ZERO", 0, -1, 0, 0, 0);

  saveCommand("JUMP", -1, -1, 0, buf2, buf);

  registers[0].value = 0;
  saveCommand("ZERO", 0, -1, 0, 0, 0);
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
    sprintf(buf, "endwhile%d", whileCount);
  }

  registers[0].value = 0;
  saveCommand("ZERO", 0, -1, 0, 0, 0);

  saveCommand("JZERO", 1, -1, 0, buf, 0);
}

void different(symrec * a, symrec * b)
{
  //a - b != 0 || b - a != 0
 // registers[0].value = 0;
 // saveCommand("ZERO", 0, -1, 0, 0, 0);
  
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
    sprintf(buf, "orwhilecond%d", whileCount);
    sprintf(buf2, "whileoperations%d", whileCount);
  }
  saveCommand("JZERO", 1, -1, 0, buf, 0);

  registers[0].value = 0;
  saveCommand("ZERO", 0, -1, 0, 0, 0);

  saveCommand("JUMP", -1, -1, 0, buf2, 0);

  registers[0].value = 0;
  saveCommand("ZERO", 0, -1, buf, 0, 0);

  changeAccumlatorPositionToVar(b);
  saveCommand("LOAD", 1, -1, 0, 0, 0);
  changeAccumlatorPositionToVar(a);
  saveCommand("SUB", 1, -1, 0, 0, 0);
  if (isIf)
  {
    sprintf(buf, "elseif%d", ifCount);
  }
  else 
  {
    sprintf(buf, "endwhile%d", whileCount);
  }

  registers[0].value = 0;
  saveCommand("ZERO", 0, -1, 0, 0, 0);

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
    sprintf(buf, "andwhilecond%d", whileCount);
    sprintf(buf2, "endwhile%d", whileCount);
  }
  saveCommand("JZERO", 1, -1, 0, buf, 0);

  registers[0].value = 0;
  saveCommand("ZERO", 0, -1, 0, 0, 0);

  saveCommand("JUMP", -1, -1, 0, buf2, 0);

  registers[0].value = 0;
  saveCommand("ZERO", 0, -1, buf, 0, 0);

  changeAccumlatorPositionToVar(b);
  saveCommand("LOAD", 1, -1, buf, 0, 0);
  changeAccumlatorPositionToVar(a);
  if (isIf)
  {
    sprintf(buf, "operations%d", ifCount);
  }
  else 
  {
    sprintf(buf, "whileoperations%d", whileCount);
  }
  saveCommand("SUB", 1, -1, 0, 0, 0);

  registers[0].value = 0;
  saveCommand("ZERO", 0, -1, 0, 0, 0);

  saveCommand("JZERO", 1, -1, 0, buf, 0);
  saveCommand("JUMP", -1, -1, 0, buf2, buf);
}

void onWhile()
{
  whileCount++; 
  isWhile = true;
  isIf = false;

  char * buf = malloc(15);
  sprintf(buf, "startwhile%d", whileCount);

  command * temp = commands;
  while (temp->next != (command*)0)
  {
      temp = temp->next;
  }

  if (strcmp(temp->nextlineLabel, "") != 0)
  {
   // printf("WARNING!! OVERRIDING NEXTLINELABEL OF %s FROM %s TO %s AT LINE %d\n", temp->name, temp->nextlineLabel, buf, temp->k);
    changeLabel(temp->nextlineLabel, buf);
  }
  temp->nextlineLabel = malloc(15);
  strcpy(temp->nextlineLabel, buf);

  registers[0].value = 0;
  saveCommand("ZERO", 0, -1, buf, 0, 0);



//  printf("WRITING AFTER %s INS LABEL %s IN WHILE\n", temp->name, temp->nextlineLabel);
}

void afterWhile()
{
  char * buf = malloc(15);
  sprintf(buf, "startwhile%d", whileCount);
  char * buf2 = malloc(15);
  sprintf(buf2, "endwhile%d", whileCount);
  //printf("WRITING AFTER JUMP INS LABEL %s IN WHILE\n", buf2);

  //remove all errors with while
  if (registers[0].value != 0)
  {
    registers[0].value = 0;
    saveCommand("ZERO", 0, -1, 0, 0, 0);
  }

  saveCommand("JUMP", -1, -1, 0, buf, buf2);

  whileCount--;
}

void forCond(char * identifier, symrec * from, symrec * to, bool downTo)
{
  char * buf = malloc(15);
  char * buf2 = malloc(15);

  symrec * loopIterator = getVariable(identifier);
  if (loopIterator == 0)
  {
    loopIterator = addVariable(identifier);
    saveVariableToMemory(loopIterator);
  }

  sprintf(buf, "forto%d", forCount);
  symrec * toVar = getVariable(buf);
  if (toVar == 0)
  {
    toVar = addVariable(buf);
    saveVariableToMemory(toVar);
  }

  if (to->isValue)
  {
    changeRegValueTo(4, to->value);
    changeAccumlatorPositionToVar(toVar);
    toVar->knownValue = true;
    toVar->value = to->value;
    saveCommand("STORE", 4, -1, 0, 0, 0);
  }
  else
  {
    changeAccumlatorPositionToVar(to);
    if (to->knownValue)
    {
      toVar->knownValue = true;
      toVar->value = to->value;
    }
    saveCommand("LOAD", 4, -1, 0, 0, 0);
    changeAccumlatorPositionToVar(toVar);
    saveCommand("STORE", 4, -1, 0, 0, 0);
  }

  changeLabels(true);

  if (from->isValue)
  {
    changeRegValueTo(3, from->value);
    changeAccumlatorPositionToVar(loopIterator);
    loopIterator->knownValue = true;
    loopIterator->value = from->value;
    saveCommand("STORE", 3, -1, 0, 0, 0);
  }
  else 
  {
    changeAccumlatorPositionToVar(from);
    if (from->knownValue)
    {
      loopIterator->knownValue = true;
      loopIterator->value = from->value;
    }
    saveCommand("LOAD", 3, -1, 0, 0, 0);
    changeAccumlatorPositionToVar(loopIterator);
    saveCommand("STORE", 3, -1, 0, 0, 0);
  }

  if (downTo)
  {
    saveCommand("LOAD", 4, -1, 0, 0, 0);
    changeAccumlatorPositionToVar(toVar);
    saveCommand("SUB", 4, -1, 0, 0, 0);
    saveCommand("INC", 4, -1, 0, 0, 0);
    saveCommand("STORE", 4, -1, 0, 0, 0);
  }
  else
  {
    saveCommand("SUB", 4, -1, 0, 0, 0);
    saveCommand("INC", 4, -1, 0, 0, 0);
    changeAccumlatorPositionToVar(toVar);
    saveCommand("STORE", 4, -1, 0, 0, 0);
  }
  changeAccumlatorPositionToVar(loopIterator);
  sprintf(buf, "forcond%d", forCount);
  sprintf(buf2, "endfor%d", forCount);
  saveCommand("JZERO", 4, -1, buf, buf2, 0);
}

/*void forCond(char * identifier, symrec * from, symrec * to, bool downTo)
{
  char * buf = malloc(15);
  char * buf2 = malloc(15);
  symrec * loopIterator = getVariable(identifier);
  if (loopIterator == 0)
  {
    loopIterator = addVariable(identifier);
    saveVariableToMemory(loopIterator);
  }

  sprintf(buf, "forto%d", forCount);
  symrec * toVar = getVariable(buf);
  if (toVar == 0)
  {
    toVar = addVariable(buf);
   // saveVariableToMemory(toVar);
  }

  if (to->isValue)
  {
    changeRegValueTo(3, to->value);
    changeAccumlatorPositionToVar(toVar);
    toVar->knownValue = true;
    toVar->value = to->value;
    saveCommand("STORE", 3, -1, 0, 0, 0);
  }
  else
  {
    changeAccumlatorPositionToVar(to);
    if (to->knownValue)
    {
      toVar->knownValue = true;
      toVar->value = to->value;
    }
    saveCommand("LOAD", 3, -1, 0, 0, 0);
    changeAccumlatorPositionToVar(toVar);
    saveCommand("STORE", 3, -1, 0, 0, 0);
  }

  changeLabels(true);

  sprintf(buf, "forcond%d", forCount);

  if (from->isValue)
  {
    changeRegValueTo(3, from->value);
    changeAccumlatorPositionToVar(loopIterator);
    loopIterator->knownValue = true;
    loopIterator->value = from->value;
    saveCommand("STORE", 3, -1, 0, 0, buf);
  }
  else 
  {
    changeAccumlatorPositionToVar(from);
    if (from->knownValue)
    {
      loopIterator->knownValue = true;
      loopIterator->value = from->value;
    }
    saveCommand("LOAD", 3, -1, 0, 0, 0);
    changeAccumlatorPositionToVar(loopIterator);
    saveCommand("STORE", 3, -1, 0, 0, buf);
  }

  if (downTo)
  {
    changeAccumlatorPositionToVar(toVar);
    saveCommand("LOAD", 3, -1, 0, 0, 0);
    changeAccumlatorPositionToVar(loopIterator);
    saveCommand("SUB", 3, -1, 0, 0, 0);
  } 
  else
  {
    changeAccumlatorPositionToVar(loopIterator);
    saveCommand("LOAD", 3, -1, 0, 0, 0);
    changeAccumlatorPositionToVar(toVar);
    saveCommand("SUB", 3, -1, 0, 0, 0);
  }

  sprintf(buf, "foroperations%d", forCount);
  sprintf(buf2, "endfor%d", forCount);

  saveCommand("JZERO", 3, -1, 0, buf, 0);
  saveCommand("JUMP", -1, -1, 0, buf2, buf);
}*/

void afterFor(char * iter, bool downTo)
{
  char * buf = malloc(15);
  char * buf2 = malloc(15);

  symrec * var = getVariable(iter);
  changeAccumlatorPositionToVar(var);
  saveCommand("LOAD", 3, -1, 0, 0, 0);
  if (downTo)
  {
    if (var->knownValue)
      --var->value;
    saveCommand("DEC", 3, -1, 0, 0, 0);
  }
  else
  {
    if (var->knownValue)
      ++var->value;
    saveCommand("INC", 3, -1, 0, 0, 0);
  }

  saveCommand("STORE", 3, -1, 0, 0, 0);

  sprintf(buf, "forto%d", forCount);
  var = getVariable(buf);
  changeAccumlatorPositionToVar(var);
  saveCommand("LOAD", 4, -1, 0, 0, 0);

  if (var->knownValue)
    --var->value;
  saveCommand("DEC", 4, -1, 0, 0, 0);

  saveCommand("STORE", 4, -1, 0, 0, 0);

  changeAccumlatorPositionToVar(getVariable(iter));

  sprintf(buf, "forcond%d", forCount);
  sprintf(buf2, "endfor%d", forCount);
  saveCommand("JUMP", -1, -1, 0, buf, buf2);
  removeVariable(iter);
  removeVariable(buf);
}