#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>

//! STEPS: First manage to output the entire assembly file context line by line into cmd and pass each into a function
//! called assemble


// Translates dest token ("D", "M", "AMD", etc.) to 3 bits
void parse_dest(char *dest, char *result);

// Translates jump token ("JGT", "JMP", etc.) to 3 bits
void parse_jump(char *jump, char *result);

// Translates comp token ("D+1", "M-D", "0", etc.) to 7 bits (a c1 c2 c3 c4 c5 c6)
void parse_comp(char *comp, char *result);


void dismantle(FILE *pSource, FILE *pOutput);
char* assemble(char line[]);

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

  dismantle(pSource, pOutput);


  fclose(pSource);
  fclose(pOutput);

  return 0;
}

void dismantle(FILE *pSource, FILE *pOutput)
{
  //? Logic for taking an input file row as a string and performing conversion and outputing
  char line[256];

  while(fgets(line, sizeof(line), pSource) != NULL){
    char *binaryCode = assemble(line);
    if(binaryCode != NULL){
      fprintf(pOutput, "%s\n", binaryCode);
    }
  }
}

char* assemble(char line[]){
  //? 1st step: look at the first character, if @ its an A instruction, if not its a C instruction
  //? Handling A instructions: set first bit to 0 and convert from decimal to binary the rest 15 characters
  //? Handling C instructions: 

  static char result[17];

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
                                                            { strcpy(&result[4], "110000"); } // FIXED: was "011000"
    else if (strcmp(comp, "!D") == 0)                       { strcpy(&result[4], "001101"); }
    else if (strcmp(comp, "!A") == 0 || strcmp(comp, "!M") == 0) 
                                                            { strcpy(&result[4], "110001"); } // FIXED: was "011001"
    else if (strcmp(comp, "-D") == 0)                       { strcpy(&result[4], "001111"); }
    else if (strcmp(comp, "-A") == 0 || strcmp(comp, "-M") == 0) 
                                                            { strcpy(&result[4], "110011"); } // FIXED: was "011001"
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