//##################\\
//##ZACHARY HORINE##\\
//##CS344---BLOCK2##\\
//##WINTER02132019##\\
//##################\\
//Adventure.c

#include <unistd.h>
#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>

typedef int bool;
#define true 1
#define false 0

const int NONE = 0;
const int START_ROOM = 1;
const int MID_ROOM = 2;
const int END_ROOM = 3;

struct Room{
   char name[15];
   int id;
   int roomType;
   int numConnections;
   int connections[6];
   char connectionNames[6];
};
struct Room rooms[7];
FILE timeFile;
bool countTimer = true;

void *game();
void *timer();

int main(){
}

void *timer() {
   time_t now = time(NULL);
   local = localtime(&now);
   char hrTime[50];
   while(countTimer) {
      pthread_mutex_lock(&mutex);
      strftime(hrTime, sizeof(hrTime), "%I:M%p, %A, %B %d, %Y", local); //HH:MMpm, WEEK, DAY dd, YYYY ex. 3:23pm, Wednesday, Febuary 13, 2019
      timeFile = fopen("./currentTime.txt","w");
      fprintf(timeFile,"%s",hrTime);
      fclose(timeFile);
      pthread_mutex_unlock(&mutex);
   }
}*/


void game() {
//   pthread_mutex_lock(&mutex);

   DIR *topLvlDir = opendir(".");
   struct dirent *newestdir;
   struct dirent *checkdir = readdir(topLvlDir);
   time_t newestdirTime = 0;

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
      checkdir = readdir(topLvlDir);
   }
   closedir(topLvlDir);

   if (newestdirTime == 0) {
      printf("ERR: NO ROOM FOLDERS DETECTED");
      exit(1);
   }

   char curdir[100];
   sprintf(curdir,"./%s",newestdir->d_name);
   printf("%s\n", curdir);
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
         printf("%s\n",fileName);
         sprintf(filePath, "%s/%s",curdir,fileName);
         printf("%s\n",filePath);
         FILE *file;
         file = fopen(filePath,"r");
         char line[50];
         rooms[fileNum].id = fileNum;
         rooms[fileNum].numConnections = 0;
         while (fgets(line, sizeof(line), file)) {
            if (line[5] == 'N') { //ROOM NAME:
               char name[15];
               for (int i = 0; i < 25; i++){
                  name[i] = line[i+11];
                  if (line[i+11] == '\n'){
                     name[i] = '\0';
                  }
               }
               sprintf(rooms[fileNum].name,"%s",name);
            }
            if (line[0] == 'C') { //CONNECTION x:
               char conn[25];
               for (int i = 0; i < 25; i++){
                  conn[i] = line[i+14];
                  if (line[i+14] == '\n'){
                     conn[i] = '\0';
                  }
               }
               rooms[fileNum].connectionNames[rooms[fileNum].numConnections] = conn[0];
               rooms[fileNum].numConnections++;
            }
            if (line[5] == 'T') { //ROOM TYPE:
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
      checkdir = readdir(roomDir);
   }
   for (int i = 0; i < 7; i++){  //each room
      for (int j = 0; j < rooms[i].numConnections; j++){ //each connection
         for (int k = 0; k < 7; k++){  //search for connection id
            //printf("%s:%s,%d\n",rooms[k].name,rooms[i].name,i);
            if (rooms[i].connectionNames[j] == rooms[k].name[0]){
               //printf("%s%s",rooms[i].connectionNames[j],rooms[k].name);
               rooms[i].connections[j] = k;
            }
         }
      }
   }

}
