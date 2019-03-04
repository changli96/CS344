//##################\\
//##ZACHARY HORINE##\\
//##CS344---BLOCK2##\\
//##WINTER02132019##\\
//##################\\
//smallsh.c

#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <signal.h>
#include <string.h>
#include <unistd.h>

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
bool backgroundProcess = false;
char curdir[512];

bool running = true;
bool allowBackground = true;
int status;
bool ctrlztoggle = false;

int backgroundProcesses[256];
int numBackgroundProcesses = 0;
int completedProcesses[256];
int completedProcessesExitValues[256];
int numCompletedProcesses = 0;


void parseCmd();
void status_cmd();
void cd_cmd();
void backgroundManager();
void callChild();


int main() {

   struct sigaction ctrlz = {0};
   ctrlz.sa_handler = backgroundManager;
   ctrlz.sa_flags = 0;
   sigfillset(&ctrlz.sa_mask);
   sigaction(SIGTSTP, &ctrlz, NULL);

   struct sigaction ctrlc = {0};
   ctrlc.sa_handler = SIG_IGN;
   ctrlc.sa_flags = 0;
   sigfillset(&ctrlc.sa_mask);
   sigaction(SIGINT, &ctrlc, NULL);
   int forkedPid;
   int result = 0;



   getcwd(curdir, sizeof(curdir));
   while(running){
      //check for returned processes
      for (int i = 0; i < numBackgroundProcesses; i++) {
         int status;
         int child = waitpid(backgroundProcesses[i],&status,WNOHANG);
         if (child > 0){
            for (int j = 0; j < numBackgroundProcesses; j++){
               backgroundProcesses[j] = backgroundProcesses[j+1];
            }
            numBackgroundProcesses--;
            i--; //keeps i in line with the head of the array, as we just removed one
            completedProcesses[numCompletedProcesses] = child;
            completedProcessesExitValues[numCompletedProcesses] = status;
            numCompletedProcesses++;
         }
      }

      //roll through exited background processes and print them to the screen
      for (int i = 0; i < numCompletedProcesses; i++) {
         if(completedProcessesExitValues[i] == 256){status = 1;}
         printf("Process %d exited with a value of %d.\n", completedProcesses[i], completedProcessesExitValues[i]);
      }
      fflush(stdout);
      numCompletedProcesses = 0;



      //reinitalize variables
      sprintf(command,"");
      sprintf(stdinLoc,"");
      sprintf(stdoutLoc,"");
      numargs = 0;
      backgroundProcess = false;
      if(!ctrlztoggle){
         //print prompt
         printf(": ");
         fflush(stdout);
         fgets(linein,2048,stdin);   //get user input
         parseCmd();
         fflush(stdout);
      }
      if (ctrlztoggle){ctrlztoggle = false;}
      else if (strcmp(command,"exit") == 0) {
         for (int i = 0; i < numBackgroundProcesses; i++) {
            kill(backgroundProcesses[i],SIGTSTP);
         }
         exit(0);
      }
      else if (strcmp(command,"cd") == 0) {
         cd_cmd();
      }
      else if (strcmp(command,"status") == 0) {
         printf("The last foreground process exited with a value of %d.",status);
      }
      else if (strcmp(command, "") != 0) {
         callChild();
      }
   }
   return 0;
}

void cd_cmd() {
   if (numargs == 0){
      chdir(getenv("HOME"));
      sprintf(curdir,"%s", getenv("HOME"));
   }
   else {
      if (strstr(args[0],"./") == args[0]){
         char *secondhalf = strstr(args[0],"./")+1;
         strcat(curdir,secondhalf);
      }
      else{
         strcpy(curdir,args[0]);
      }
      chdir(curdir);
      //sprintf(curdir,"%s", args[0]);
   }
   printf("%s\n", curdir);
   fflush(stdout);
}

void backgroundManager() {
   if (allowBackground) {
      allowBackground = false;
   }
   else {
      allowBackground = true;
   }
   ctrlztoggle = true;
}

void parseCmd() {
   int nextwordType = COMMAND;
   numargs = 0;
   if (strstr(linein,"#") == linein) {
      //this is a comment, exit parsing and return blank string
      sprintf(command,"%s","");
      fflush(stdout);
      return;
   }
   char *word = strtok(linein, " \n");
   while (word != NULL) {
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
         args[numargs] = word;
         numargs++;
      }
      if (word != NULL){
         word = strtok(NULL," \n");
      }
   }
}

void callChild() {
   //fork creates a duplicate process. forkedPid is equal to the childPid within the parent, and 0 in the child
   forkedPid = fork();
   if (forkedPid = -1) {
      //failed fork
      status = 1;
   }
   else if (forkedPid == 0) {
      //if not a background process, set the ctrlc and ctrlz values back to operational
      if (backgroundProcess == false || allowBackground == false){
         signal(SIGINT, SIG_DFL);
         signal(SIGTSTP, SIG_DFL);
      }
      //set the input and output sources. if not set, then they will default to stdin and stdout
      if (strcmp(stdinLoc,"") != 0){
         int infile = open(stdinLoc,O_RDONLY);
         if (infile == -1) { //fail to read file
            printf("Cannot open file for input: %s", stdinLoc);
            status = 1;
         }
         else {
            dup2(infile,0);
            fcntl(infile, F_SETFD, FD_CLOEXEC);
         }
      }
      else if(backgroundProcess && allowBackground){
         int devnull = open("/dev/null",O_WRONLY);
         dup2(devnull,0);
      }
      if (strcmp(stdinLoc,"") != 0){
         int outfile = open(stdoutLoc,O_WRONLY | O_APPEND | O_CREAT);
         if (outfile == -1) { //fail to write file
            printf("Cannot open file for output: %s", stdoutLoc);
            status = 1;
         }
         else {
            dup2(outfile,1);
            fcntl(outfile, F_SETFD, FD_CLOEXEC);
         }
      }
      else if(backgroundProcess && allowBackground){
         int devnull = open("/dev/null",O_WRONLY);
         dup2(devnull,1);
      }

      result = execvp(command,args);
      if (result < 0){
         printf("Bad Command!\n");
         fflush(stdout);
         result = 0;
      }
   }
   else{
      if (backgroundProcess && allowBackground){
         printf("Background process id is %d\n", forkedPid);
         fflush(stdout);
         backgroundProcesses[numBackgroundProcesses] = forkedPid;
         numBackgroundProcesses++;
      }
      else{ //handle foreground waiting
         int child = waitpid(forkedPid,&status,0);
         if(status == 256){status = 1;}
         else if(WIFEXITED(status) <= 0){
             printf("Process exited with value of %d\n",WTERMSIG(status));
             fflush(stdout);
         }
      }
   }
}
