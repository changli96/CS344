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
int countTimer = 1;

void *game();
void *timer();

int main(){
}

void *timer() {
   time_t now = time(NULL);
   local = localtime(&now);
   char hrTime[50];
   while(countTimer == 1) {
      strftime(hrTime, sizeof(hrTime), "%I:M%p, %A, %B %d, %Y", local); //HH:MMpm, WEEK, DAY dd, YYYY ex. 3:23pm, Wednesday, Febuary 13, 2019
      timeFile = fopen("./currentTime.txt","w");
      fprintf(timeFile,"%s",hrTime);
      fclose(timeFile);
   }
}


void *game() {
}
