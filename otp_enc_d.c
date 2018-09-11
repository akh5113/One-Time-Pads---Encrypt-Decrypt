/*******************************************************************************************************************
 * Anne Harris (harranne@oregonstate.edu)
 * CS 344 - 400
 * Program 4 - OTP
 * otp_enc_d.c
 * This program acts as the server. It recieves a message from otp_enc (client). This message contains the plain
 * text message and the key. The program then seperates them out and encrypts the message and sends it back
 * to the client
 * ****************************************************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <math.h>

//function to display error
void error(const char *msg) { perror(msg); exit(1); } // Error function used for reporting issues

/* removedChildPid function
 * parameters: integer representing the pid that completed
 * 	integer represeting the array of pids
 * 	integer representing the number of elements in teh array
 * returns: void
 * Function to removed child pid's from array when it's completed */
void removeChildPid(int pidDone, int *pidArray, int numArr){
	int pos; //postion of pid to be removed
	//find position of pid
	int i = 0;
	for(i; i < numArr; i++){
		if(pidDone == pidArray[i]){
			pos = i;
		}
	}
	
	//replace elements of array from that point on with the next element
	int j = pos;
	for(j; j < numArr -1; j++){
		pidArray[j] = pidArray[j+1];
	}
}

// main driver function
int main(int argc, char *argv[])
{
	//reference of chars
	char charSet[27] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ ";	

	//initializations
	int listenSocketFD, establishedConnectionFD, portNumber, charsRead;
	socklen_t sizeOfClientInfo;
	char buffer[200000]; //large buffer to account for plaintext4
	struct sockaddr_in serverAddress, clientAddress;

	//confirm one argument, the port
	if (argc < 2) { fprintf(stderr,"USAGE: %s port\n", argv[0]); exit(1); } // Check usage & args

	// Set up the address struct for this process (the server)
	memset((char *)&serverAddress, '\0', sizeof(serverAddress)); // Clear out the address struct
	portNumber = atoi(argv[1]); // Get the port number, convert to an integer from a string
	serverAddress.sin_family = AF_INET; // Create a network-capable socket
	serverAddress.sin_port = htons(portNumber); // Store the port number
	serverAddress.sin_addr.s_addr = INADDR_ANY; // Any address is allowed for connection to this process

	// Set up the socket
	listenSocketFD = socket(AF_INET, SOCK_STREAM, 0); // Create the socket
	if (listenSocketFD < 0) error("ERROR opening socket");

	// Enable the socket to begin listening
	if (bind(listenSocketFD, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) < 0) // Connect socket to port
		error("ERROR on binding");
	listen(listenSocketFD, 5); // Flip the socket on - it can now receive up to 5 connections

	//keep track of pid's for 5 concurrent connections
	int numConnections = 0;
	int childPidArray[5];

	//accept connections while there is 5 or less
	while(numConnections < 5){
		// Accept a connection, blocking if one is not available until one connects
		sizeOfClientInfo = sizeof(clientAddress); // Get the size of the address for the client that will connect
		establishedConnectionFD = accept(listenSocketFD, (struct sockaddr *)&clientAddress, &sizeOfClientInfo); // Accept
		if (establishedConnectionFD < 0) error("ERROR on accept");
	
		//child writes back ciphered text
		pid_t spawnPid = -5;
		spawnPid = fork();

		//successful fork
		if(spawnPid == 0){
			//put pid in array
			pid_t childPid = getpid();
			//printf("in child %d\n", childPid);
			//fflush(stdout);
			childPidArray[numConnections] = childPid;
			
			// Get the message from the client
			// clear the buffer with null chars
			memset(buffer, '\0', 200000);
			//read the key
			//use MSG_WAITALL flag to make sure all is recieved
			charsRead = recv(establishedConnectionFD, buffer, sizeof(buffer), MSG_WAITALL); // Read the client's message from the socket
			if (charsRead < 0) error("ERROR reading from socket");
	
			//verify child is communicating with otp_enc by checking for e character
			if(strncmp(buffer, "e", 1) == 0){	
				//printf(	"SERVER: I received this from the client: \"%s\"\n", buffer);
				//fflush(stdout);
				
				//search for positions of symbols
				char percentSym = '%';
				char atSym = '@';
				char dollarSym = '$';
	
				//returns pointer to position of char
				char *percentPtr = strchr(buffer, percentSym);
				char *atPtr = strchr(buffer, atSym);
				char *dollarPtr = strchr(buffer, dollarSym);
				
				//set up positions
				//percent sign is where plain text begins
				int percentPos = percentPtr - buffer; //second character
				//@ is where plain text ends and key begins
				int atPos = atPtr - buffer;
				//$ is where key ends
				int dollarPos = dollarPtr - buffer;
	
				//printf("percent position = %d\nat posision = %d\ndollar posiion = %d\n", percentPos, atPos, dollarPos);
				//fflush(stdout);			
	
				//declare plain text bufer
				char plainTextIn[200000];
				memset(plainTextIn, '\0', 200000);
				//get length of plain text
				int plainTextLen = atPos - percentPos- 1;
				//copy in just the plain text portion
				strncpy(plainTextIn, buffer + percentPos + 1, plainTextLen);
				//printf("plaintextIn = %s\n", plainTextIn);
				//fflush(stdout);	

				//declare encrypted buffer
				char keyIn[200000];
				memset(keyIn, '\0', 200000);
				//get length of key
				int keyLen = dollarPos - atPos - 1;
				//copy in just the key portion
				strncpy(keyIn, buffer + atPos + 1, keyLen);
				//printf("keyIn = %s\n", keyIn);
				//fflush(stdout);	

				//declare encrypted buffer
				char encBuffer[200000];
				memset(encBuffer, '\0', 200000);
			
				//encrypt text
				//loop through each char in plain text and get it's index
				//loop get the cooresponding key char's index
				//add them together and mod by 27
				int index = 0; //used to get numeric representation from charSet array (for PLAINTEXT)
				int keyIndex = 0; //used to get numeric representation from charSet array (for KEY)
				int keyPosition = 0; //used to walk through key 
				int i = 0; //index for for loop
				for(i; i < plainTextLen; i++){
					//get character
					char tempChar = plainTextIn[i];
					//index char to position in charSet array
					char *tempPtr = strchr(charSet, tempChar);
					index = tempPtr-charSet;
					//printf("index: %d\n", index);
					//fflush(stdout);			
		
					//get key index position in charSet array
					char *temp2Ptr = strchr(charSet, keyIn[keyPosition]);
					keyIndex = temp2Ptr - charSet;
					//printf("keyIndex: %d\n", keyIndex);
					//fflush(stdout);
	
					//add plain text index to key index
					int addition = index + keyIndex;
					//mod by 27
					int cipher = addition % 27;
					//printf("cipher = %d\n", cipher);	
					//fflush(stdout);
	
					//add character at cipher position to encrypted text buffer
					//strcat(encBuffer, charSet[cipher]);		
					encBuffer[i] = charSet[cipher];
	
					//increase key position and make sure key position is less than the key length
					keyPosition++;
				}

				//add newline
				encBuffer[plainTextLen] = '\n';

				//printf("encBuffer = %s", encBuffer);
				//fflush(stdout);
	
				// Send a Success message back to the client
				charsRead = send(establishedConnectionFD, encBuffer, sizeof(encBuffer), 0);
				//check for error
				if (charsRead < 0) error("ERROR writing to socket");
				
				close(establishedConnectionFD); // Close the existing socket which is connected to the client
			}
			//first letter is not e
			else{
				//printf("in else statement in server\n");
				char badBuff[200000];
				memset(badBuff, '\0', sizeof(badBuff));
				badBuff[0] = 'x';
				send(establishedConnectionFD, badBuff, sizeof(badBuff), 0);
				//fprintf(stderr, "ERROR otp_enc_d not communicating with otp_enc");
				exit(2);
				return 2;
			}
		}

		//check for child pids completing
		int childExitMethod = -5;
		int j = 0;
		for(j; j < numConnections; j++){
			pid_t childPid_actual = waitpid(-1, &childExitMethod, WNOHANG);
			if(childPid_actual != 0){
				//remove pid from array
				removeChildPid(childPid_actual, childPidArray, numConnections);
				numConnections--; //decrease number of connections
			}
		}
	}

	close(listenSocketFD); // Close the listening socket
	
	return 0; 
}
