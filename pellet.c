#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <time.h>
#include <signal.h>
#include <unistd.h>
#include <sys/shm.h> // shmctl for the shared memory allocation
#include <sys/ipc.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <pthread.h>
#include <fcntl.h>
#include <sys/stat.h>

#define TIME 30 //for 30 sec timeout

//sets dimenions of 2d array 9*9
#define millRows 9
#define millCols 9
static const int millSize = 81;

//starts shared memory
int shmid;
key_t idKey = 9856;

char *shm, *pellet;  //pointers to the start of shared memory

int countEaten; // variable to count number of pellets ate

//pellet function instance to check if it has been eaten
static void *pelletEaten(void *arg);

int countThreads;
int totalThreads;

int main(){
	
	
	pthread_t pellet_threads[TIME]; //to store threads of pellets
	
	srand(time(NULL)); // random num
	
	//locates shared memory, 0666 allows access of memory
	if((shmid = shmget(idKey, millSize , 0666)) < 0){
		perror("Could not locate shared memory, exiting on error\n");
		exit(1);
	}
	
	// to attach memory to address of calling process, exits on error
	if((shm = shmat(shmid, NULL, 0)) == (char*) - 1){
		perror("Could not attach memory to address\n");
		exit(1);
	}
	
	
	//loop to make pellets while there is time
	for(int i = 0; i < TIME; i++){
		//randomly makes pellet and makes a thread for it
		if(rand() % 2 == 0){
			
			totalThreads++;
			int pel = pthread_create(&pellet_threads[i], NULL, pelletEaten, NULL);
			sleep(3);
			
			if(pel){ //if thread fails to create, exits
				printf("Failed to create thread: %d\n", pel);
				return 0;
			
			}
			
		}
		while(countThreads < totalThreads);
		
	}
	return(0); // exit main
}

static void *pelletEaten(void *arg){
	
	int tid = pthread_self(); //thread id store
	int drop = rand() % millCols; //where to drop pellet
	
	char *pellet = shm + drop;
	*pellet = '!';  //outputs pellet as !
	countThreads++;
	sleep(1);
	
	//loop moves pellet down the grid till second last row
	for(int i = 0; i < millCols - 2; i++){
		*pellet = '*';
		pellet += millRows;
		
		*pellet = '!';
		countThreads++;
		sleep(1);
	}
	
	*pellet = '*'; //output mill
	pellet += millRows;
	
	//to see if pellet is on the fish, therefore the fish will eat the pellet
	if(*pellet == 'F'){
		
		countEaten++;
		printf("Pellet eaten, total pellets eaten: %d\n", countEaten);
		totalThreads--;
		return NULL;
		
	}
	else{ //if pellet is not below fish, pellet is not eaten 
		
		*pellet = '!';		
	}
	
	countThreads++;
	sleep(1);
	*pellet = '*'; //output mill 
	countThreads++;
	totalThreads--;
	printf("Pellet was not eaten, fish did not find it :(\n");
	return NULL;
	
}



