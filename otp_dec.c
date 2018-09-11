/*********************************************************************************************************************
 * Anne Harris (harranne@oregonstate.edu)
 * CS 344 - 400
 * Program 4 - otp
 * otp_dec.c
 * Description: This file is the client. It connects to otp_dec_d and asks the server to decrypt the text it sends.
 * This program reads in two files from the command line, the first is an encrypted text file and the second is a key
 * file. It then concatinates them and send them to the server. The server decrypts the text and this program 
 * recieves the decrypted text and prints it to stdout
 * ******************************************************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 

//error messages
void error(const char *msg, int errorNum) { perror(msg); exit(errorNum); } // Error function used for reporting issues

//main diriver function
int main(int argc, char *argv[])
{
	//declare variables
	int socketFD, portNumber, charsWritten, charsRead;
	struct sockaddr_in serverAddress;
	struct hostent* serverHostInfo;
	char buffer[200000];	//large buffer
	
	//verify 3 arguments
	//encrypted text file, keyfile, port
	if (argc < 4) { fprintf(stderr,"USAGE: %s hostname port\n", argv[3]); exit(0); } // Check usage & args
	
	// Get input message from useir
	//printf("argv[1]: %s\n", argv[1]);
	//printf("argv[2]: %s\n", argv[2]);
	
	//fill buffer will null terminators
	memset(buffer, '\0', sizeof(buffer));

	//add decryption char "d"
	buffer[0] = 'd';
	//add percent sign to indicate beginning of enctrypted text
	buffer[1] = '%';

	//get length of enc text
	int encLen = 0;
	//open encrypted text to read
	FILE * encF = fopen(argv[1], "r");
	
	//check to make sure file opened
	if(encF == NULL){
		error("ERROR no such file", 0);
		exit(1);
	}
	//set the index to the third spot (index 2)
	int t=2;
	int x; 
	while((x = fgetc(encF)) != '\n'){
		//printf("x: %d - %c\n", x, (char)x);

		//test for bad char
		//if the characters ascii key is less than 65 or greater than 91 and is NOT a space
		if((x < 65 || x > 91) && (x != 32)){
			fprintf(stderr, "otp_dec error: input contains bad characters");
			exit(1);
		}

		//add char to buffer
		buffer[t++] = (char)x;	//add to buffer and incrase
		encLen++;		//length of encrypted message
	}

	//add in seperator from encryption to key
	buffer[t++] = '@';

	//get length of key
	int keyLen = 0;
	//open key
	FILE *keyF = fopen(argv[2], "r");

	//check to make sure file opened
	if(keyF == NULL){
		error("ERROR no such file", 0);
		exit(1);
		return 1;
	}

	//read in file and concatanate to bufer
	while((x = fgetc(keyF)) != '\n'){
		buffer[t++] =(char)x;	//add to buffer and increase
		keyLen++;		//length of the key
	}

	//check to make sure key length is at least as long as encrypted text
	if(encLen > keyLen){
		//error("ERROR key is too short", 1);
		fprintf(stderr, "ERROR: key '%s' is too short\n",argv[2] ); 
		exit(1);
		return 1;
	}

	//add in end of file char '$'
	buffer[t++] = '$';

	//add newline
	buffer[t] = '\n';
    
	// Set up the server address struct
	memset((char*)&serverAddress, '\0', sizeof(serverAddress)); // Clear out the address struct
	portNumber = atoi(argv[3]); // Get the port number, convert to an integer from a string
	serverAddress.sin_family = AF_INET; // Create a network-capable socket
	serverAddress.sin_port = htons(portNumber); // Store the port number
	serverHostInfo = gethostbyname("localhost"); //using localhoast
	if (serverHostInfo == NULL) { fprintf(stderr, "CLIENT: ERROR, no such host\n"); exit(0); }
	memcpy((char*)&serverAddress.sin_addr.s_addr, (char*)serverHostInfo->h_addr, serverHostInfo->h_length); // Copy in the address
	
	// Set up the socket
	socketFD = socket(AF_INET, SOCK_STREAM, 0); // Create the socket
	if (socketFD < 0) error("CLIENT: ERROR opening socket", 0);
	
	// Connect to server
	if (connect(socketFD, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) < 0) // Connect socket to address
		error("CLIENT: ERROR connecting", 0);

	//print the buffer to confirm
	//printf("buffer: %s", buffer);
	//fflush(stdout);

	// Send message to server
	charsWritten = send(socketFD, buffer, sizeof(buffer), 0); // Write to the server
	//check for errors
	if (charsWritten < 0) error("CLIENT: ERROR writing to socket", 1); //if no chars are written
	if (charsWritten < strlen(buffer)) printf("CLIENT: WARNING: Not all data written to socket!\n"); //if notall chars are written

	// Get return message from server
	memset(buffer, '\0', sizeof(buffer)); // Clear out the buffer again for reuse
	//receve the data from the socket
	//since we know the size of the buffer coming in, we can wait to read it all with the flag MSG_WAITALL
	charsRead = recv(socketFD, buffer, sizeof(buffer), MSG_WAITALL); // Read data from the socket
	if (charsRead < 0) error("CLIENT: ERROR reading from socket", 1);
	
	//printf("CLIENT: I received this from the server: \"%s\"\n", buffer);
	
	if(strncmp(buffer, "x", 1) == 0){
	//	printf("in strncmp\n");
		//print to stderr
		fprintf(stderr, "error: otp_dec cannot use otp_enc_d\n");
		exit(2);
		return 2;
	}

	//printf("CLIENT: I received this from the server: \"%s\"\n", buffer);
	//fflush(stdout);
	
	//print to stdout
	fprintf(stdout, buffer);

	close(socketFD); // Close the socket
	return 0;
}
