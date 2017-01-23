#include "compiler.h"

FILE * output = NULL;
int k = 0;
int currentMemoryLength = 0;
int currentCommandsLength = 0;

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
  printf("SAVED VAR %s AT %lld\n", var->name, registers[0].value);
  registers[0].value = reg0Value;
}

void saveTableToMemory(symrec * tab)
{
	saveVariableToMemory(tab);
    int size = tab->tabLength;
    int memoryPosition = tab->memoryPosition;
    int i = 0;
    for (i = 0; i < size; ++i)
    {
      if (currentMemoryLength <= memoryPosition + i)
      {
        mem * moreMemory = realloc(memory, 2 * currentMemoryLength * sizeof(mem));
        memory = moreMemory;
        int i;
        for (i = currentMemoryLength; i < 2 * currentMemoryLength; ++i)
          memory[i].isUsed = false;
        currentMemoryLength = 2 * currentMemoryLength;
      }
      tab->elements[i].name = malloc(1);
      tab->elements[i].initialized = true;
      tab->elements[i].declared = true;
      tab->elements[i].isTable = false;
      tab->elements[i].isVariable = true;
      tab->elements[i].isValue = false;
      tab->elements[i].tabLength = -1;
      tab->elements[i].value = 0;
      tab->elements[i].regNumber = -1;
      tab->elements[i].knownValue = false;
      tab->elements[i].memoryPosition = memoryPosition + i;
      memory[memoryPosition + i].isUsed = true;
    }
}

void saveVariableToAssemblyMemory(symrec * var)
{
	if (!registers[0].isInitialized)
	{
		registers[0].isInitialized = true;
		registers[0].value = 0;
		saveCommand("ZERO", 0, -1, 0, 0, 0);
	}
	
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
  printf("SAVED VAR %s AT %lld\n", var->name, registers[0].value);
}

void changeAccumlatorPositionToVar(symrec * var)
{
  //changeRegValueTo(0, var->memoryPosition);
  //saveCommand("ZERO", 0, -1, 0, 0, 0);
  //registers[0].value = 0;

	if (var->memoryPosition == -1)
	{
		symrec * tab = getVariable(var->name);
		changeRegValueTo(4, tab->memoryPosition);
  	symrec * temp = getVariable(var->currentElement);
  	changeAccumlatorPositionToVar(temp);
  	saveCommand("ADD", 4, -1, 0, 0, 0);
  	saveCommand("COPY", 4, -1, 0, 0, 0);
  	registers[0].isInitialized = false;
  	return;
	}

	if (!registers[0].isInitialized)
	{
		registers[0].isInitialized = true;
		registers[0].value = 0;
		saveCommand("ZERO", 0, -1, 0, 0, 0);
	}
 	printf("CHANGING ACCUMLATOR FROM %llu TO %d\n", registers[0].value, var->memoryPosition);

	ull from = registers[0].value;
	ull to = var->memoryPosition;

	if (to == from)
		return;

	if (to == 0)
	{
		saveCommand("ZERO", 0, -1, 0, 0, 0);
		registers[0].value = 0;
		return;
	}

  if (to > from)
  {
    int currentSize = 20;
    ull * tab = (ull*)malloc(sizeof(ull) * currentSize);
    int i = 0;
    while (to > from)
    {
      if (i == currentSize - 1)
      {
          ull * moreSpace = realloc(tab, 2 * currentSize * sizeof(ull));
          tab = moreSpace;
          currentSize = 2 * currentSize;
      }
      tab[i++] = to;
      if (to % 2 == 0 && to / 2 >= from)
        to /= 2;
      else 
        --to;
    }
    tab[i] = to;

    while (i > 0)
    {
      if (tab[i] * 2 == tab[i - 1])
      {
       // sprintf(buf, "SHL %d", reg);
       // writeToFile(buf);
        saveCommand("SHL", 0, -1, 0, 0, 0);
        registers[0].value *= 2;
      }
      else if (tab[i] + 1 == tab[i - 1])
      {
       // sprintf(buf, "INC %d", reg);
       // writeToFile(buf);
        saveCommand("INC", 0, -1, 0, 0, 0);
        registers[0].value++;
      }
      --i;
    }

    if (registers[0].value != to)
    {
      if (registers[0].value * 2 == to)
      {
       // sprintf(buf, "SHL %d", reg);
       // writeToFile(buf);
        saveCommand("SHL", 0, -1, 0, 0, 0);
        registers[0].value *= 2;
      }
      else if (registers[0].value + 1 == to)
      {
       // sprintf(buf, "INC %d", reg);
       // writeToFile(buf);
        saveCommand("INC", 0, -1, 0, 0, 0);
        registers[0].value++;
      }
    }
  }
  else
  {
    int currentSize = 20;
    ull * tab = (ull*)malloc(sizeof(ull) * currentSize);
    int i = 0;
    while (from > to)
    {
      if (i == currentSize - 1)
      {
          ull * moreSpace = realloc(tab, 2 * currentSize * sizeof(ull));
          tab = moreSpace;
          currentSize = 2 * currentSize;
      }
      tab[i++] = from;
      if (from % 2 == 0 && from / 2 >= to)
        from /= 2;
      else 
        --from;
    }

    int j = 0;
    while (j < i)
    {
      if (tab[j] / 2 == tab[j + 1])
      {
       // sprintf(buf, "SHL %d", reg);
       // writeToFile(buf);
        saveCommand("SHR", 0, -1, 0, 0, 0);
        registers[0].value /= 2;
      }
      else if (tab[j] - 1 == tab[j + 1])
      {
       // sprintf(buf, "INC %d", reg);
       // writeToFile(buf);
        saveCommand("DEC", 0, -1, 0, 0, 0);
        registers[0].value--;
      }
      ++j;
    }

    if (registers[0].value != to)
    {
      if (registers[0].value / 2 == to)
      {
       // sprintf(buf, "SHL %d", reg);
       // writeToFile(buf);
        saveCommand("SHR", 0, -1, 0, 0, 0);
        registers[0].value /= 2;
      }
      else if (registers[0].value - 1 == to)
      {
       // sprintf(buf, "INC %d", reg);
       // writeToFile(buf);
        saveCommand("DEC", 0, -1, 0, 0, 0);
        registers[0].value--;
      }
    }
  }
 
}

int getRegWithValue(ull value)
{
  int i;
  for (i = 0; i < 5; ++i)
    if (registers[i].value == value)
      return i;
  return -1;
}

void changeRegValueTo(int reg, ull value)
{
  //char buf[7];
  //sprintf(buf, "ZERO %d", reg);
  //writeToFile(buf);
  saveCommand("ZERO", reg, -1, 0, 0, 0);
  registers[reg].value = 0;
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
      registers[reg].value *= 2;
    }
    else 
    {
     // sprintf(buf, "INC %d", reg);
     // writeToFile(buf);
      saveCommand("INC", reg, -1, 0, 0, 0);
      registers[reg].value++;
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
 // printf("%d %s %d\n", k, name, arg1);
  if (commands->name != 0)
  {
    command * temp = commands;
    while (temp->next != (command*)0)
      temp = temp->next;
    if (strcmp(temp->nextlineLabel, "") != 0)
    {
   //   if (strcmp(com->label, "") != 0)
     //   printf("WARNING!! OVERRIDING LABEL AT %d COMMAND %s\n", k, com->name);
      com->label = malloc(sizeof(temp->nextlineLabel) + 1);
      strcpy(com->label, temp->nextlineLabel);
      //printf("Set label %s to ins %s at line %d\n", com->label, com->name, k);
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

void changeLabels(bool removeLabel)
{
  command * current = commands;
  while (current != (command*)0)
  {
    if (strcmp(current->label, "") != 0) 
    {
      int k = current->k;
      command * temp = commands;
      bool found = false;
      while (temp != (command*)0)
      {
        if (strcmp(temp->tolabel, current->label) == 0)
        {
          found = true;
          printf("Changing label %s at command %s to %d\n", temp->tolabel, temp->name, k);
          temp->tolabel = "";
          if (temp->arg1 == -1)
            temp->arg1 = k;
          else temp->arg2 = k;
        }
        temp = temp->next;
      }
      if (found && removeLabel)
      {
        printf("REMOVING LABEL %s\n", current->label);
        strcpy(current->label, "\0");
      }
    }
    current = current->next;
  }
}

void changeLabel(char * tolabel, char * label)
{
  command * current = commands;
  while (current != (command*)0)
  {
    if (strcmp(current->tolabel, tolabel) == 0)
    {
      current->tolabel = malloc(sizeof(label) + 1);
      strcpy(current->tolabel, label);
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