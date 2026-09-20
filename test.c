#include <stdio.h>
#include <stdlib.h>

int main()
{

  char grade[] = "ABCD";
  char *address = &grade[0];
  
  printf("%s", address);

  return 0;
}