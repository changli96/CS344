//##################\\
//##ZACHARY HORINE##\\
//##CS344---BLOCK4##\\
//##WINTER03172019##\\
//##################\\
//keygen.c

#include <unistd.h>
#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char *argv[]) {
   if (argc < 2) {printf("No keylength given!\n"); exit(1);}
   if (argc > 2) {printf("Too many arguments!\n"); exit(1);}
   int keylength = atoi(argv[1]);
   if (keylength == 0) {printf("Please input a number.\n"); exit(1);}
   char chars[28] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ ";
   srand(time(NULL));   // random initialization
   int i;
   char *key = malloc(keylength+1*sizeof(char));
   for (i = 0; i < keylength; i++){
      strncat(key,&chars[rand() % 27],1);
   }
   printf("%s\n",key);
}
