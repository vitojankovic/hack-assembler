#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <ctype.h>

//! STEPS: First manage to output the entire assembly file context line by line into cmd and pass each into a function
//! called assemble


//? Table for symbols
typedef struct {
  char name[64];
  int address;
} SymbolEntry;

typedef struct {
  SymbolEntry entries[1000];
  int count;
} SymbolTable;

void st_init(SymbolTable *t);
void st_add(SymbolTable *t, const char *name, int addr);
int st_contains(SymbolTable *t, const char *name);
int st_get_address(SymbolTable *t, const char *name);


// Translates dest token ("D", "M", "AMD", etc.) to 3 bits
void parse_dest(char *dest, char *result);

// Translates jump token ("JGT", "JMP", etc.) to 3 bits
void parse_jump(char *jump, char *result);

// Translates comp token ("D+1", "M-D", "0", etc.) to 7 bits (a c1 c2 c3 c4 c5 c6)
void parse_comp(char *comp, char *result);


void dismantle(FILE *pSource, FILE *pOutput, SymbolTable *t);
char* assemble(char line[], SymbolTable *t, int *nextVarAdress);

void strip_comment_newline(char *line);
void trim(char *line);
int is_blank(char *line);

//? Record all labels
void first_pass(FILE *pSource, SymbolTable *t);



int main()
{
  //? 1st step: Prompt user for absolute file path
  char fileName[256];

  printf("Enter your file path: ");

  scanf("%255s", fileName);

  FILE *pSource;

  pSource = fopen(fileName , "r");

  if(pSource == NULL){
    printf("The file is not found!\n");
    return 1;
  }

  printf("File opened successfully!\n");

  //? 2nd step: output the binary into name.txt
  FILE *pOutput = fopen("output.txt", "w");
  if(pOutput == NULL){
    printf("File cannot be created");
    fclose(pSource);
    return 1;
  }

  SymbolTable table;
  st_init(&table);
  first_pass(pSource, &table);
  rewind(pSource);

  dismantle(pSource, pOutput, &table);


  fclose(pSource);
  fclose(pOutput);

  return 0;
}

void dismantle(FILE *pSource, FILE *pOutput, SymbolTable *t)
{
  //? Logic for taking an input file row as a string and performing conversion and outputing
  char line[256];
  int nextVarAddress = 16;

  while(fgets(line, sizeof(line), pSource) != NULL){
    char *binaryCode = assemble(line, t, &nextVarAddress);
    if(binaryCode != NULL){
      fprintf(pOutput, "%s\n", binaryCode);
    }
  }
}

char* assemble(char line[], SymbolTable *t, int *nextVarAddress){
  //? 1st step: look at the first character, if @ its an A instruction, if not its a C instruction
  //? Handling A instructions: set first bit to 0 and convert from decimal to binary the rest 15 characters
  //? Handling C instructions: 

  static char result[17];

  strip_comment_newline(line);
  trim(line);

  if(is_blank(line)){
    return NULL;
  }

  if(line[0] == '('){
    return NULL;
  }

  for (int i = 0; line[i] != '\0'; i++) {
    if (line[i] == '/' || line[i] == '\n' || line[i] == '\r') {
      line[i] = '\0'; // Truncate string here!
      break;
    }
  }

  if(line[0] == '\0'){
    return NULL;
  }

  if(line[0] == '@'){
    //! A instruction
    result[0] = '0';


    //? Conversion from decimal to binary
    //? Loop result from left to right and check if pow(15-i, 2) + counter < line/first 0 then replace 0 with 1 and 
    //? counter+=pow(15-i, 2)


    //* Convert from string to int using atoi(&line[i])

    int addressNumber = atoi(&line[1]);


    //? Checks if @x is a number or a letter
    char *symbol = &line[1];
    if (isdigit((unsigned char)symbol[0])) {
      addressNumber = atoi(symbol);
    } else {
      if (!st_contains(t, symbol)) {
        //? first time we've ever seen this variable name -> give it a fresh RAM slot
        st_add(t, symbol, *nextVarAddress);
        (*nextVarAddress)++;
      }
      addressNumber = st_get_address(t, symbol);
    }

    int i = 1;
    int counter = 0;

    while(i <= 15){
      if( (pow(2,(15-i))+counter) <=  addressNumber){
          counter+=pow(2,15-i);
          result[i] = '1';
      }
      else{
        result[i] = '0';
      }
      i++;
    }
    result[16] = '\0';
    return result;
  }
  else {
    //! C instruction
    result[0] = '1';
    result[1] = '1';
    result[2] = '1';

    char dest_str[10] = "";
    char comp_str[10] = "";
    char jump_str[10] = "";

    char *equals_pos = strchr(line, '=');
    char *semicolon_pos = strchr(line, ';');

    // Extract dest and comp
    if (equals_pos != NULL) {
        int dest_len = equals_pos - line;
        strncpy(dest_str, line, dest_len);
        dest_str[dest_len] = '\0';

        if (semicolon_pos != NULL) {
            int comp_len = semicolon_pos - (equals_pos + 1);
            strncpy(comp_str, equals_pos + 1, comp_len);
            comp_str[comp_len] = '\0';
        } else {
            strcpy(comp_str, equals_pos + 1);
        }
    } else {
        dest_str[0] = '\0';

        if (semicolon_pos != NULL) {
            int comp_len = semicolon_pos - line;
            strncpy(comp_str, line, comp_len);
            comp_str[comp_len] = '\0';
        } else {
            strcpy(comp_str, line);
        }
    }

    // Extract jump
    if (semicolon_pos != NULL) {
        strcpy(jump_str, semicolon_pos + 1);
    } else {
        jump_str[0] = '\0';
    }

    // Fill bit pattern using lookup functions
    parse_comp(comp_str, result);
    parse_dest(dest_str, result);
    parse_jump(jump_str, result);

    result[16] = '\0';
    return result;
    }
}




void parse_comp(char *comp, char *result) {
    // Set the 'a' bit (bit 3) depending on whether 'M' is referenced
    if (strchr(comp, 'M') != NULL) {
        result[3] = '1';
    } else {
        result[3] = '0';
    }

    // Match the c1-c6 bits (indices 4 to 9)
    if (strcmp(comp, "0") == 0)                             { strcpy(&result[4], "101010"); }
    else if (strcmp(comp, "1") == 0)                        { strcpy(&result[4], "111111"); }
    else if (strcmp(comp, "-1") == 0)                       { strcpy(&result[4], "111010"); }
    else if (strcmp(comp, "D") == 0)                        { strcpy(&result[4], "001100"); }
    else if (strcmp(comp, "A") == 0 || strcmp(comp, "M") == 0) 
                                                            { strcpy(&result[4], "110000"); }
    else if (strcmp(comp, "!D") == 0)                       { strcpy(&result[4], "001101"); }
    else if (strcmp(comp, "!A") == 0 || strcmp(comp, "!M") == 0) 
                                                            { strcpy(&result[4], "110001"); }
    else if (strcmp(comp, "-D") == 0)                       { strcpy(&result[4], "001111"); }
    else if (strcmp(comp, "-A") == 0 || strcmp(comp, "-M") == 0) 
                                                            { strcpy(&result[4], "110011"); }
    else if (strcmp(comp, "D+1") == 0)                      { strcpy(&result[4], "011111"); }
    else if (strcmp(comp, "A+1") == 0 || strcmp(comp, "M+1") == 0) 
                                                            { strcpy(&result[4], "110111"); }
    else if (strcmp(comp, "D-1") == 0)                      { strcpy(&result[4], "001110"); }
    else if (strcmp(comp, "A-1") == 0 || strcmp(comp, "M-1") == 0) 
                                                            { strcpy(&result[4], "110010"); }
    else if (strcmp(comp, "D+A") == 0 || strcmp(comp, "D+M") == 0) 
                                                            { strcpy(&result[4], "000010"); }
    else if (strcmp(comp, "D-A") == 0 || strcmp(comp, "D-M") == 0) 
                                                            { strcpy(&result[4], "010011"); }
    else if (strcmp(comp, "A-D") == 0 || strcmp(comp, "M-D") == 0) 
                                                            { strcpy(&result[4], "000111"); }
    else if (strcmp(comp, "D&A") == 0 || strcmp(comp, "D&M") == 0) 
                                                            { strcpy(&result[4], "000000"); }
    else if (strcmp(comp, "D|A") == 0 || strcmp(comp, "D|M") == 0) 
                                                            { strcpy(&result[4], "010101"); }
}

void parse_dest(char *dest, char *result) {
    if (strcmp(dest, "M") == 0)        { result[10]='0'; result[11]='0'; result[12]='1'; }
    else if (strcmp(dest, "D") == 0)   { result[10]='0'; result[11]='1'; result[12]='0'; }
    else if (strcmp(dest, "MD") == 0 || strcmp(dest, "DM") == 0) 
                                       { result[10]='0'; result[11]='1'; result[12]='1'; }
    else if (strcmp(dest, "A") == 0)   { result[10]='1'; result[11]='0'; result[12]='0'; }
    else if (strcmp(dest, "AM") == 0)  { result[10]='1'; result[11]='0'; result[12]='1'; }
    else if (strcmp(dest, "AD") == 0)  { result[10]='1'; result[11]='1'; result[12]='0'; }
    else if (strcmp(dest, "AMD") == 0) { result[10]='1'; result[11]='1'; result[12]='1'; }
    else { result[10]='0'; result[11]='0'; result[12]='0'; }
}

void parse_jump(char *jump, char *result) {
    if (strcmp(jump, "JGT") == 0)      { result[13]='0'; result[14]='0'; result[15]='1'; }
    else if (strcmp(jump, "JEQ") == 0) { result[13]='0'; result[14]='1'; result[15]='0'; }
    else if (strcmp(jump, "JGE") == 0) { result[13]='0'; result[14]='1'; result[15]='1'; }
    else if (strcmp(jump, "JLT") == 0) { result[13]='1'; result[14]='0'; result[15]='0'; }
    else if (strcmp(jump, "JNE") == 0) { result[13]='1'; result[14]='0'; result[15]='1'; }
    else if (strcmp(jump, "JLE") == 0) { result[13]='1'; result[14]='1'; result[15]='0'; }
    else if (strcmp(jump, "JMP") == 0) { result[13]='1'; result[14]='1'; result[15]='1'; }
    else { result[13]='0'; result[14]='0'; result[15]='0'; }
}



void first_pass(FILE *pSource, SymbolTable *t){
  char line[256];
  int romAddress = 0;

  while(fgets(line, sizeof(line), pSource) != NULL){
    strip_comment_newline(line);
    trim(line);

    if(is_blank(line)){
      continue;
    }

    if(line[0] == '('){
      //? (LOOP)
      int len=strlen(line);
      char label[64];
      strncpy(label, line + 1, len-2);
      label[len - 2] = '\0';

      st_add(t, label, romAddress);
    }
    else{
      romAddress++;
    }
  }
}

void strip_comment_newline(char *line){
  for(int i = 0; line[i] != '\0'; i++){
    if(line[i] == '/' || line[i] == '\n' || line[i] == '\r'){
      line[i] = '\0';
      break;
    }
  }
}

void trim(char *line){
  int len = strlen(line);
  while(len > 0 && isspace((unsigned char)line[len - 1])){
    line[--len] = '\0';
  }
}

void st_init(SymbolTable *t)
{
  t->count = 0;

  st_add(t, "SP", 0);
  st_add(t, "LCL", 1);
  st_add(t, "ARG", 2);
  st_add(t, "THIS", 3);
  st_add(t, "THAT", 4);
  st_add(t, "SCREEN", 16384);
  st_add(t, "KBD", 24576);

  char regName[4];
  for (int i = 0; i <= 15; i++) {
    sprintf(regName, "R%d", i);
    st_add(t, regName, i);
  }
}

void st_add(SymbolTable *t, const char *name, int addr)
{
  strcpy(t->entries[t->count].name, name);
  t->entries[t->count].address = addr;
  t->count++;
}

int st_contains(SymbolTable *t, const char *name)
{
  for (int i = 0; i < t->count; i++) {
    if (strcmp(t->entries[i].name, name) == 0) {
      return 1;
    }
  }
  return 0;
}

int st_get_address(SymbolTable *t, const char *name)
{
  for (int i = 0; i < t->count; i++) {
    if (strcmp(t->entries[i].name, name) == 0) {
      return t->entries[i].address;
    }
  }
  return -1;
}

int is_blank(char *line){
  return line[0] == '\0';
}