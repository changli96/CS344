//##################\\
//##ZACHARY HORINE##\\
//##CS344---BLOCK2##\\
//##WINTER02132019##\\
//##################\\
//Buildrooms.c

#include <unistd.h>
#include <stdio.h>
void printRoomtoFile(int x);
bool IsGraphFull();
void AddRandomConnection();
int GetRandomRoom();
bool CanAddConnectionFrom(int x);
bool ConnectionAlreadyExists(int x, int y);
void ConnectRoom(int x, int y);
bool IsSameRoom(int x, int y);

int main() {
   
}



}

// Returns true if all rooms have 3 to 6 outbound connections, false otherwise
bool IsGraphFull() {
   for (int i = 0; i < 7; i++) {
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
      printf("A: %d\n",A);
      if (CanAddConnectionFrom(A) == true)
         break;
   }
   do {
      B = GetRandomRoom();
      printf("Pick B: %d\n", B);
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
   for (int j = 0; j < rooms[x].numConnections; j++) {
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
