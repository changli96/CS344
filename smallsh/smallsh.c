//##################\\
//##ZACHARY HORINE##\\
//##CS344---BLOCK2##\\
//##WINTER02132019##\\
//##################\\
//smallsh.c

int COMMAND = 0;
int ARG = 1;
int REDIR_STDIN = 2;
int REDIR_STDOUT = 3;

char linein[2048];
char command[1024];
char* args[512];
int numargs = 0;
char stdin[512];
char stdout[512];



int main() {

}

void cd_cmd() {

}

void exit_cmd() {
   exit(0);
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
