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
   char *name;
   int id;
   int roomType;
   int numConnections;
   int connections[6];
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
}
