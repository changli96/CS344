//##################\\
//##ZACHARY HORINE##\\
//##CS344---BLOCK4##\\
//##WINTER03172019##\\
//##################\\
//otp_enc_d.c

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

int main(int argc, char *argv[]) {
   if (argc != 2) {fprintf(stderr,"Usage: otp_enc_d [port]\n"); exit(1);}

   char chars[28] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ ";
   int pid;
   int listenSocketFD, establishedConnectionFD, portNumber, charsRead,numPackets,i,j,k;
   int packetSize = 512;
   int truePacketSize = 511;
   socklen_t sizeOfClientInfo;
   char keybuffer[packetSize];// For storeing key packets
   char plaintext[packetSize];// For storeing plaintext packets
   char cypher[packetSize];
   struct sockaddr_in serverAddress, clientAddress;
   memset((char *)&serverAddress, '\0', sizeof(serverAddress)); // Clear out the address struct
   portNumber = atoi(argv[1]); // Get the port number, convert to an integer from a string
   serverAddress.sin_family = AF_INET; // Create a network-capable socket
   serverAddress.sin_port = htons(portNumber); // Store the port number
   serverAddress.sin_addr.s_addr = INADDR_ANY; // Any address is allowed for connection to this process

   // Set up socket
   if (listenSocketFD < 0) {fprintf(stderr,"Error: Could not open socket\n"); exit(1);}
   listenSocketFD = socket(AF_INET, SOCK_STREAM, 0); // Create the socket

   // Enable socket and start listening
   if (bind(listenSocketFD, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) < 0) {// Connect socket to port
      fprintf(stderr,"Error: Could not bind socket to port %d\n", portNumber);
      exit(1);
   }
   listen(listenSocketFD, 5); // Flip the socket on - it can now receive up to 5 connections

   while(1){
      // Accept a connection, blocking if one is not available until one connects
      sizeOfClientInfo = sizeof(clientAddress); // Get the size of the address for the client that will connect
      establishedConnectionFD = accept(listenSocketFD, (struct sockaddr *)&clientAddress, &sizeOfClientInfo); // Accept
      if (establishedConnectionFD < 0) {
         fprintf(stderr,"Error: Could not accept connection\n");
         exit(1);
      }

      pid = fork();
      if(pid == -1){// Failed to create child
         exit(1);
      }
      else if(pid == 0){// Child process
         //Preform our two checks
         //The first packet we get should be "otp_enc"
         memset(keybuffer, '\0', packetSize);
         charsRead = recv(establishedConnectionFD, keybuffer, 7, 0); // Read the client's message from the socket
         if (charsRead < 0) {fprintf(stderr,"Error: Could not read from socket\n"); exit(1);}
         //printf("SERVER: I received this from the client: \"%s\"\n", keybuffer);
         if(!strcmp(keybuffer,"otp_enc")){
            charsRead = send(establishedConnectionFD, "ack", 3, 0); // Send success back
            if (charsRead < 0) {fprintf(stderr,"Error: Could not write to socket\n"); exit(1);}
         }
         else{
            charsRead = send(establishedConnectionFD, "fek", 3, 0); // Tell the process to kill it'self then kill ourself
            if (charsRead < 0) {fprintf(stderr,"Error: Could not write to socket\n"); exit(1);}
            close(establishedConnectionFD); // Close the existing socket which is connected to the client
            close(listenSocketFD); // Close the listening socket
            exit(1);
         }


         //The second packet is the number of packet we are expecting
         memset(keybuffer, '\0', packetSize);
         charsRead = recv(establishedConnectionFD, keybuffer, 7, 0); // Read the client's message from the socket
         if (charsRead < 0) {fprintf(stderr,"Error: Could not read from socket\n"); exit(1);}
         numPackets = atoi(keybuffer);
         numPackets = numPackets;
         //printf("SERVER: I received this from the client: \"%d\"\n", numPackets);

         // Get the message from the client and display it
         while(numPackets > 0){// The connection is open if we get an end of transmission packet then set on = 0

            //Grab the key
            memset(keybuffer, '\0', packetSize);
            while((charsRead = recv(establishedConnectionFD, keybuffer, truePacketSize, 0)) < truePacketSize){} // Read the client's message from the socket
            if (charsRead < 0) {fprintf(stderr,"Error: Could not read from socket\n"); exit(1);}
            if(charsRead > 254){
               //printf("SERVER: I received this from the client: \"%s\"\n%d\n\n", keybuffer,strlen(keybuffer));
            }
            else{
               printf("Server: Packet droped too small\n\n");
            }

            //Grab the plaintext
            memset(plaintext, '\0', packetSize);
            while((charsRead = recv(establishedConnectionFD, plaintext, truePacketSize, 0)) < truePacketSize){} // Read the client's message from the socket
            if (charsRead < 0) {fprintf(stderr,"Error: Could not read from socket\n"); exit(1);}
            if(charsRead > 254){
               //printf("SERVER: I received this from the client: \"%s\"\n%d\n\n", plaintext,strlen(plaintext));
            }
            else{
               printf("Server: Packet droped too small\n\n");
            }

            //Send back the encrypted text
            memset(cypher, '\0', packetSize);
            for(i=0;i<packetSize;i++){
               if(plaintext[i] == '*'){ //end of last packet filled with '*' chars.
                  break;
               }
               j = keybuffer[i] - 65;// Key
               k = plaintext[i] - 65;// Plaintext
               if(j == -33){j=26;}// For " "
               if(k == -33){k=26;}
               if(j >= 0 && k >= 0){
                  cypher[i] = chars[(j + k) % 27];
               }
            }
            if(numPackets == 1){
               for(j=0;j<truePacketSize;j++){
                  if(cypher[j] == '\0' || cypher[j] == '\n'){
                     cypher[j] = '*';
                  }
               }
            }

            //Send a Success message back to the client
            while((charsRead = send(establishedConnectionFD, cypher, strlen(cypher), 0)) != truePacketSize){} // Send success back
            if (charsRead < 0) {fprintf(stderr,"Error: Could not write to socket\n"); exit(1);}
            numPackets--;
         }
         close(establishedConnectionFD); // Close the existing socket which is connected to the client
         return 0;
      }
      close(establishedConnectionFD); // Close the existing socket which is connected to the client
   }
   close(listenSocketFD); // Close the listening socket
}
