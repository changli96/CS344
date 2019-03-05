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

#define COMMAND 0
#define ARG 1
#define REDIR_STDIN 2
#define REDIR_STDOUT 3
#define COMMENT 4

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

int backgroundProcesses[256];
int numBackgroundProcesses = 0;
int completedProcesses[256];
int completedProcessesExitValues[256];
int numCompletedProcesses = 0;


void parseCmd();
void callChild();

void backgroundManager(){
   allowBackground = !allowBackground;
   if (allowBackground){
      write(1,"\nExiting foreground only mode\n",30);
   } else {
      write(1,"\nEntering foreground only mode\n",31);
   }
   fflush(stdout);
}

int main() {
   //set up signal handlers (ctrlz)
   struct sigaction ctrlz = {0};
   ctrlz.sa_handler = backgroundManager;
   ctrlz.sa_flags = 0;
   sigfillset(&ctrlz.sa_mask);
   sigaction(SIGTSTP, &ctrlz, NULL);
   /*
   //set up signal handlers (ctrlc)
   struct sigaction ctrlc = {0};
   ctrlc.sa_handler = SIG_IGN;
   ctrlc.sa_flags = 0;
   sigfillset(&ctrlc.sa_mask);
   sigaction(SIGINT, &ctrlc, NULL);
   int forkedPid;
   int result = 0;
   */



   getcwd(curdir, sizeof(curdir));
   while(running){

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
      for (int i = 0; i < numargs; i++){
         args[i] = "\0";
      }
      numargs = 0;
      backgroundProcess = false;

      //print prompt
      printf(": ");
      fflush(stdout);
      fgets(linein,2048,stdin);   //get user input
      parseCmd(linein, command, args, &numargs, &backgroundProcess, stdinLoc, stdoutLoc);
      printf("%s%d%d\n", command,numargs,backgroundProcess);
      fflush(stdout);

      if (strcmp(command,"exit") == 0) {  //exit called
         for (int i = 0; i < numBackgroundProcesses; i++) {
            kill(backgroundProcesses[i],SIGTSTP);
         }
         exit(0);
      }
      else if (strcmp(command,"cd") == 0) { //cd called
         char workingdir[512];
         strcpy(workingdir,curdir);
         if (numargs == 1){
            chdir(getenv("HOME"));
            sprintf(curdir,"%s", getenv("HOME"));
         }
         else {
            if (strstr(args[1],"./") == args[1]){
               char *secondhalf = strstr(args[1],"./")+1;
               strcat(curdir,secondhalf);
            }
            else{
               strcpy(curdir,args[1]);
            }
            if (chdir(curdir) == -1){
               printf("The directory \"%s\" does not exist, staying in current directory.\n", curdir);
               strcpy(curdir,workingdir);
               chdir(workingdir);
            }
            //sprintf(curdir,"%s", args[1]);
         }
         printf("%s\n", curdir);
         fflush(stdout);
      }
      else if (strcmp(command,"status") == 0) { //status called
         printf("The last foreground process exited with a value of %d.\n",status);
      }
      else if (strcmp(command, "") != 0) { //call a non-built-in
         callChild(command, args, numargs, backgroundProcess, stdinLoc, stdoutLoc);
      }
      //check for returned processes
      for (int i = 0; i < numBackgroundProcesses; i++) {
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
   }
   return 0;
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
         args[numargs] = word;
         numargs++;
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

void callChild(char command[], char *args[], int numargs, bool backgroundProcess, char stdinLoc[], char stdoutLoc[]) {
   if (allowBackground == false) {backgroundProcess = false;}
   //fork creates a duplicate process. forkedPid is equal to the childPid within the parent, and 0 in the child
   pid_t forkedPid = fork();
   if (forkedPid = -1) { //failed fork
      status = 1;
   }
   else if (forkedPid == 0) { //child process
      //allow CTRL-c to be default as normal
      if (!backgroundProcess) {
         signal(SIGINT, SIG_DFL);
      }

      //IO redirection: stdin
      if (strcmp(stdinLoc,"") != 0){ //stdin var set (open file)
         int infile = open(stdinLoc,O_RDONLY);
         if (infile == -1) { //fail to read file
            printf("Cannot open file \"%s\" for input.\n", stdinLoc);
            status = 1;
         }
         else { //no error, set file as input
            dup2(infile,0);
            fcntl(infile, F_SETFD, FD_CLOEXEC);
         }
      }
      else if (backgroundProcess){ //stdin var not set, default to /dev/null
         int devnull = open("/dev/null",O_WRONLY);
         dup2(devnull,0);
      }
      //IO redirection: stdout
      if (strcmp(stdoutLoc,"") != 0){ //stdout var set (open file)
         int outfile = open(stdoutLoc,O_WRONLY | O_APPEND | O_CREAT);
         if (outfile == -1) { //fail to write file
            printf("Cannot open file for output: %s", stdoutLoc);
            status = 1;
         }
         else { //no error, set file as output
            dup2(outfile,1);
            fcntl(outfile, F_SETFD, FD_CLOEXEC);
         }
      }
      else if(backgroundProcess){ //stdin var not set, default to /dev/null
         int devnull = open("/dev/null",O_WRONLY);
         dup2(devnull,1);
      }

      //command execution
      int result = execvp(command, args);
      //check result for error
      if (result < 0) {
         printf("Error on execution\n");
         fflush(stdout);
         result = 0;
      }
      exit(1);
   }
   else{ //parent process
      if (backgroundProcess){
         printf("Background process id is %d\n", forkedPid);
         fflush(stdout);
         backgroundProcesses[numBackgroundProcesses] = forkedPid;
         numBackgroundProcesses++;
         backgroundProcess = false;
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
