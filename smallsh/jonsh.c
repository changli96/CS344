//✔ 💢
// Make sure to use fflush() after every output
// Program must have exit, cd, and status included
// Input that starts with # are comments and must be supported. Keyword begining

// Syntax for use is: command [arg1 arg2 ...] [< input_file] [> output_file] [&]
// Last item & means that it is ment to be run in the background
// Must support 2048 chars max and 512 worth of args, no error chechking on input syntax
// $$ must be replaced with the shell id
// If the program being run isn't a background (&) then then the shell needs to wait() for the forground to finish
// Stdin/out on background (&) should be redirected to /dev/null
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
int allowBackgroudnProcess = 1;
int ctrlzTOGGLE = 0;



/*
    Kill all the children
*/
void order66(int numChildren,pid_t childrenID[512],int forgroundChild){
    int i;
    if(forgroundChild != -1){
        kill(forgroundChild,SIGTSTP);
    }
    if(numChildren > 0){
        for(i = 0;i < numChildren;i++){
            kill(childrenID[i],SIGTSTP);// Kill each and every child
        }
    }
}

char* replace(char* input, char* a, char* b){// issues with $$$
    char* c;
    int inputLength = strlen(input);
    int aLength = strlen(a);
    int bLength = strlen(b);
    for(c = input; c = strstr(c,a); c++){
        if(aLength != bLength){
            memmove(c+bLength,c+aLength,inputLength - (c - input) + bLength);
            memcpy(c,b,bLength);
        }
    }
}

/*
✔ 💢
The goal of this function is to grab everything we need for the exec function.
This includes a couple things:
[✔] 1. Tokenise input on spaces: This way we can examine a string one atrabute at a time.
[✔] 2. Handle special chars: <, >, as well as & will need to be handled
[✔] 3. Put all other things into one large primitive that allows exec to acces all of the other random args the user puts in.
[✔] 4. Sanatise input makeing sure that total size of args is less than 512, and replacing all &&
To do this we need:
*/
void getInput(char **execArgs, int *execArgsSize, char input_file[512], char output_file[512], int *backgroundProcess,int pid){
    int i;
    const char s[4] = " \n";
    char charID[10];
    sprintf(charID,"%d",pid);
    char arguments[2048];

    // Get input
    printf(": ");
    fflush(stdout);
    fflush(stdin);
    fgets(arguments, 2048, stdin);
    char *token = strtok(arguments, s);

    while(token != NULL){
        *backgroundProcess = 0;
        if(token[0] == '#'){// Look for comments if we find some then we are done.
            return;
        }
        else if(!strcmp(token,"<")){
            token = strtok(NULL,s);
            strcpy(input_file,token);
        }
        else if(!strcmp(token,">")){
            token = strtok(NULL,s);
            strcpy(output_file,token);
        }
        else if(!strcmp(token,"&")){
            *backgroundProcess = 1;
        }
        else{
            execArgs[*execArgsSize] = token;
            *execArgsSize += 1;
        }

        token = strtok(NULL,s);
    }

    //Before we exit we need to do a tally on the size of args, and replace any &&

    for(i = 0; i < *execArgsSize; i++){
        replace(execArgs[i],"$$",charID);
    }
    replace(input_file,"$$",charID);
    replace(output_file,"$$",charID);
    execArgs[*execArgsSize] = NULL;
    return;
}

void forgroundOnlyMode(int signum){
    allowBackgroudnProcess = !allowBackgroudnProcess;
    if(allowBackgroudnProcess){
        printf("\nExiting foreground-only mode\n");
        fflush(stdout);
    }
    else{
        printf("\nEntering foreground-only mode (& is now ignored)\n");
        fflush(stdout);
    }
    ctrlzTOGGLE = 1;
}

void kec(){};

int main(){
   int i,j,k;
   int mainPid = getpid();
   int backgroundProcess = 0;
   int execArgsSize = 512;
   int runCondition = 1;
   int devNull = open("/dev/null",O_WRONLY);
   int targetFile;
   int sourceFile;
   char *execArgs[512];
   char input_file[512] = "";
   char output_file[512] = "";
   char workingDir = "~";
   struct sigaction ctrlz = {0};
   struct sigaction ctrlc = {0};
   ctrlc.sa_handler = SIG_IGN;
   ctrlz.sa_handler = forgroundOnlyMode;
   ctrlc.sa_flags = 0;
   ctrlz.sa_flags = 0;
   sigfillset(&ctrlc.sa_mask);
   sigfillset(&ctrlz.sa_mask);

    //Flags
    int execResult = 0;
    int chdirResult = 0;

    //Children
    pid_t pid;
    pid_t deadChild;
    int childStatus = -5;// Forground
    int bchildStatus = -5;// Background
    int forgroundChild = -5;
    int numChildren = 0;
    int numDeaths = 0;
    pid_t deathsToAnnounce[512];
    int causeOfDeath[512]; // realy exit status for all dead children
    pid_t childrenID[512];

    for(i = 0; i < 512;i++){
        childrenID[i] = -1;
    }


    //Signal handling
	sigaction(SIGINT, &ctrlc, NULL);
	sigaction(SIGTSTP, &ctrlz, NULL);



    //Begining of the shell
    while(runCondition){

        //Cleanup after last comand
        fflush(stdin);
        fflush(stdout);
        for(i = 0; i < execArgsSize; i++){// Sanatise execArgs
            execArgs[i] = NULL;
        }

        for(i = 0;i < numDeaths;i++){
            if(WIFEXITED(causeOfDeath[i]) > 0){
                if(causeOfDeath[i] == 256){childStatus = 1;}
                printf("Background pid: %d is done: exit value %d\n",deathsToAnnounce[i],WEXITSTATUS(causeOfDeath[i]));
                fflush(stdout);
            }
            else{
                printf("Background pid: %d is done: terminated by signal %d\n",deathsToAnnounce[i],WTERMSIG(causeOfDeath[i]));
                fflush(stdout);
            }
            fflush(stdout);
        }
        numDeaths = 0;
        execArgsSize = 0;
        strcpy(input_file,"");
        strcpy(output_file,"");

        //Start next command
        if(!ctrlzTOGGLE){
            getInput(execArgs,&execArgsSize,input_file,output_file,&backgroundProcess,mainPid);
        }

        //Handle built in
        if(execArgs[0] == NULL || ctrlzTOGGLE){ctrlzTOGGLE = 0;}// skip empty input
        else if(!strcmp(execArgs[0],"cd")){
            if(execArgsSize == 1){// If there are no args go to home
                chdirResult = chdir(getenv("HOME"));
                workingDir = getenv("HOME");
                fflush(stdout);
            }
            else{// if there are args use those
                if(execArgs[1][0] == "." && execArgs[1][1] == "/"){// If we have ./ then we need to replace that with the current path
                    replace(execArgs[1],"./",workingDir);
                }
                if(execResult != 0){
                  printf("Exec returned %d\n",execResult);
                  fflush(stdout);
                }
                chdirResult = chdir(execArgs[1]);
                workingDir = execArgs[1];
            }
        }
        else if(!strcmp(execArgs[0],"status")){
            printf("exit value %d\n",childStatus);
            fflush(stdout);
        }
        else if(!strcmp(execArgs[0],"exit")){
            order66(numChildren,childrenID,forgroundChild);
            runCondition = 0;
        }
        else if(!strcmp(execArgs[0],"children")){
            printf("The children are:\n");
            fflush(stdout);
            for(i = 0;i < numChildren;i++){
                printf("%d: %d\n",i,childrenID[i]);
                fflush(stdout);
            }
        }
        else {// We are doing forking
            if(!allowBackgroudnProcess){backgroundProcess = 0;}//Background mode
            if(execArgs[0] != NULL){// Make sure we have a command to run
                pid = fork();
                if(pid == -1){//TODO we failed to fork
                    childStatus = 1;
                }
                else if(pid == 0){// Child

                    //UNDO CTRL-C & CTRL-Z
                    if(!backgroundProcess){
                        signal(SIGINT, SIG_DFL);
                        signal(SIGTSTP, SIG_DFL);
                    }

                    //Handle input & output redirection
                    if(strcmp(output_file,"")){
                        targetFile = open(output_file, O_WRONLY | O_CREAT | O_TRUNC, 0644);
                        if (targetFile == -1) { printf("cannot open %s for output\n",output_file);fflush(stdout); exit(1); childStatus = 1;}
                        dup2(targetFile,1);
                        fcntl(targetFile, F_SETFD, FD_CLOEXEC);
                    }
                    else if(backgroundProcess){
                        dup2(devNull,1);
                    }
                    if(strcmp(input_file,"")){
                        sourceFile = open(input_file, O_RDONLY);
                        if (sourceFile == -1) { printf("cannot open %s for input\n",input_file);fflush(stdout); exit(1); childStatus = 1;}
                        dup2(sourceFile,0);
                        fcntl(sourceFile, F_SETFD, FD_CLOEXEC);
                    }
                    else if(backgroundProcess){
                        dup2(devNull,0);
                    }

                    // Call the command
                    execResult = execvp(execArgs[0],execArgs);
                    // If it fails throw error
                    if(execResult < 0){
                        printf("Exec error\n");
                        fflush(stdout);
                        execResult = 0;
                    }
                    exit(1);
                }
                else{// Parent
                    if(backgroundProcess){
                        printf("Background pid is %d\n",pid);
                        fflush(stdout);
                        childrenID[numChildren] = pid;
                        numChildren++;
                        backgroundProcess = 0;
                    }
                    else{
                        deadChild = waitpid(pid,&childStatus,0);
                        if(childStatus == 256){childStatus = 1;}
                        else if(WIFEXITED(childStatus) <= 0){
                            printf("terminated by signal %d\n",WTERMSIG(childStatus));
                            fflush(stdout);
                        }
                    }
                }
            }
        }

        // No mater what the last thing we do is check for dead
        for(i = 0;i < numChildren;i++){
            if((deadChild = waitpid(childrenID[i],&bchildStatus,WNOHANG)) > 0){
                for(j = i; j < numChildren;j++){
                    childrenID[j] = childrenID[j+1];
                }
                deathsToAnnounce[numDeaths] = deadChild;
                causeOfDeath[numDeaths] = bchildStatus;
                numDeaths++;
                numChildren--;
                i--;
            }
        }
    }
    return 0;
}
