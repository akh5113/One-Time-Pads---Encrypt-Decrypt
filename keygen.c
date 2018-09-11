/****************************************************************************
 * Anne Harris (harranne@oregonstate.edu)
 * CS 344 - 400 
 * Program 4 - OTP
 * keygen.c 
 * Description: This program creates a key file of specified length for up
 * to 27 characters. This will be used for encryption and decryption within 
 * the overall OTP programs
 * *************************************************************************/

#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<string.h>
#include<time.h>

int main(int argc, char *argv[]){

	//get key length from command line
	//printf("argc: %d\n", argc);
	int keyLen = atoi(argv[1]);
	//printf("keylen: %d\n", keyLen);

	//string to store key generated
	//add one new line character
	char myKey[keyLen+1];
	//myKey = calloc(keyLen+1, sizeof(char));

	//set myKey to be full of null terminators
	memset(myKey, '\0', keyLen+1);

	//set time for random number generator
	srand(time(NULL));

	//string to hold possible characters
	//26 upper case letters possible plus the space
	char charSet[28] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ ";

	//for loop to pick numbers to generate a number 0 to 26
	//26 upper case letters possible plus space
	int r; 	//random number generator 
	int i = 0;
	for(i; i < keyLen; i++){
		r = rand() % (26 + 1 - 0);
		//printf("r : %d\n", r);
		//printf("charSet[r] = %c\n", charSet[r]);	
		myKey[i] = charSet[r];
		//printf("mykey: %s\n", myKey);
	}
	//add in newline
	myKey[keyLen] = '\n';

	fflush(stdout);
	fprintf(stdout, myKey);

	return 0;
}
