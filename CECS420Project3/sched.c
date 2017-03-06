#define TRACE // { printf("EXECUTING LINE %d \n", __LINE__); }

#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
// Tyler Hamilton sched.c

float turnaround;
float waitTime;

typedef struct Process {
	int id;
	int wait;
	int finish;
	int arrival;
	int burst;
	int roundRobinBurst;					// rename to something else
	bool complete;
	struct Process* next;
} Process;

typedef struct List {
	struct Process* first;
	struct Process* this; 						// rename to 'this'
} List;

void writeFile(char* fileName, List* list);
void List_show(List* list);
void duplicatePush(List* list, Process* process);
void getLast(List* list);
bool isDone(List* list);

void List_free(List* list) {
	Process *temp = NULL;
		
	while(list->first != NULL) {
		temp = list->first;
		list->first = list->first->next;
		free(temp);
	}
}

void swap(List* queue) {
	queue->this->next = queue->first;
	queue->this = queue->first;
	queue->first = queue->first->next;
	queue->this->next = NULL;
}

// insert a process into the list
void insert(List *list, int tempid, int tempArrival, int tempBurst)
{
	Process *temp = malloc(sizeof(Process));
	temp->wait = 0;
	temp->finish = 0;
	temp->id = tempid;
	temp->arrival = tempArrival;
	temp->burst = tempBurst;
	temp->roundRobinBurst = tempBurst;
	temp->complete = false;
	temp->next = NULL;
	
	if (list == NULL)
		printf("list is NULL in insert.\n");
		
	if(list->first == NULL)
	{
		list->first = temp;
		list->this = list->first;
	}
	else
	{
		list->this->next = temp;
		list->this = list->this->next;
	}
}

void setWait(Process* working) {
	working->wait = working->finish - working->arrival;
	working->wait -= working->burst;
}

void duplicateList(List* old, List* new)
{
	new->first = NULL;
	new->this = NULL;
	if(old->first != NULL)
	{
		old->this = old->first;
		while(old->this != NULL)
		{
			duplicatePush(new, old->this);
			old->this = old->this->next;
		}
	}
}

// copies the process (process) and pushes it to the list
void duplicatePush(List* list, Process* process) {
	Process* temp = malloc(sizeof(Process));
	temp->id = process->id;
	temp->wait = process->wait;
	temp->finish = process->finish;
	temp->complete = process->complete;
	temp->arrival = process->arrival;
	temp->burst = process->burst;
	temp->roundRobinBurst = process->roundRobinBurst;
	temp->next = NULL;
	
	// push to end of list, or make it the beginning
	
	if(list->first == NULL)
	{
		list->first = temp;
		list->this = list->first;
	}
	else
	{
		list->this->next = temp;
		list->this = list->this->next;
	}
}

void getLast(List* list) {
	if (list->first == NULL) {
		return;
	}
	list->this = list->first;
	while (list->this->next != NULL) {
		list->this = list->this->next;
	}
}

void roundRobin(List* list, int quantum) {
	int time = 0;
	int wQuantum = 0;
	int currentBurst = 0;
	List* ready = malloc(sizeof(List));
	List* done = malloc(sizeof(List));
	Process *working, *temp;
	
	ready->first = NULL;
	ready->this = NULL;
	done->first = NULL;
	done->this = NULL;
	// initializing and set first node
	insert(ready, list->first->id, list->first->arrival, list->first->burst);
	working = ready->first;
	list->this = list->first;
	currentBurst = working->roundRobinBurst;
	
	while(list->this->next->next != NULL) {			
		time++;
		// moving through time
		if(currentBurst > 0) {
			currentBurst--;
		}
		// if quantum has been reached
		if(wQuantum == quantum) {
			if(working != NULL) {
				working->roundRobinBurst = currentBurst;
			
				if(currentBurst == 0) {
					// if we've reached the end of execution for this process
					working->finish = time;
					working->complete = true;
					// make one function, called six times
					
					setWait(working);
					duplicatePush(done, working);
					
					if(ready->first->next != NULL) {
						temp = ready->first;
						ready->first = ready->first->next;
						free(temp);
						working = ready->first;
						currentBurst = working->roundRobinBurst;
					}
					else {
						free(ready->first);
						ready->first = NULL;
						ready->this = NULL;
						working = NULL;
					}
					// if next process has arrived and quantum time is up IMPORTANT -----------------------------------------------
					if(time == list->this->next->arrival) {
						getLast(ready);	// could make all of this a function
						insert(ready, list->this->next->id, list->this->next->arrival, list->this->next->burst);
						list->this = list->this->next;
					}
				}
				else {
					// if the next process has arrived and we continue execution
					if(time == list->this->next->arrival) {
						if(ready->first->next == NULL) {
							insert(ready, list->this->next->id, list->this->next->arrival, list->this->next->burst);
							getLast(ready);
							swap(ready);
						}
						else { // swap and insert
							getLast(ready);
							swap(ready);
							insert(ready, list->this->next->id, list->this->next->arrival, list->this->next->burst);
							list->this = list->this->next;
						}
						
						working = ready->first;
						currentBurst = working->roundRobinBurst;
					}
					else {
						// delete head node from ready queue
						if(ready->first->next == NULL) {
							swap(ready);
						}
						else {
							getLast(ready);
							swap(ready);							
							working = ready->first;
							currentBurst = working->roundRobinBurst;
						}
					}
				}
			}
			else {
				if(time == list->this->next->arrival) {
					// if arrived again
					getLast(ready);
					
					insert(ready, list->this->next->id, list->this->next->arrival, list->this->next->burst);
					// iterate through
					list->this = list->this->next;
				}
			}
			wQuantum = 0;
		}
		else {
			if(working == NULL) {
					//
				if(time == list->this->next->arrival) {
					getLast(ready);
					insert(ready, list->this->next->id, list->this->next->arrival, list->this->next->burst);
					list->this = list->this->next;
					// insert and iterate, empty quantum, set attributes
					wQuantum = 0;
					working = ready->first;
					currentBurst = working->roundRobinBurst;
				}
				
			} // if working == NULL
			else {
				working->roundRobinBurst = currentBurst;
				if(currentBurst == 0) {
					working->finish = time;
					
					working->complete = true;
					setWait(working);
					duplicatePush(done, working);
															// printf("working->id is %d\n", working->id);
					if(ready->first->next == NULL) {
						// if ready queue is empty!
						free(ready->first);
						ready->first = NULL;
						ready->this = NULL;
						working = NULL;
					}
					else {
						temp = ready->first;
						ready->first = ready->first->next;
						free(temp);
						working = ready->first;
						currentBurst = working->roundRobinBurst;
					}
					
					if(time == list->this->next->arrival) {					// if next process has arrived, push
						getLast(ready);
						insert(ready, list->this->next->id, list->this->next->arrival, list->this->next->burst);
						list->this = list->this->next;
					}
					wQuantum = 0;
				}
				else {
					if(time == list->this->next->arrival)
					{
						getLast(ready);
						insert(ready, list->this->next->id, list->this->next->arrival, list->this->next->burst);
						list->this = list->this->next;
					}
				}
			}
		}
		wQuantum++;
	}
		while(isDone(ready)) {			
			time++;
			if(currentBurst > 0) {
				currentBurst--;
			}
		if(wQuantum == quantum) {	// right quantum
			working->roundRobinBurst = currentBurst;
			if(currentBurst == 0) {
				working->finish = time;
				working->complete = true;
				setWait(working);
				duplicatePush(done, working);
				
				if(ready->first->next == NULL) {	// if no time left and nothing else left in ready queue
					free(ready->first);
					ready->first = NULL;
					ready->this = NULL;
					working = NULL;
				}
				else {
					temp = ready->first;
					ready->first = ready->first->next;
					free(temp);
					working = ready->first;
					currentBurst = working->roundRobinBurst;
				}
				if (list->this->next == NULL) {
					// nothing
				}
				else {
					if(list->this->next->arrival == time) {
						getLast(ready);
						insert(ready, list->this->next->id, list->this->next->arrival, list->this->next->burst);
						// check these
						list->this = list->this->next->next;
					}
				}
			}
				else {
					if(list->this != NULL && time == list->this->arrival) {
						if(ready->first->next == NULL) {
							insert(ready, list->this->next->id, list->this->next->arrival, list->this->next->burst);
							//finishing things up
							getLast(ready);
							swap(ready);
						}
					else {
						getLast(ready);
						swap(ready);
						// swap function here
						insert(ready, list->this->next->id, list->this->next->arrival, list->this->next->burst);
						list->this = list->this->next;
					}
					// set new node we're working with and this node's burst
					working = ready->first;
					currentBurst = working->roundRobinBurst;
				}
					else if(ready->first->next == NULL) {
						working = ready->first;
						// fix burst
						currentBurst = working->roundRobinBurst;
					}
				else {
					getLast(ready);
					swap(ready);
					working = ready->first;
					// set working back
					currentBurst = working->roundRobinBurst;
				}
			}			
			wQuantum = 0; // reset
		}
		else {
			working->roundRobinBurst = currentBurst;
			
			if(currentBurst == 0) {
				working->finish = time;
				working->complete = true;
				setWait(working);
				duplicatePush(done, working);
				// empty the values if we've got an empty list here
				if(ready->first->next == NULL) {
					free(ready->first);
					ready->first = NULL;
					ready->this = NULL;
					working = NULL;
				}
				else {
				
				// loop through and free the queue
				temp = ready->first;
				ready->first = ready->first->next;
				free(temp);
				
				working = ready->first;
				currentBurst = working->roundRobinBurst;
				}
				wQuantum = 0;		// reset
			}
			
			if(list->this->next != NULL) {
				if(list->this->next->arrival == time) {
					getLast(ready);
					insert(ready, list->this->next->id, list->this->next->arrival, list->this->next->burst);
					list->this = list->this->next;
				}
			}
		}
			wQuantum++;
	}
	
	List_free(ready);
	free(ready);
	List_free(list);
	duplicateList(done, list);
	List_free(done);
	
	free(done);
}

// returns true if there's something left in the ready queue that needs to be executed.
bool isDone(List* list) {

	list->this = list->first;
	while(list->this != NULL) {
		if (!list->this->complete) {
			return true;
		}
		list->this = list->this->next;
	}
	return false;
}

void freeSJFAndPushToComplete(List* list, List* ready, List* complete) {
	// freeing queues
	List_free(ready);
	free(ready);
	List_free(list);
	
	duplicateList(complete, list);
	List_free(complete);
	free(complete);
}

void fcfs(List* ready, Process* temp, Process* temp2, Process* prev) {
	// first come first serve algorithm for tie cases
	while(ready->this != NULL) {				
		if (ready->this->burst < temp->burst || (ready->this->arrival < temp->arrival && ready->this->burst == temp->burst)) {
			temp = ready->this;
			prev = temp2;
		}						
		// iterate through and do this for all processes in the ready queue. temp2 is ready->this->prev
		ready->this = ready->this->next;
		temp2 = temp2->next;
	}// end while
	if (ready->first->id != temp->id) {
		prev->next = temp->next;
		temp->next = ready->first;
		ready->first = temp;
	} // close if
}

void shortestJobFirst(List* list, char* fileName) {
	int time = 0;
	int currentBurst = 0;
	// current process we're working with
	Process* working;
	Process* temp;
	Process* temp2;
	Process* prev;
	// ready queue and finished list
	List* ready = malloc(sizeof(List));
	List* complete = malloc(sizeof(List));
	
	ready->first = NULL;
	ready->this = NULL;
	complete->first = NULL;
	complete->this = NULL;
	list->this = list->first;
	// insert first element
	insert(ready, list->first->id, list->first->arrival, list->first->burst);
	// process in the ready queue that needs to execute
	working = ready->first;
	currentBurst = working->burst;
	
	// while the next two nodes aren't null, increase time count. If current node's burst > 0, decrease it. We'll iterate through this list.
	while(list->this->next->next != NULL) {
	//printf("\nopening main while loop. There are at least two nodes left after the current one.\n");
		time++;
		if(currentBurst > 0) {
			currentBurst--;
		}
		
		if(currentBurst  == 0 && working != NULL) {
			// set finish time, complete = true (this process is finished), and waiting time.
			working->finish = time;
			working->complete = true;
			setWait(working);
			
			// push this process to the complete list after having executed it.
			duplicatePush(complete, working);
			if (ready->first->next != NULL) {
				temp = ready->first;
				ready->first = ready->first->next;
				free(temp);
			} else {
				// if ready queue only has one element, free and empty it.
				free(ready->first);
				working = NULL;
				ready->first = NULL;
				ready->this = NULL;
			}
			// if we've arrived at the next process' arrival time
			if (time == list->this->next->arrival) {
				// insert it into the ready queue because it's arrived. This doesn't mean it's being executed, just that it's ready.
				getLast(ready);										// change to getLast
				insert(ready, list->this->next->id, list->this->next->arrival, list->this->next->burst);
				// move to next item in the list.
				list->this = list->this->next;
			}
			// when there are still processes in the ready queue
			if (isDone(ready)) {
			// if there's more than one process in the ready queue
				if (ready->first->next != NULL) {
					temp = ready->first;
					temp2 = ready->first;
					prev = ready->first;
					// set this = the next node so you can compare it with the previous one for FCFS.
					ready->this = ready->first->next;
					fcfs(ready, temp, temp2, prev);
				}
				// set working process = first process in the ready queue and set currentBurst = its burst.
				working = ready->first;
				currentBurst = working->burst;
			}
		}
		else {
			// insert into ready queue at end of list if next process in list has arrived
			if (time == list->this->next->arrival) {
				getLast(ready);
				insert(ready, list->this->next->id, list->this->next->arrival, list->this->next->burst);
				list->this = list->this->next;
			}
			// if working == NULL AND there are still processes left in the ready queue
			if (working == NULL && isDone(ready)) {
				if (ready->first->next != NULL) {
					temp = ready->first;
					temp2 = ready->first;
					prev = ready->first;
					ready->this = ready->first->next;
					fcfs(ready, temp, temp2, prev);
				}
				
				currentBurst = ready->first->burst;
				working = ready->first;
			}
		}
	}
	
	// while ready queue still has something in it
	while(isDone(ready)) {
		// increment time, so decrease the current process' burst time.
		time++;
		if (currentBurst > 0) {
			currentBurst--;
		}
		
		// if our process has finished executing, PUSH it to the complete list.
		if (currentBurst == 0) {
			working->finish = time;
			working->complete = true;
			setWait(working);
			duplicatePush(complete, working);
			
			// if next process in ready queue exists, free the first and move on.
			if (ready->first->next != NULL) {
				temp = ready->first;
				ready->first = ready->first->next;
				free(temp);
			} else {
				// otherwise, that was the end of the ready queue, so free it and set its values to NULL.
				free(ready->first);
				working = NULL;
				ready->first = NULL;
				ready->this = NULL;
			}
			// if there are things left in the ready queue
			if (isDone(ready)) {
			
				// if more processes exist in the ready queue, do a FCFS on them.
				if (ready->first->next != NULL) {
					temp = ready->first;
					temp2 = ready->first;
					prev = ready->first;
					ready->this = ready->first->next;
					fcfs(ready, temp, temp2, prev);
				}
				working = ready->first;
				currentBurst = working->burst;
			}
		}
		// else, if currentBurst != 0, so if there are still milliseconds for the process to run through
		else {			
		
			// if next node exists, and its arrival time is now, insert it to the end of the ready queue.			
			if (list->this->next != NULL) {
				TRACE;
				if (list->this->next->arrival == time) {
					TRACE;
					getLast(ready);
					insert(ready, list->this->next->id, list->this->next->arrival, list->this->next->burst);
					list->this = list->this->next;
				}
			}
		}
		
	} // while (isDone(ready))
			
	//shortestJobFirst
	
	freeSJFAndPushToComplete(list, ready, complete);	
}

void writeFile(char* fileName, List* list) {
	waitTime = 0;
	turnaround = 0;
	FILE* file;
	file = fopen(fileName, "w");
	if (file == NULL) {
		fclose(file);
		return;
	}
	list->this = list->first;
	
	while (list->this != NULL) {
		fprintf(file, "%d %d %d %d\n", list->this->id, list->this->arrival, list->this->finish, list->this->wait);
		waitTime += list->this->arrival;
		turnaround += list->this->finish;
		//printf("IN WRITEFILE(), %d %d %d %d is being inserted.\n", list->first->id, list->first->arrival, list->first->finish, list->first->wait);
		list->this = list->this->next;
	}
	fclose(file);	
}

int readFile(char* fileName, List* list, int limit) {
	int id = 0, arrival = 0, burst = 0;
	
	FILE* file = fopen(fileName, "r");
	if (file == NULL) perror ("Error opening file");
	if (limit != 0) {
		while(limit != 0) {
			if (feof(file))
				break;
			fscanf(file, "%d", &id);
			fscanf(file, "%d", &arrival);
			fscanf(file, "%d", &burst);
			insert(list, id, arrival, burst);
			limit--;
		}
	} else {
		while (!feof(file)) {
		fscanf(file, "%d", &id);
		fscanf(file, "%d", &arrival);
		fscanf(file, "%d", &burst);
		insert(list, id, arrival, burst);
		}
	}
	
	fclose(file);
	return 0;
}

void List_show(List* list) {
	Process *node;
	node = list->first;
	int nodeCounter = 0;
	while (node != NULL) {
		printf("%d, ", node->id);
		printf("%d, ", node->arrival);
		printf("%d\n", node->burst);
		node = node->next;
	}
	printf("List_show done and node count is %d\n", nodeCounter);
}

int main(int argc, char** argv) {

	char* inputFile = argv[1];						// size of List is 16, size of Process is 40
	char* outputFile = argv[2];
	int limit = 0;
	int quantum = 0;
	char* terminator = "";

	List* list = malloc(sizeof(List));
	
	list->first = NULL;
	list->this = NULL;
	
	if (argc < 4 || argc > 6) {
		printf("Error, invalid number of arguments.\n");
		return 0;
	}
	if (argc > 5 && strcmp(argv[3], "SJF") == 0) {
		printf("Error, too many arguments for SJF.\n");
		return 0;
	}
	if (argc == 4 && strcmp(argv[3], "RR") == 0)
		printf("Too few arguments for Round Robin.\n");
		
	if (argc == 5 && strcmp(argv[3], "RR") == 0) {
		readFile(inputFile, list, limit);
		quantum = atoi(argv[4]);
		printf("quantum (%d) specified and RR\n", quantum);
		roundRobin(list, quantum);
		writeFile(outputFile, list);
	}
	
	if (argc == 6 && strcmp(argv[3], "RR") == 0) {
		quantum = strtol(argv[4], &terminator, 10); // quantum = 4th arg
		limit = strtol(argv[5], &terminator, 10); // limit = 5th arg
		printf("quantum (%d) and limit specified and RR\n", quantum);
		readFile(inputFile, list, limit);
		roundRobin(list, quantum);
		writeFile(outputFile, list);
	}
	
	if (argc == 5 && strcmp(argv[3], "SJF") == 0) {
		printf("limit specified and SJF\n\n");
		limit = strtol(argv[4], &terminator, 10);
		readFile(inputFile, list, limit);
		shortestJobFirst(list, outputFile);
		writeFile(outputFile, list);
	}
	
	if (strcmp(argv[3], "SJF") == 0 && argc != 5) {
		printf("SJF with no limit specified\n\n");
		readFile(inputFile, list, limit);
		shortestJobFirst(list, outputFile);
		writeFile(outputFile, list);
	}
	List_free(list);
	free(list);
	
	waitTime = waitTime / 10000;
	turnaround = turnaround / 10000;
	printf("waitTime is %f\n", waitTime);
	printf("turnaround is %f\n", turnaround);	
	return 0;
}
