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

void *game();
void *timer();

int main(){
}

void *timer() {
}
void *game() {
}
