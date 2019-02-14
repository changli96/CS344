//##################\\
//##ZACHARY HORINE##\\
//##CS344---BLOCK2##\\
//##WINTER02132019##\\
//##################\\
//Adventure.c

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <string.h>
#include <dirent.h>
#include <pthread.h>

typedef int bool;
#define true 1
#define false 0

const int NONE = 0;
const int START_ROOM = 1;
const int MID_ROOM = 2;
const int END_ROOM = 3;
int i,j,k;

struct Room{
   char name[15];
   int id;
   int roomType;
   int numConnections;
   int connections[6];
   char connectionNames[6];
};
struct Room rooms[7];

pthread_mutex_t mutex;
pthread_t gameT;
pthread_t timeT;

FILE *timeFile;
bool countTimer = true;

void *game();
void *timer();

int main(){
   int gameThreadInt;
   int timeThreadInt;

   pthread_mutex_init(&mutex,NULL) != 0;
   gameThreadInt=pthread_create(&gameT,NULL,game,NULL);  //init threads
   timeThreadInt=pthread_create(&timeT,NULL,timer,NULL);
   pthread_join(gameT,NULL);  //threads will wait for the other thread's completion
   pthread_join(timeT,NULL);
   pthread_mutex_destroy(&mutex);
}

void *timer() {
   while(countTimer) {  //constantly running (when allowed)
      time_t now = time(NULL);   //get raw time
      struct tm *local = localtime(&now);
      char hrTime[50];
      pthread_mutex_lock(&mutex);
      strftime(hrTime, sizeof(hrTime), "%I:%M%p, %A, %B %d, %Y", local); //for mat time: HH:MMpm, WEEK, DAY dd, YYYY ex. 3:23pm, Wednesday, Febuary 13, 2019
      timeFile = fopen("./currentTime.txt","w");   //write time to file
      fprintf(timeFile,"%s",hrTime);
      fclose(timeFile);
      pthread_mutex_unlock(&mutex);
   }
}


void *game() {
   pthread_mutex_lock(&mutex);

   DIR *topLvlDir = opendir(".");   //current directory
   struct dirent *newestdir;
   struct dirent *checkdir = readdir(topLvlDir);
   time_t newestdirTime = 0;
   //walk through all directories
   while (checkdir != NULL) {
      struct stat res;
      stat(checkdir->d_name,&res);
      time_t lastmod = res.st_mtime;
      if (S_ISDIR(res.st_mode) && checkdir->d_name[0] != '.') { //is a directory and is not the "." or ".." directories
         if (lastmod > newestdirTime) { //current dir time is newer than previous best
            newestdir = checkdir;
            newestdirTime = lastmod;
         }
      }
      checkdir = readdir(topLvlDir); //get next directory
   }
   closedir(topLvlDir);

   if (newestdirTime == 0) {
      printf("ERR: NO ROOM FOLDERS DETECTED");
      exit(1);
   }

   char curdir[100];
   sprintf(curdir,"./%s",newestdir->d_name);
   //printf("%s\n", curdir);
   DIR *roomDir = opendir(curdir);
   checkdir = readdir(roomDir);

   char fileName[25];
   char filePath[100];
   int startRoom = -1;
   int endRoom = -1;
   int fileNum = 0;
   //read in files
   while (checkdir != NULL) {
      if (checkdir->d_name[0] != '.'){ //name is not "." or "..", or a hidden file
         sprintf(fileName, "%s",checkdir->d_name);
         sprintf(filePath, "%s/%s",curdir,fileName);
         FILE *file;
         file = fopen(filePath,"r");
         char line[50];
         rooms[fileNum].id = fileNum;  //set id
         rooms[fileNum].numConnections = 0;  //initialize numConnections
         while (fgets(line, sizeof(line), file)) {
            if (line[5] == 'N') { //ROOM NAME: ______
               char name[15];
               for (i = 0; i < 25; i++){
                  name[i] = line[i+11];
                  if (line[i+11] == '\n'){   //remove trailing newlines
                     name[i] = '\0';
                  }
               }
               sprintf(rooms[fileNum].name,"%s",name);
            }
            if (line[0] == 'C') { //CONNECTION x: ______
               char conn[25];
               for (i = 0; i < 25; i++){
                  conn[i] = line[i+14];
                  if (line[i+14] == '\n'){   //remove trailing newlines
                     conn[i] = '\0';
                  }
               }
               rooms[fileNum].connectionNames[rooms[fileNum].numConnections] = conn[0];
               rooms[fileNum].numConnections++;
            }
            if (line[5] == 'T') { //ROOM TYPE: ______   sets roomType in room, as well as sets global start and end variables
               if (line[11] == 'S'){ //START_ROOM
                  rooms[fileNum].roomType = START_ROOM;
                  startRoom = fileNum;
               }
               if (line[11] == 'M'){ //MID_ROOM
                  rooms[fileNum].roomType = MID_ROOM;
               }
               if (line[11] == 'E'){ //END_ROOM
                  rooms[fileNum].roomType = END_ROOM;
                  endRoom = fileNum;
               }
            }
         }
         int fclose(FILE *file);
         fileNum++;
      }
      checkdir = readdir(roomDir);  //get next file
   }
   //convert text values to integer representations
   for (i = 0; i < 7; i++){  //each room
      for (j = 0; j < rooms[i].numConnections; j++){ //each connection
         for (k = 0; k < 7; k++){  //search for connection id
            if (rooms[i].connectionNames[j] == rooms[k].name[0]){
               rooms[i].connections[j] = k;
            }
         }
      }
   }

   char linein[100];
   int currentRoom = startRoom;
   int steps = 0;
   int path[100];
   bool lastinstTime = false;
   while (currentRoom != endRoom){  //start game
      if (lastinstTime != true){ //check if last command run was "time" if so, don't print room info
         printf("CURRENT LOCATION: %s\n",rooms[currentRoom].name);
         printf("POSSIBLE CONNECTIONS:");
         for (i = 0; i < rooms[currentRoom].numConnections; i++){   //walk through connections and print out names
            if (i == rooms[currentRoom].numConnections-1){
               printf(" %s%s",rooms[rooms[currentRoom].connections[i]].name,".\n"); //if last one, print a period
            }
            else{
               printf(" %s%s",rooms[rooms[currentRoom].connections[i]].name,","); //otherwise, print a comma
            }
         }
         lastinstTime = false;
      }
      printf("WHERE TO? >");
      fgets(linein,100,stdin);   //get user input
      printf("\n");
      if (strcmp(linein,"time\n") == 0) { //open other thread, sleep for 1 sec, close other thread
         char timetxt[50];
         pthread_mutex_unlock(&mutex);
         sleep(1);
         pthread_mutex_lock(&mutex);
         timeFile = fopen("./currentTime.txt","r");   //read in time from file
         fgets(timetxt, sizeof(timetxt), timeFile);
         printf(" %s\n\n", timetxt);
         int fclose(FILE *timeFile);
         lastinstTime = true;
      }
      else {   //check for match against available connections
         bool togo = false;
         for (i = 0; i < rooms[currentRoom].numConnections; i++){
            char testline[25];
            sprintf(testline,"%s\n",rooms[rooms[currentRoom].connections[i]].name); //add '\n' on to end of input to match user input
            if (strcmp(linein,testline) == 0){
               currentRoom = rooms[rooms[currentRoom].connections[i]].id;
               path[steps] = currentRoom; //add room id to path
               steps++;
               togo = true;
            }
         }
         if (togo == false){  //does not match any room or command
            printf("HUH? I DON’T UNDERSTAND THAT ROOM. TRY AGAIN.\n\n");
         }
      }
   }
   //ending
   printf("YOU HAVE FOUND THE END ROOM. CONGRATULATIONS!\n");
   printf("YOU TOOK %d STEPS. YOUR PATH TO VICTORY WAS:\n", steps);
   for (i = 0; i < steps; i++){
      printf("%s\n",rooms[path[i]].name); //read path back out
   }
   countTimer = false;  //stop timer thread
   pthread_mutex_unlock(&mutex);
}
