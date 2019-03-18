//##################\\
//##ZACHARY HORINE##\\
//##CS344---BLOCK4##\\
//##WINTER03172019##\\
//##################\\
//otp_dec.c

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

int testChar(int c);

int main(int argc, char *argv[]) {
   int curchar;
   long plaintextSize = -1; //plaintext length
   long keySize = -1; //key lenth

   if (argc != 4) {fprintf(stderr,"Usage: otp_dec [plaintext] [key] [port]\n"); exit(1);}
   FILE *plaintextFile = fopen(argv[1],"rb");
   if (plaintextFile == NULL) {fprintf(stderr, "the file \"%s\" is unavailable\n", argv[1]); exit(1);}
   fseek(plaintextFile,0,SEEK_END);
   plaintextSize = ftell(plaintextFile)+1;
   fclose(plaintextFile);
   // Verify that we have good text in files
   plaintextFile = fopen(argv[1],"r"); //check for invalid characters
   while((curchar = fgetc(plaintextFile)) != EOF){
      if(!testChar(curchar)){
         fprintf(stderr,"Error: invalid chars in plaintext\n");
         exit(1);
      }
   }
   fclose(plaintextFile); //reset plaintext file pointer
   plaintextFile = fopen(argv[1],"r");

   FILE *keyFile = fopen(argv[2],"rb");
   if (keyFile == NULL) {fprintf(stderr, "the file \"%s\" is unavailable\n", argv[2]); exit(1);}
   fseek(keyFile,0,SEEK_END);
   keySize = ftell(keyFile)+1;
   fclose(keyFile);
   keyFile = fopen(argv[2],"r"); //check for invalid characters
   while((curchar = fgetc(keyFile)) != EOF){
      if(!testChar(curchar)){
         fprintf(stderr,"Error: invalid chars in key\n");
         exit(1);
      }
   }
   fclose(keyFile); //reset key file pointer
   keyFile = fopen(argv[2],"r");

   //set port
   int port = atoi(argv[3]);
   if(port <= 0 || port > 65535) {fprintf(stderr, "the port you entered \"%s\" is invalid. (Range: 1-65535)\n", argv[3]); exit(1);}


   int i, j;
   int charsWritten;
   int charsRead;
   struct sockaddr_in serverAddress;
   struct hostent* serverHostInfo;
   int packetSize = 512;
   int truePacketSize = 511;
   char buffer[packetSize];
   char *local = "localhost";

   int numSegments = plaintextSize/truePacketSize + 1;// calculate number of packets to create
   memset(buffer, '\0', sizeof(buffer)); // Clear buffer

   if(plaintextSize > keySize){// Check to make sure that the key is not shorter than the plaintext
      fprintf(stderr,"Error: key must be longer than the message\n");
      exit(1);
   }

   for(i=0;i<sizeof(buffer);i++){
      buffer[i] = 0;
   }


   // Set up the server address struct
   memset((char*)&serverAddress, '\0', sizeof(serverAddress));
   serverAddress.sin_family = AF_INET; // create socket
   serverAddress.sin_port = htons(port); // set the port
   serverHostInfo = gethostbyname(local); // set host name
   if (serverHostInfo == NULL) { fprintf(stderr, "CLIENT: ERROR, no such host\n"); exit(0); }
   memcpy((char*)&serverAddress.sin_addr.s_addr, (char*)serverHostInfo->h_addr, serverHostInfo->h_length); // set the address

   // Set up the socket
   int socketFD = socket(AF_INET, SOCK_STREAM, 0);
   if (socketFD < 0) {fprintf(stderr, "Error: could not open socket"); exit(1);}

   // Connect to server
   if (connect(socketFD, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) < 0) // Connect socket to address
      {fprintf(stderr, "Error: could not connect to socket"); exit(1);}

   strcpy(buffer,"otp_dec"); //send id
   charsWritten = send(socketFD, buffer, strlen(buffer), 0); // Write to the server who we are
   if (charsWritten < 0) {fprintf(stderr, "Error: could not write to socket"); exit(1);}
   if (charsWritten < strlen(buffer)) printf("CLIENT: WARNING: Not all data written to socket!\n");
   memset(buffer, '\0', sizeof(buffer));
   charsRead = recv(socketFD, buffer, 3, 0); // Wait for ack
   if (charsRead < 0) {fprintf(stderr, "Error: could not read from socket"); exit(1);}
   if (strcmp(buffer,"ack")){fprintf(stderr,"Error bad port");exit(1);}// no return ack

   sprintf(buffer,"%d",numSegments);
   charsWritten = send(socketFD, buffer, strlen(buffer), 0); // send number of segments
   if (charsWritten < 0) {fprintf(stderr, "Error: could not write to socket"); exit(1);}
   if (charsWritten < strlen(buffer)) printf("CLIENT: WARNING: Not all data written to socket!\n");

   // Send the key then plaintext
   for(i=0;i<numSegments;i++){

      //key
      memset(buffer, '\0', packetSize);
      fgets(buffer,packetSize,keyFile);
      // Fill the packet with * if it isn't a full packet
      for(j=0;j<truePacketSize;j++){
         if(buffer[j] == '\0' || buffer[j] == '\n'){buffer[j] = '*';}
      }
      //Send the packet while there are still bits to send
      while((charsWritten = send(socketFD, buffer, strlen(buffer), 0)) != truePacketSize){}
      if (charsWritten < 0) {fprintf(stderr, "Error: could not write to socket"); exit(1);}
      if (charsWritten < strlen(buffer)) printf("CLIENT: WARNING: Not all data written to socket!\n");

      //plaintext
      memset(buffer, '\0', packetSize);
      fgets(buffer,packetSize,plaintextFile);
      // Fill the packet with * if it isn't a full packet
      for(j=0;j<truePacketSize;j++){
         if(buffer[j] == '\0' || buffer[j] == '\n'){buffer[j] = '*';}
      }
      //Send the packet while there are still bits to send
      while((charsWritten = send(socketFD, buffer, strlen(buffer), 0)) != truePacketSize){}
      if (charsWritten < 0) {fprintf(stderr, "Error: could not write to socket"); exit(1);}
      if (charsWritten < strlen(buffer)) printf("CLIENT: WARNING: Not all data written to socket!\n");


      memset(buffer, '\0', sizeof(buffer));
      while((charsRead = recv(socketFD, buffer, truePacketSize, 0) < truePacketSize)){} // Read data, leave \0
      if (charsRead < 0) {fprintf(stderr, "Error: could not read from socket"); exit(1);}
      for(j=0;j<strlen(buffer);j++){
         if(buffer[j] != '*' && testChar(buffer[j])){
            printf("%c", buffer[j]);
         }
         else{
            break;
         }
      }
   }
   printf("\n");
   return 0;
}

int testChar(int c){
   int i = 0;
   char chars[28] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ ";
   for(i=0;i<27;i++){
      if(c == chars[i]){// It's a valid char
         return 1;
      }
   }
   if(c == 88 || c == 10){//end of file chars
      return 1;
   }
   return 0;
}
