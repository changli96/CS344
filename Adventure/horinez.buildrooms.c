//##################\\
//##ZACHARY HORINE##\\
//##CS344---BLOCK2##\\
//##WINTER02132019##\\
//##################\\
//Buildrooms.c

#include <unistd.h>
#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>

typedef int bool;
#define true 1
#define false 0

const int NONE = 0;
const int START_ROOM = 1;
const int MID_ROOM = 2;
const int END_ROOM = 3;
int i,j;

char *names[10] = {"Apple","Banana","Coconut","Date","Elderberry","Fig","Grape","Huckleberry","Imbe","Jackfruit"};

struct Room{
   char *name;
   int id;
   int roomType;
   int numConnections;
   int connections[6];
};
struct Room rooms[7];

void printRoomtoFile(int x);
bool IsGraphFull();
void AddRandomConnection();
int GetRandomRoom();
bool CanAddConnectionFrom(int x);
bool ConnectionAlreadyExists(int x, int y);
void ConnectRoom(int x, int y);
bool IsSameRoom(int x, int y);

int main() {
   srand(time(NULL));   // random initialization
   int ex1 = 0;
   int ex2 = 0;
   int ex3 = 0;
   while (ex1 == ex2 || ex1 == ex3 || ex2 == ex3) {
      ex1 = rand() % 10;
      ex2 = rand() % 10;
      ex3 = rand() % 10;
   }
   int namejumper = 0;
   for (i = 0; i < 7; i++) {
      struct Room temp;
      if (i+namejumper == ex1 || i+namejumper == ex2 || i+namejumper == ex3){namejumper++;}
      temp.name = names[i+namejumper];
      temp.id = i;
      temp.roomType = NONE;
      temp.numConnections = 0;
      for (j = 0; j < 6; j++) {
         temp.connections[j] = -1;
      }
      rooms[i] = temp;
   }

   // Create all connections in graph
   while (IsGraphFull() == false)
   {
      AddRandomConnection();
   }

   //add roomtypes
   int startroom = GetRandomRoom();
   rooms[startroom].roomType = START_ROOM;
   int endroom = GetRandomRoom();
   while (endroom == startroom) {endroom = GetRandomRoom();}
   rooms[endroom].roomType = END_ROOM;
   for (i = 0; i < 7; i++) {
      if (rooms[i].roomType == NONE) {
         rooms[i].roomType = MID_ROOM;
      }
   }

   char dirName[100];
   sprintf(dirName,"./horinez.rooms.%d",getpid());
   mkdir(dirName,0755);
   for (i = 0; i < 7; i++) {
      printRoomtoFile(i);
   }

}

void printRoomtoFile(int x) {
   char fileName[100];
   char roomName[25];
   char connection[25];
   char roomType[10];
   sprintf(fileName,"./horinez.rooms.%d/%s_room",getpid(),rooms[x].name);
   FILE *file = fopen(fileName,"w");
   sprintf(roomName,"ROOM NAME: %s\n",rooms[x].name);
   fprintf(file,roomName);
   for (j = 0; j < rooms[x].numConnections; j++) {
      sprintf(connection,"CONNECTION %d: %s\n",j+1,rooms[rooms[x].connections[j]].name);
      fprintf(file,connection);
   }
   if (rooms[x].roomType == START_ROOM) {sprintf(roomType,"ROOM TYPE: %s\n","START_ROOM");}
   else if (rooms[x].roomType == MID_ROOM) {sprintf(roomType,"ROOM TYPE: %s\n","MID_ROOM");}
   else if (rooms[x].roomType == END_ROOM) {sprintf(roomType,"ROOM TYPE: %s\n","END_ROOM");}
   fprintf(file,roomType);
   int fclose(FILE *file);

}

// Returns true if all rooms have 3 to 6 outbound connections, false otherwise
bool IsGraphFull() {
   for (i = 0; i < 7; i++) {
      if (rooms[i].numConnections < 3 || rooms[i].numConnections > 6) {
         return false;
      }
   }
}

// Adds a random, valid outbound connection from a Room to another Room
void AddRandomConnection() {
   int A;
   int B;

   while(true) {
      A = GetRandomRoom();
      if (CanAddConnectionFrom(A) == true)
         break;
   }
   do {
      B = GetRandomRoom();
   } while(CanAddConnectionFrom(B) == false || IsSameRoom(A, B) == true || ConnectionAlreadyExists(A, B) == true);

   ConnectRoom(A, B);
}

// Returns a random Room, does NOT validate if connection can be added
int GetRandomRoom() {
   int r = rand() % 7;
   return r;
}

// Returns true if a connection can be added from Room x (< 6 outbound connections), false otherwise
bool CanAddConnectionFrom(int x) {
   if (rooms[x].numConnections < 6) {
      return true;
   }
   else {
      return false;
   }
}
// Returns true if a connection from Room x to Room y already exists, false otherwise
bool ConnectionAlreadyExists(int x, int y) {
   for (j = 0; j < rooms[x].numConnections; j++) {
      if (rooms[x].connections[j] == y){
         return true;
      }
   }
   return false;
}

// Connects Rooms x and y together, does not check if this connection is valid
void ConnectRoom(int x, int y) {
  rooms[x].connections[rooms[x].numConnections] = y;
  rooms[y].connections[rooms[y].numConnections] = x;
  rooms[x].numConnections += 1;
  rooms[y].numConnections += 1;
}

// Returns true if Rooms x and y are the same Room, false otherwise
bool IsSameRoom(int x, int y) {
   if (rooms[x].id == rooms[y].id) {
      return true;
   }
   else {
      return false;
   }
}
