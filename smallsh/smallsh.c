//##################\\
//##ZACHARY HORINE##\\
//##CS344---BLOCK2##\\
//##WINTER02132019##\\
//##################\\
//smallsh.c

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <time.h>
#include <string.h>
#include <dirent.h>
#include <pthread.h>

typedef int bool;
#define true 1
#define false 0

int COMMAND = 0;
int ARG = 1;
int REDIR_STDIN = 2;
int REDIR_STDOUT = 3;
int COMMENT = 4;

char linein[2048];
char command[1024];
char* args[512];
int numargs = 0;
char stdinLoc[512];
char stdoutLoc[512];
char curdir[512];

bool running = true;
bool allowBackground = true;


void parseCmd();
void status_cmd();
void cd_cmd();


int main() {

   getcwd(curdir, sizeof(curdir));

   while(running){
      command = "";
      numargs = 0;
      printf(": ");
      fflush(stdout);
      fgets(linein,2048,stdin);   //get user input
      printf("PARSE\n");
      fflush(stdout);
      parseCmd();
   }
}

}

void status_cmd() {

}

void parseCmd() {
   int wordStart = 0;
   char word[1024];
   int nextwordType = COMMAND;
   numargs = 0;
   for (int i = 0; i < linein.length; i++){
      if (linein[i] == ' '){
         snprintf(word, i-wordStart, "%s", linein[wordStart]);
         if (word == "#") {
            //this is a comment, exit parsing and return blank string
            command = "";
            return;
         }
         else if (word == "<") {
            nextwordType = REDIR_STDIN;
         }
         else if (word == ">") {
            nextwordType = REDIR_STDOUT;
         }
         else if (word == "&") {

         }
         else if (nextwordType == COMMAND) {
            sprintf(command, "%s", word);
            nextwordType = ARG;
         }
         else if (nextwordType == ARG) {
            sprintf(args[numargs],"%s",word);
            numargs++;
         }
         else if (nextwordType == REDIR_STDIN) {
            sprintf(stdin,"%s",word);
            nextwordType = ARG;
         }
         else if (nextwordType == REDIR_STDOUT) {
            sprintf(stdout,"%s",word);
            nextwordType = ARG;
         }
         wordStart = i+1;
      }
   }
}

//this method takes in a command string and executes the given command with its arguments
int runChild() {

}
