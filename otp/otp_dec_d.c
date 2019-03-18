//##################\\
//##ZACHARY HORINE##\\
//##CS344---BLOCK4##\\
//##WINTER03172019##\\
//##################\\
//otp_dec_d.c

#include <unistd.h>
#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

int main(int argc, char *argv[]) {
   if (argc != 2) {fprintf(stderr,"Usage: otp_dec_d [port]\n"); exit(1);}

   char chars[28] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ ";
   int pid, listenSocket, establishedConnection, charsRead,numPackets,i,j,k,l;
   int packetSize = 512;
   int truePacketSize = 511;
   socklen_t sizeOfClientInfo;

   struct sockaddr_in serverAddress, clientAddress;
   memset((char *)&serverAddress, '\0', sizeof(serverAddress)); // Clear out the address struct
   int portNumber = atoi(argv[1]); // Get the port number, convert to an integer from a string
   serverAddress.sin_family = AF_INET; // Create a network-capable socket
   serverAddress.sin_port = htons(portNumber); // Store the port number
   serverAddress.sin_addr.s_addr = INADDR_ANY; // Any address is allowed for connection to this process

   // Set up socket
   if (listenSocket < 0) {fprintf(stderr,"Error: Could not open socket\n"); exit(1);}
   listenSocket = socket(AF_INET, SOCK_STREAM, 0); // Create the socket

   // Enable socket and start listening
   if (bind(listenSocket, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) < 0){ // Connect socket to port
      fprintf(stderr,"Error: Could not bind socket to port %d\n", portNumber);
      exit(1);
   }
   listen(listenSocket, 5); // Flip the socket on - it can now receive up to 5 connections

   char keybuffer[packetSize];// For storeing key packets
   while(1){
      // Accept a connection, blocking if one is not available until one connects
      sizeOfClientInfo = sizeof(clientAddress); // Get the size of the address for the client that will connect
      establishedConnection = accept(listenSocket, (struct sockaddr *)&clientAddress, &sizeOfClientInfo); // Accept
      if (establishedConnection < 0) {fprintf(stderr,"Error: Did not accept\n"); exit(1);}

      pid = fork();
      if(pid == -1){// Failed to create child
         exit(1);
      }
      else if(pid == 0){// Child process

         memset(keybuffer, '\0', packetSize);
         charsRead = recv(establishedConnection, keybuffer, 7, 0); // Read the client's message from the socket
         if (charsRead < 0) {fprintf(stderr,"Error: Could not read from socket\n"); exit(1);}
         if(!strcmp(keybuffer,"otp_dec")){
            charsRead = send(establishedConnection, "ack", 3, 0);
            if (charsRead < 0) {fprintf(stderr,"Error: Could not write to socket\n"); exit(1);}
         }
         else{
            charsRead = send(establishedConnection, "fek", 3, 0); // Tell the process to kill it'self then kill ourself
            if (charsRead < 0) {fprintf(stderr,"Error: Could not write to socket\n"); exit(1);}
            close(establishedConnection); // Close the existing socket which is connected to the client
            close(listenSocket); // Close the listening socket
            exit(1);
         }


         //The second packet is the number of packet we are expecting
         memset(keybuffer, '\0', packetSize);
         charsRead = recv(establishedConnection, keybuffer, 7, 0); // Read the client's message from the socket
         if (charsRead < 0) {fprintf(stderr,"Error: Could not read from socket\n"); exit(1);}
         numPackets = atoi(keybuffer);
         numPackets = numPackets;
         //printf("SERVER: I received this from the client: \"%d\"\n", numPackets);

         char cyphertext[packetSize];
         // Get the message from the client and display it
         while(numPackets > 0){// The connection is open if we get an end of transmission packet then set on = 0

            //Grab the key
            memset(keybuffer, '\0', packetSize);
            while((charsRead = recv(establishedConnection, keybuffer, truePacketSize, 0)) < truePacketSize){} // Read the client's message from the socket
            if (charsRead < 0) {fprintf(stderr,"Error: Could not read from socket\n"); exit(1);}
            if(charsRead > 254){
               //printf("SERVER: I received this from the client: \"%s\"\n%d\n\n", keybuffer,strlen(keybuffer));
            }
            else{
               printf("Server: Packet droped too small\n\n");
            }

            //Grab the cyphertext
            memset(cyphertext, '\0', packetSize);
            while((charsRead = recv(establishedConnection, cyphertext, truePacketSize, 0)) < truePacketSize){} // Read the client's message from the socket
            if (charsRead < 0) {fprintf(stderr,"Error: Could not read from socket\n"); exit(1);}
            if(charsRead > 254){
               //printf("SERVER: I received this from the client: \"%s\"\n%d\n\n", cyphertext,strlen(cyphertext));
            }
            else{
               printf("Server: Packet droped too small\n\n");
            }

            char cypher[packetSize];
            //Send back the encrypted text
            memset(cypher, '\0', packetSize);
            for(i=0;i<packetSize;i++){
               j = keybuffer[i] - 65;// Key
               k = cyphertext[i] - 65;// Cyphertext
               if(j == -33){j=26;}
               if(k == -33){k=26;}
               if(j >= 0 && k >= 0){
                  l = k-j;
                  if (l < 0) {
                     l = l + 27;
                  }
                  cypher[i] = chars[l % 27];
               }
            }
            for(j = strlen(cypher); j<truePacketSize; j++){ //fill remainder of packet
               cypher[j] = '*';
            }

            //Send a Success message back to the client
            while((charsRead = send(establishedConnection, cypher, strlen(cypher), 0)) != truePacketSize){} // Send success back
            if (charsRead < 0) {fprintf(stderr,"Error: Could not write to socket\n"); exit(1);}
            numPackets--;
         }
         close(establishedConnection); // Close the existing socket which is connected to the client
         return 0;
      }
      close(establishedConnection); // Close the existing socket which is connected to the client
   }
   close(listenSocket); // Close the listening socket
}
