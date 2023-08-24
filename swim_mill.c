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

char *shm, *mill;  //pointers to the start of shared memory

void whenInterrupted();
void whenTerminated();
void memDisplay(char *shm);

int shmid;
key_t idKey = 9856;  //for shared mem and its key
 

struct shmid_ds buf;

//process ID for all 3 processes
pid_t pid;
pid_t pellet;
pid_t fish;

int main(){
	signal(SIGINT, whenInterrupted);  //signal for interruption
	pid = getpid();  //calls function to get process ID
	
	//prints initial messages including first mill ID
	printf("Welcome to the swim mill!\n");
	printf("swim mill started with ID:(pid: %d)\n", pid);
	
	//sets flag to perform functions, ipc_creat to make sure it creates new shm segment
	if((shmid = shmget(idKey, millSize,0666|IPC_CREAT)) < 0){
		perror("flagged in shmget, will now exit\n");
		exit(1);
	}
	
	//to attach shm to address of the calling process
	if((shm = shmat(shmid, NULL, 0)) == (char*) - 1){
		perror("Failed to attach shared memory, will now exit\n");
		exit(1);	
	}
	
	printf("Shared memory has been created with ID: (shmid: %d)\n", shmid);
	
	//creating our 2d array shm with '*' symbols in a for loop
	mill = shm;
	for(int x=0; x < millRows; x++){
		for(int y=0; y < millCols; y++){
			*mill = '*';
			mill++;
		}
	}
	mill = shm;
	
	//to create fish process
	fish = fork();
	if(fish == 0){
		/*fork creates overlay process with file "fish" that has our code for the
		fish process, it is a char len list of args which must end in a NULL */
		const char *arg;
		execl("./fish", arg, (char*) NULL);
	}
	printf("Starting fish process with ID: (pid: %d)\n", fish);
	
	//to create pellet process
	pellet = fork();
	if(pellet == 0){
		//same thing with fish process above, except now it is for pellet process
		const char *arg;
		execl("./pellet", arg, (char*) NULL);
	}
	printf("Starting pellet process with ID: (pid: %d)\n", pellet);
	
	//printing swim mill while time is not 0
	for(int t = TIME; t > 0; t--){
		printf("Time remaining: %d\n", t);
		memDisplay(shm); //calls memDisplay to print shared memory of mill
		sleep(1); //waits for 1 second	
	}
	printf("30 second time limit completed, Program is done and will exit.\n");
	
	//function call for termination and exitting main
	whenTerminated();
	return(0);
}	
//calls to terminate program if interrupted
void whenInterrupted(){
	printf("Program interrupted.\n");
	whenTerminated();
}

void whenTerminated(){
	printf("Program is being terminated, killing all processes.\n");
	
	kill(fish,SIGTERM);
	kill(pellet,SIGTERM);
	printf("Killed pellet and fish processes!");
	
	shmdt(shm);
	shmctl(shmid, IPC_RMID, &buf);
	printf("Shared memory has been detached from program and has been removed completely!\n");
	
	printf("Exiting Program. Bye Bye... .... .....\n");
	exit(0);
}

//prints shared mem by the use of loop
void memDisplay(char *shm){
	for(int x = 0; x < millRows; x++){
		for(int y = 0; y < millCols; y++){
			char c = *shm;
			shm++;
			putchar(c);
			putchar(' '); //for space
	}
		putchar('\n');
	}
	putchar('\n');
}

/*
final comments: all processes now work, the issue I had with the pellet process started as where 
initially it would not spawn any pellets, my fish spawned at the bottom middle but no pellets would spawn.
and so I went back to my program and found that I was missing a while statement so that the spawning of pellets
would keep happening while my thread counter was smaller than total number of threads. After this I 
ran into another problem where all my pellets spawned at the same time on the top of the mill, so I fixed this by sleeping my spawning loop for 1 second
where then I ran into another problem of the mill stopping after a certain amount of pellets were eaten even if there was still time.
So I played around with my thread counter and total counter increment and decrements to no avail till I tried to sleep 
my spawning loop for 3 seconds instead of 1 which worked somehow although admittedly I do not understand how or why this change worked.
*/
