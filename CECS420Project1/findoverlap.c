#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
//understanding valgrind: http://stackoverflow.com/questions/23791398/is-not-stackd-mallocd-or-recently-freed-when-all-the-variables-is-used
// method(firstListNode, secondListNode)
//remember total # of occurances, not # of repeats. Decrement repeats by 1

struct ListNode;

typedef struct ListNode {
    struct ListNode *next;
    struct ListNode *prev;
    char *value;
    int repeats;
} ListNode;

typedef struct List {
    int count;
    ListNode *first;
    ListNode *last;
} List;


int sumRepeats(ListNode* firstListNode, ListNode* secondListNode) {
	int thirdNodeRepeats = 0;
	thirdNodeRepeats = firstListNode->repeats + secondListNode->repeats;
	return thirdNodeRepeats;
}

void incrementRepeat(ListNode* node) {
	
	node->repeats++;
	//printf("just repeated. # of repeats is %d\n", node->repeats);
}

bool isWordOld(ListNode* node, char* word) {
	bool isOld = false;
	//printf("checking isWordOld for %s\n", word);
	if (node != NULL) {
		ListNode* temp;
		temp = node;
		
		while (temp != NULL) {
			if (strcmp(temp->value, word) == 0) {
				temp->repeats++; //increase temp.repeats b/c temp is the node that is the 
				//same as the word
				isOld = true;
				break;
			}
			temp = temp->next;
		}
	}
	return isOld;
}

ListNode* createNode() {
	ListNode *node = (ListNode *)malloc(sizeof(ListNode));
	node->repeats = 1;
	return node;
}

ListNode* setValue(ListNode* node, char* word) {
	node->value = word;
	return node;
}

void addNodeWithInt(List* list, ListNode* node, int repeats) {

//ListNode *node = createNode();
		
		//node->value = strdup(value); //strdup has its own memory
		node->prev = NULL;
		node->next = NULL;
		
		if(list->last == NULL) { //if empty list
			//node->repeats = 0;
		    list->first = node;
		    list->last = node;
		}
		else {
			list->last->next = node;
			node->prev = list->last;
			list->last = node;
			list->last->next = NULL; // same as node->next = NULL;
		}
		list->count++;
}

void addNode(List* list, char* value) {

	ListNode *node = createNode();
		
		node->value = strdup(value); //strdup has its own memory
		node->prev = NULL;
		node->next = NULL;
		
		if(list->last == NULL) { //if empty list
			//node->repeats = 0;
		    list->first = node;
		    list->last = node;
		}
		else {
			list->last->next = node;
			node->prev = list->last;
			list->last = node;
			list->last->next = NULL; // same as node->next = NULL;
		}
		list->count++;
}

void List_push(List* list, char *value)
{
	if(value != NULL) {
		
		//check if word is old
		if (isWordOld(list->first, value)) {
			//isWordOld increments repeats
		} else {
			addNode(list, value);
		}
	}
} 

void List_show(List* list) {
	ListNode *node;
	node = list->first;
	while (node != NULL) {
		printf("%s, ", node->value);
		printf("%d\n", node->repeats);
		node = node->next;
	}
	printf("\n");
}

void List_free(List* list) {
	ListNode *temp = NULL;
	//printf("freeing memory in %s\n", list->first->value);
	
	while(list->first != NULL) {
		temp = list->first;
		list->first = list->first->next;
		//printf("\nfreeing word: %s \n", temp->value);
		free(temp->value);
		free(temp);
	} 
	
	free(list);
}

void readFile(List* list, char* file) {
	char* word;
	FILE* inputFile = fopen(file, "r"); //open for reading
	if (inputFile == NULL) perror ("Error opening file");
	else {
		while (!feof(inputFile)) {//not end of file
		//&word is the address of word, so it points to word, so it's char**
			if (fscanf(inputFile, "%ms", &word) == 0) { //reads empty line
				free(word);
		    	break;
			} else {
				List_push(list, word);
				free(word);
			}
		} //while
		fclose(inputFile); // close file
	} //else
}

void output_push(List* list, ListNode* node) {
	if(node->value != NULL) {
		addNodeWithInt(list, node, node->repeats);
		//addNode(list, word);
	} else {printf(" word is null in output_push"); }
	//List_show(list);
}

List* createList() {
	List* list = (List*)malloc(sizeof(List));
	list->first = NULL;
	list->last = NULL;
	list->count = 0;
	return list;
}

ListNode* insert(ListNode* head, ListNode* position, ListNode* newNode) {
	if (head == position) {
		newNode->next = head;
		return newNode;
	}
	ListNode* temp = head;
	while(temp->next != position) {
		temp = temp->next;
	}
	newNode->next = temp->next;
	temp->next = newNode;
	return head;
}

void addSorted(ListNode* first, ListNode* new, List* list){
	ListNode* current;
	
	//push node to beginning if new word is < old head node or if empty
	if((first->value != NULL && new->value != NULL) && strcmp(first->value, new->value) > 0) {
		new->next = first;
		first = new;
	} else {
	  // Locate the node before the point of insertion 
		current = first;
		// while not at end of list AND next word is earlier in alphabet than new
    // so, iterate thru until new word is earlier. 
		while (current->next != NULL && current->next->value < new->value){
			current = current->next;
		}
		// then, insert in the middle
		new->next = current->next;
		current->next = new;
	} 
	
	 printf("%s\n", current->value);
	//printf("%s\n", list->first->value); 
	// doesn't work because node hasn't been added to list yet
	
}

void sort(List *list)
{
if(list->first != NULL)
{ // last node of the list is NULL
		ListNode *tmp;
		ListNode *tempCurr = list->first->next;
		do
		{ //printf("tempCurr->value is %s\n", list->first->next->value);
			while((tempCurr != list->first) && (strcmp(tempCurr->value, tempCurr->prev->value) < 0))
			{
				tmp = tempCurr;
				tempCurr = tmp->prev;
				tmp->prev = tempCurr->prev;
				tempCurr->next = tmp->next;
				//if tmp->prev isn't null set it to curr->prev else its the head of the list so set headnode to tmp.
				((tmp->prev) != NULL)?((tmp->prev)->next = tmp):(tmp->prev=NULL);		
				//if curr->next isn't null then set (curr->next)->prev to curr else its the end of list so set null.
				((tempCurr->next) != NULL)?((tempCurr->next)->prev = tempCurr):(tempCurr->next = NULL);
				tmp->next = tempCurr;
				tempCurr->prev = tmp;
			
				if(tmp->prev == NULL)
					list->first = tmp;
				
				tempCurr = tmp;
			}
		
		}while((tempCurr = tempCurr->next) != NULL);
	}
}

List* compareLists(List* firstList, List* secondList) {
	List* output = createList();
	ListNode* temp = NULL;
	ListNode* secondTemp = NULL;
	temp = firstList->first;
	
	while (temp != NULL) {
	secondTemp = secondList->first;
		
		while (secondTemp != NULL) {
			
			if (strcmp(temp->value, secondTemp->value) == 0) {
				 
				ListNode* newNode = malloc(sizeof(ListNode));// = createNode();
				newNode->value = strdup(temp->value);
				newNode->repeats = sumRepeats(temp, secondTemp);
				
				//addSorted(output->first, newNode, output);
				output_push(output, newNode);
				//List_show(output);
				//free(newNode); //////// should
			}
			secondTemp = secondTemp->next;
		}
		temp = temp->next;
	}
	return output;
}
// a < b
void sendFile(List* list, char* arg) {
	ListNode *node = list->first;
	//char str2[20];
	
	FILE* fp = fopen(arg, "w+");
	while(node != NULL) {
		fprintf(fp, "%s,%d\n", node->value, node->repeats);
	/*	fputs(node->value, fp);
		fprintf(fp, ",");
		sprintf(str2, "%d", node->repeats); // use (char*)node->repeats?
		fputs(str2, fp);
		fputs("\n", fp); */
		node = node->next; 
	}
	fclose(fp);
}

int main(int argc, char* argv[]) {

	List* firstList = createList();
	List* secondList = createList();
	
	readFile(firstList, argv[1]);
	readFile(secondList, argv[2]);
	List* outputList = compareLists(firstList, secondList);
	//List_show(outputList);
	sort(outputList);
	sendFile(outputList, argv[3]);
		
	List_show(outputList);

	List_free(firstList);
	
	List_free(secondList);
	
	List_free(outputList);

	return 0;
}
/*if (strcmp(temp->value, secondTemp->value) > 0) {
		//if 1st is earlier (A) than 2nd (Z)
} else if (strcmp(temp->value, secondTemp->value) < 0) {
		//if 1st is later (A) than 2nd (A)
} */

