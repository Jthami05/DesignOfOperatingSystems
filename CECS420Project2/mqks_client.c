#define TRACE  //{ printf("EXECUTING LINE %d \n", __LINE__); }
#define MAXDIRPATH 1024
#define MAXKEYWORD 256
#define MAXLINESIZE 1024
#define MAXOUTSIZE 2048

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <dirent.h>
#include <pthread.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>

typedef struct message {
	long mtype;
	char path[MAXDIRPATH];
	char keyword[MAXKEYWORD];
} message;


#define MESSAGESIZE (MAXDIRPATH + MAXKEYWORD)//(sizeof(message) - sizeof(long))

int numOfLines(char* file) {
	int num = 0;
	int ch;
	
	FILE* inputFile = fopen(file, "r"); 
	if (inputFile == NULL) perror ("Error opening file\n");
	while(!feof(inputFile))
	{
	  ch = fgetc(inputFile);
	  if(ch == '\n')
	  {
		num++;
	  }
	}
	
	fclose(inputFile);
	return num;
}

// don't need to create a message queue in client program
void readFile(char* file) {
	message msg;
	int message_queue_id;
	key_t key;
	int numLines = numOfLines(file);
	int i;
	
	if ((key = ftok("mqks_server.c", 1)) == -1) {
		perror("ftok in client");
		exit(1);
	}
	if ((message_queue_id = msgget(key, 0644)) == -1) {
		perror("msgget in client");
		exit(1);
	}
	
	/******************************************************
				Read file in.
	******************************************************/
	
	FILE* inputFile = fopen(file, "r");
	if (inputFile == NULL) perror ("Error opening file\n");
	else {
		for (i = 0; i < numLines; i++) {
			
			if (feof(inputFile))
				break;
				
			fscanf(inputFile, "%s", msg.path);
			
			if (strlen(msg.path) > MAXDIRPATH)
				perror("Filepath exceeds max directory path.\n");
			
			fscanf(inputFile, "%s", msg.keyword);
			if (strlen(msg.keyword) > MAXKEYWORD)
				perror("Keyword exceeds max keyword length.\n");
				
			msg.mtype = 1;
			
			//printf("path is %s and keyword is %s and type is %ld\n", msg.path, msg.keyword, msg.mtype);
			
		//	message test_msg = {1, "Hello world", "home"};
			if(msgsnd(message_queue_id, &msg, MESSAGESIZE, 0) == -1) {
				perror("msgsnd");
			}
		}
		fclose(inputFile);
	}
}

int main(int argc, char** argv) {
	char* file = argv[1];
		
	readFile(file);
	
	return 0;
}
// msgget: no such file or directory
//	
