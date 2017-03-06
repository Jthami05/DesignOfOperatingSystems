#define TRACE  //{ printf("EXECUTING LINE %d \n", __LINE__); }
#define MAXDIRPATH 1024
#define MAXKEYWORD 256
#define MAXLINESIZE 1024
#define MAXOUTSIZE 2048

	/*************************************************************************
	Check if buffer is full before allowing worker thread to access buffer.
	**************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <dirent.h>
#include <stdbool.h>
#include <errno.h>
#include <pthread.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <fcntl.h>
#include <semaphore.h>

// we use file locking for the output file, since multiple threads will be accessing the same output file.

typedef struct message {
	long mtype;
	char path[MAXDIRPATH];
	char keyword[MAXKEYWORD];
} message;

typedef struct item {
	char* fileName;
	char* line;
	int lineNumber;
	struct item* next;
} item;

typedef struct threadData {
	char* fileName;
	char keyword[MAXKEYWORD];
	char path[MAXDIRPATH];
	item** buffer;
} threadData;

	int max;
	int current;
	int use;
	int numOfElements;
	bool flag = false;
	sem_t empty;
	sem_t full;
	sem_t mutex;

	//struct item* buffer[];

int max;
int bufferSize;

#define MESSAGESIZE (MAXDIRPATH + MAXKEYWORD) //(sizeof(message) - sizeof(long))

item* retrieve(item** buffer) {

	item* this;
	this = buffer[use];
	use++;
	numOfElements--;
	
	if (use == max)
		use = 0;
	
	return this;
}

void* printer(void* params) {
	printf("\n//// %lu: within printer thread.\n", pthread_self());
	threadData* pass = params;
	item** buffer = pass->buffer;
	struct flock fl = {F_WRLCK, SEEK_END, 0, 0, 0};
	int fd;
	
	sem_wait(&full);
    sem_wait(&mutex);//---------------------------------------------------------------------------------
	if ((fd = open("output.txt", O_CREAT | O_APPEND | O_RDWR, 0666)) == -1) {
		perror("open");
		exit(-1);
	}
	
	// set file as locked, unless it is already locked, in which case wait to lock it for yourself
	fl.l_pid = getpid();
	if (fcntl(fd, F_SETLKW, &fl) == -1) {
		perror("fcntl");
		exit(-1);
	}
	
	while (numOfElements != 0) {
			
		printf("%lu: before retrieving from buffer, there are %d elements\n", pthread_self(), numOfElements);
		item* printItem = retrieve(buffer);
	
		if (printItem->line == NULL)
			exit(1);
			
		char *output = malloc(1024);
		printf("~~~~~~~~~~~~~~\n%lu: printing %s:%d:%s to output file\n~~~~~~~~~~~~~~\n", pthread_self(), printItem->fileName, printItem->lineNumber, printItem->line);
	
		if (sprintf(output, "%s:%d:%s", printItem->fileName, printItem->lineNumber, printItem->line) < 0) {
			perror("snprintf");
			exit(1);
		}
	
		if ( (write(fd, output, strlen(output)) ) == -1) {
			perror("write");
			exit(1);
		}
		
		printf("%lu: after retrieving from buffer, there are %d elements\n", pthread_self(), numOfElements);
		//free(printItem->fileName); // if I free this here, the worker thread can't access it
		//free(printItem->line); // if I free this here, the worker thread can't access it
		//free(printItem); // if I free this here, the worker thread can't access it
		free(output);
		
	}
//	printf("////printed to output file.\n");
    
    sem_post(&mutex);//---------------------------------------------------------------------------------
    sem_post(&empty);
	
	pthread_exit(0);
}

int numFiles(char* path) {
	int count = 0;
	DIR* dirp;
	struct dirent* entry;
	
	if ((dirp = opendir(path)) == NULL) {
		perror("numFiles: opendir");
        exit(1);
    }
	// while there are still files left
	while ((entry = readdir(dirp)) != NULL) {
		if (entry->d_type == DT_REG) {
			if (strstr(entry->d_name, "~") == NULL) {
				count++;
			}
		}
	}
	closedir(dirp);
	return count;
}

// sets files[] equal to a list of the filenames in the <path> directory.
void getNames(char* path, int count, char* files[]) {
	DIR* dirp;
	struct dirent* entry;
	int i = 0;
	dirp = opendir(path);
	
	while ((entry = readdir(dirp)) != NULL) {
		if (entry->d_type == DT_REG) {
			if (strstr(entry->d_name, "~") == NULL) {
				files[i] = strdup(entry->d_name);
				i++;
			}	
		}
	} 
	closedir(dirp);
}

void addToBuffer(item* thisItem, item** buffer) {
	
	buffer[current] = thisItem;
	//buffer[current]->line = thisItem->line;
//	printf("%d: line %s added\n", numOfElements, buffer[current]->line);
	
	if (current != 0)
		printf("%lu: current(%d): prev line was %s\n", pthread_self(), current, buffer[current-1]->line);
	
	current++;
	numOfElements++;
	printf("%lu: after adding to buffer, there are %d elements\n", pthread_self(), numOfElements);
	
	if (current == max)
		current = 0;
}

void* searchFile(void* param) {
	threadData* data = param;
	item** buffer = data->buffer;
	item* newItem;
	FILE* fp;
	int line_num = 0;
	int find_result = 0;
	char temp[MAXLINESIZE];
	char* token;
	char* token2;
	char* line_save;
	char* line_save2;
	char* line_delim = "\n\r";
	char* line_delim2 = " ";
	
	data->fileName = strcat(data->path, data->fileName);
	
	printf("fileName is %s\n", data->fileName);
	
	if ((fp = fopen(data->fileName, "r")) == NULL) {
		perror("searchFile fopen");
		exit(1);
	}
	while(fgets(temp, MAXLINESIZE, fp) != NULL) {
		// copy temp (this line) into linecopy
		char linecopy[sizeof(temp)];
		strcpy(linecopy, temp);
		line_num++;
		
		token = strtok_r(temp, line_delim, &line_save);
		while (token) {
	
			token2 = strtok_r(token, line_delim2, &line_save2);
			while (token2) {
			
			if (strcmp(token2, data->keyword) == 0) {
				// size of temp is 1024
				newItem = malloc(sizeof(item));
				newItem->lineNumber = line_num;
				newItem->fileName = data->fileName;
				newItem->line = strdup(linecopy);
				/*
					Need to allocate for newItem->line. Using strdup() will alloc automatically (free it later) or you can
					use malloc() ( probably passing it sizeof(temp) )
				*/
//				printf("-----------------FOUND MATCH-------------\n%s:%d:%s\n---------------\n", newItem->fileName, newItem->lineNumber, newItem->line);
				//printf("about to call (%d) addToBuffer() and add '%s' because: %s matches %s\n", line_num, newItem->line, token2, data->keyword);
			
				sem_wait(&empty);
				sem_wait(&mutex);
				printf("in semaphore in searchFile()\n");
				addToBuffer(newItem, buffer);
				sem_post(&mutex);
				sem_post(&full);
			
				find_result++;
				break;
				}
				token2 = strtok_r(NULL, line_delim2, &line_save2);
			}
			token = strtok_r(NULL, line_delim, &line_save);
		}
	}
	
	if (find_result == 0) {
		printf("No matches found!\n");
		flag = false;
	}
		
	if (fp) {
		fclose(fp);
	}
	
	TRACE;
	
	//free(temp); // necessary? probably not since temp is a char[]
	pthread_exit(0);
}

// server creates N child processes, 1 each for every file in the directory. Each process will search for the keyword in the file.
int main(int argc, char** argv) {
	message msg;
	int message_queue_id;
	key_t key;
	int fileNum = 0;
	pid_t process;
	int i;
	int numberOfForks = 0;
	
	current = 0;
	use = 0;
	numOfElements = 0;
	
	bufferSize = atoi(argv[1]);
	
	sem_init(&empty, 0, bufferSize); // max are empty 
  	sem_init(&full, 0, 0);    // 0 are full
  	sem_init(&mutex, 0, 1);   // mutex
	
	item** buffer = malloc(sizeof(item)*bufferSize);
	
	// invoke using ./mqks_server <buffer size>	
	max = 10; // change this to argv[1] for buffer size
		
	if ((key = ftok("mqks_server.c", 1)) == -1) {
		perror("ftok in server");
		exit(1);
	}

	if ((message_queue_id = msgget(key, 0644 | IPC_CREAT)) == -1) {
		perror("msgget in server");
		exit(1);
	}
	
	if (msgrcv(message_queue_id, &msg, MESSAGESIZE, 0, 0) == -1) {
		perror("msgrcv in server");
		exit(1);
	}
	//printf("%s is keyword and %s is path\n", msg.keyword, msg.path);
	
	/**************************************
			Setup array of file names.
	**************************************/
	fileNum = numFiles(msg.path);
	//printf("fileNum is %d\n", fileNum);
	char* files[fileNum];
	getNames(msg.path, fileNum, files);
	// files[i] is a list of filenames
	
	/**************************************
			Child process creation.
	**************************************/
	process = fork();
	numberOfForks++;
	
	// additional thread is the printer thread
	pthread_t tid[fileNum];
	pthread_attr_t attr;
	pthread_attr_init(&attr);
		
	if (process < 0) 
		perror("Error in child creation");
	else if (process == 0) {
		//printf("I am the child process with pid = %d, n = %d\n", getpid(), process);
		
	/*******************************************
		Create worker threads and structs.
	*******************************************/
	
		// thread for each file in the requested directory.
		threadData pass[fileNum];
		
		for (i = 0; i < fileNum; i++) {
																
			strcpy(pass[i].path, msg.path);
			strcpy(pass[i].keyword, msg.keyword);
			
			pass[i].buffer = buffer;
			
			char* currentFile = strdup(files[i]);
			pass[i].fileName = strdup(currentFile);
			
			// worker
			if (pthread_create(&tid[i], &attr, searchFile, (void*)&pass[i]) != 0) {
				perror("Thread creation");
			}
			
			TRACE;
			
			free(currentFile);
			//free(pass[i].fileName);
		}// for
		
		pass[fileNum].buffer = buffer;
			
		/**************************************
					Join threads.
		**************************************/
		
		for(i = 0; i < fileNum; i++) {
			
			// this can't be run until (fileNum - 1) b/c it could then run until i < 0 if fileNum is 1
			pthread_join(tid[i], NULL);
			//printf("joining thread %d\n", i);
		}
		
		/**************************************
				Printer thread and join.
		**************************************/
		TRACE;
		
		if (flag) {
			if (pthread_create(&tid[fileNum], NULL, printer, (void *)&pass[fileNum]) != 0)
				perror("Printer thread creation");
			
			pthread_join(tid[fileNum], NULL);
		}
		
	/**************************************
	Free memory. NOTE: Is this unnecessary 
		because of free(currentFile) ? 
	Probably b/c of error saying invalid free.
	**************************************/
		TRACE;
		for (i = 0; i < fileNum; i++) {
			free(files[i]);
		//	free(pass[i].fileName);
		}
	} // child process
	else {
	//	int status;
		//printf("I am the parent process with pid = %d, n = %d\n", getpid(), process);	
		for (i = 0; i < numberOfForks; i++) {
		//	printf("------------------------------ waiting for child\n\n");
			wait(NULL);
		//	printf("------------------------------ finished wait for child\n");
		}
		
		//while (wait(NULL) != -1 || errno != ECHILD) {
    		// nothing
		//}
		//waitpid(process, &status, 0); a possibility
	}
	
	/*for (i = 0; i < fileNum; i++) {
		free(files[i]);
	} */
	
	TRACE;
	free(buffer);
	return 0;
}

// maybe memory leaks are coming from child process since child and parent both create threads?

