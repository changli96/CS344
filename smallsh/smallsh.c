//##################\\
//##ZACHARY HORINE##\\
//##CS344---BLOCK2##\\
//##WINTER03032019##\\
//##################\\
//smallsh.c

#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>

typedef int bool;
#define true 1
#define false 0

bool allowBackground = true;

void parseCmd(char **args, int *numargs, char stdinLoc[512], char stdoutLoc[512], bool *backgroundProcess,int pid) {
   char pidstr[10];
   sprintf(pidstr,"%d",getpid());
   char linein[2048];
   // Get input
   printf(": ");
   fflush(stdout);
   fflush(stdin);
   fgets(linein, 2048, stdin);
   if(linein[0] == '#'){return;} //this is a comment, exit parsing and return
   char *word = strtok(linein," \n"); //tokenize string, word is pointer to next token

   while(word != NULL){
      *backgroundProcess = false;
      while (strstr(word,"$$") != NULL){ //look for all occurences of '$$' and replace them with the PID
         char newstring[strlen(word) + strlen(pidstr)];
         strcpy(newstring,word);
         char *s = strstr(newstring,"$$");
         char *secondhalf = strstr(word,"$$")+2;
         strncpy(s,pidstr,strlen(pidstr));
         strcpy(s+strlen(pidstr),secondhalf);
         strcpy(word,newstring);
      }
      if (strcmp(word,"<") == 0) {
         word = strtok(NULL," \n");
         if (word != NULL) {
            sprintf(stdinLoc,"%s",word);
         }
      }
      else if (strcmp(word,">") == 0) {
         word = strtok(NULL," \n");
         if (word != NULL) {
            sprintf(stdoutLoc,"%s",word);
         }
      }
      else if(strcmp(word,"&") == 0){
         *backgroundProcess = true;
      }
      else{
         args[*numargs] = word;
         *numargs += 1;
      }
      word = strtok(NULL," \n");
   }
   args[*numargs] = NULL;
   return;
}

void callCmd(char* args[], int* fgExitStatus, bool* backgroundProcess, char stdinLoc[], char stdoutLoc[]) {
   if (allowBackground == false) {backgroundProcess = false;}
   int input, output, result;
   int childExitStatus = -1;
   pid_t forkedPid = -5;
   //fork creates a duplicate process. forkedPid is equal to the childPid within the parent, and 0 in the child
   forkedPid = fork(); //fork the process
   if (forkedPid == -1){ //failed to fork
      printf("FAILED FORK!\n");
      fflush(stdout);
      exit(1);
   }
   else if (forkedPid == 0){ //child fork
      //allow CTRL-c to be default as normal
      signal(SIGINT, SIG_DFL);

      //IO redirection: stdin
      if (strcmp(stdinLoc,"") != 0){ //stdin var set (open file)
         int infile = open(stdinLoc,O_RDONLY);
         if (infile == -1) { //fail to read file
            printf("Cannot open \"%s\" for input.\n", stdinLoc);
            fflush(stdout);
            exit(1);
         }
         else { //no error, set file as input
            dup2(infile,0);
            fcntl(infile, F_SETFD, FD_CLOEXEC);
         }
      }
      else if (*backgroundProcess){ //stdin var not set, default to /dev/null
         int devnull = open("/dev/null",O_WRONLY);
         dup2(devnull,0);
      }
      //IO redirection: stdout
      if (strcmp(stdoutLoc,"") != 0){ //stdout var set (open file)
         int outfile = open(stdoutLoc,O_WRONLY | O_TRUNC | O_CREAT);
         if (outfile == -1) { //fail to write file
            printf("Cannot open \"%s\" for output.\n", stdoutLoc);
            fflush(stdout);
            exit(1);
         }
         else { //no error, set file as output
            dup2(outfile,1);
            fcntl(outfile, F_SETFD, FD_CLOEXEC);
         }
      }
      else if(*backgroundProcess){ //stdin var not set, default to /dev/null
         int devnull = open("/dev/null",O_WRONLY);
         dup2(devnull,1);
      }

      // command execution
      int result = execvp(args[0], (char* const*)args);
      if (result < 0) { //issue in running the command
         printf("%s: no such file or directory\n", args[0]);
         fflush(stdout);
         result = 0;
         exit(1); //exit the child fork
      }
   }
   else{ //parent fork
      // Execute a process in the background ONLY if allowBackground
      if (*backgroundProcess) {
         pid_t actualPid = waitpid(forkedPid, &childExitStatus, WNOHANG);
         printf("background pid is %d\n", forkedPid);
         fflush(stdout);
         *backgroundProcess = 0;
      }
      // Otherwise execute it like normal
      else {
         pid_t actualPid = waitpid(forkedPid, fgExitStatus, 0);
         if(*fgExitStatus == 256){*fgExitStatus = 1;}
         else if(WIFEXITED(*fgExitStatus) <= 0){
             printf("terminated by signal %d\n",WTERMSIG(*fgExitStatus));
             fflush(stdout);
         }
      }
   }
}

void backgroundManager(int sig){
   if (allowBackground){
      allowBackground = false;
      write(1,"\nEntering foreground only mode\n",31);
   } else {
      allowBackground = true;
      write(1,"\nExiting foreground only mode\n",30);
   }
}

int main(){
   //initalize variables
   int i, j;
   bool running = true;
   char *args[512];
   int numargs = 0;
   bool backgroundProcess = false;
   char stdinLoc[512] = "";
   char stdoutLoc[512] = "";
   //set current direcotry
   char curdir[512];
   getcwd(curdir, sizeof(curdir));

   //set up signal handlers (ctrlz)
   struct sigaction ctrlz = {0};
   ctrlz.sa_handler = backgroundManager;
   ctrlz.sa_flags = 0;
   sigfillset(&ctrlz.sa_mask);
   sigaction(SIGTSTP, &ctrlz, NULL);

   //set up signal handlers (ctrlc)
   struct sigaction ctrlc = {0};
   ctrlc.sa_handler = SIG_IGN;
   ctrlc.sa_flags = 0;
   sigfillset(&ctrlc.sa_mask);
   sigaction(SIGINT, &ctrlc, NULL);
   int forkedPid;
   int result = 0;

   int fgProcessStatus = 0;// Foreground
   int bgProcessStatus = -5;// Background

   //Signal handling
   sigaction(SIGINT, &ctrlc, NULL);
   sigaction(SIGTSTP, &ctrlz, NULL);

   while(running){
      // check for finished background processes
      pid_t childPid;
      int childExitStatus;
      while ((childPid = waitpid(-1, &childExitStatus, WNOHANG)) > 0) {
         printf("background pid %d is done: ", childPid);
         if (WIFEXITED(childExitStatus)) { // If exited by status
            printf("exit value %d\n", WEXITSTATUS(childExitStatus));
         }
         else { // terminated by signal
            printf("terminated by signal %d\n", WTERMSIG(childExitStatus));
         }
         fflush(stdout);
      }

      //reinitalize variables
      sprintf(stdinLoc,"");
      sprintf(stdoutLoc,"");
      for (int i = 0; i < numargs; i++){
         args[i] = NULL;
      }
      numargs = 0;
      backgroundProcess = false;

      //get input
      parseCmd(args,&numargs,stdinLoc,stdoutLoc,&backgroundProcess,getpid());

      //find command - checks builtin first, then system. if arg[0] is blank, then it rolls off the bottom.
      if (args[0] == NULL){continue;}
      else if(strcmp(args[0],"cd") == 0){ //cd called
         char workingdir[512];
         strcpy(workingdir,curdir);
         if (numargs == 1){ //no arguments, so go to home
            chdir(getenv("HOME"));
            sprintf(curdir,"%s", getenv("HOME"));
         }
         else { //has an argument
            if (strstr(args[1],"./") == args[1]){ //has relative directory, change to absolute
               strcat(curdir,strstr(args[1],"./")+1); //add the relative location to the end of the current directory
            }
            else{ //absolute path, so change current drectory to input directory
               strcpy(curdir,args[1]);
            }
            if (chdir(curdir) == -1){ //if the attept comes back negative, return to the previous directory
               printf("The directory \"%s\" does not exist, staying in current directory.\n", curdir);
               strcpy(curdir,workingdir);
               chdir(workingdir);
            }
         }
         //printf("%s\n", curdir);
         fflush(stdout);
      }
      else if (strcmp(args[0],"status") == 0) { //status called
         if (WIFEXITED(fgProcessStatus)) {
            // status
            printf("exit value %d\n", WEXITSTATUS(fgProcessStatus));
         }
         else {
            // terminated by signal
            printf("terminated by signal %d\n", WTERMSIG(fgProcessStatus));
         }
      }
      else if (strcmp(args[0],"exit") == 0) {  //exit called
         exit(0);
      }
      else if (strcmp(args[0], "") != 0) { //call a non-built-in
         callCmd(args, &fgProcessStatus, &backgroundProcess, stdinLoc, stdoutLoc);
      }
   }
   return 0;
}
