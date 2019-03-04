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
      if (strcmp(command,"exit\n") == 0) {
         exit(0);
      }
      if (strcmp(command,"cd") == 0) {
         cd_cmd();
      }
      if (strcmp(command,"status") == 0) {
         status_cmd();
      }
   }
}

void cd_cmd() {
   if (numargs == 0){
      chdir(getenv("HOME"));
      sprintf(curdir,"%s", getenv("HOME"));
   }
   else {
      if (&args[0][0] == "." && &args[0][1] == "/"){
         //char *arg = args[0];
         //arg += 2;
         sprintf(args[0],"%s%s",curdir,args[0][2]);
      }
      else{
         printf("%s%s:%s\n", args[0][0],args[0][1],&args[0][0]);
      }
      printf("%d",chdir(args[0]));
      sprintf(curdir,"%s", args[0]);
   }
   printf("%d%s\n", 90,curdir);
   fflush(stdout);
}

void status_cmd() {

}

void parseCmd() {
   int nextwordType = COMMAND;
   numargs = 0;
   bool backgroundProcess = false;
   char *word = strtok(linein, " \n");
   while (word != NULL) {
      printf("\"%s\"\n",word);
      fflush(stdout);

      //look for all occurences of '$$' and replace them with the PID
      while (strstr(word,"$$") != NULL){
         char pid[10];
         printf("%d", getpid());
         sprintf(pid,"%d",getpid());
         char newstring[strlen(word) + strlen(pid)];
         strcpy(newstring,word);
         char *s = strstr(newstring,"$$");
         char *secondhalf = strstr(word,"$$")+2;
         strncpy(s,pid,strlen(pid));
         strcpy(s+strlen(pid),secondhalf);
         strcpy(word,newstring);
      }
      if (strcmp(word,"<") == 0) {
         word = strtok(NULL," \n");
         if (word != NULL) {
            sprintf(stdinLoc,"%s",word);
            nextwordType = ARG;
         }
      }
      else if (strcmp(word,">") == 0) {
         word = strtok(NULL," \n");
         if (word != NULL) {
            sprintf(stdoutLoc,"%s",word);
            nextwordType = ARG;
         }
      }
      else if (strcmp(word,"&") == 0) {
         word = strtok(NULL," \n");
         if (word == NULL) {
            backgroundProcess = true;
         }
      }
      else if (nextwordType == COMMAND) {
         sprintf(command, "%s", word);
         nextwordType = ARG;
      }
      else if (nextwordType == ARG) {
         printf("ARG: %s\n", word);
         args[numargs] = word;
         numargs++;
      }
      if (word != NULL){
         word = strtok(NULL," \n");
      }
   }
}

//this method takes in a command string and executes the given command with its arguments
int runChild() {

}
