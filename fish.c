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

char *shm, *fish;  //pointers to the start of shared memory

//methods needed 
void stay();
void goRight();
void goLeft();
int findPellet();

int col; //col of location of fish

int main(){
	int shmid; //sets up shared memory with key 0001
	key_t idKey = 9856;
	
	//this locates shared memory, if error then exits
	if((shmid = shmget(idKey, millSize, 0666)) < 0){
		perror("could not locate, exiting on error!\n");
		exit(1);
	}
	
	//this attaches shared memory to address of calling process, exits on error
	if((shm = shmat(shmid, NULL, 0)) == (char *) - 1){
		perror("could not attach shared memory, exiting on error!\n");
		exit(1);
	}
	
	//sets up fish location in the mill
	fish = shm + millCols * (millRows - 1) + millRows / 2;
	*fish = 'F'; //sets the fish as F on the mill
	col = millRows / 2; // to start fish at the middle of the mill
	
	//this while loop finds where the pellet is in comparison to the fish
	//and then makes the fish move towards the pellet
	while(1){
		int pltDirection = findPellet(); //direction of pellet from fish
		if(pltDirection > 0 ){
			goLeft(); //makes fish go left if pellet is on its left
		}
		else if(pltDirection < 0){
			goRight(); //makes fish go right if pellet is on its right
		}
		else{
			stay(); //if on the same line in array then stay
		}
	}
	return(0);	
}

void stay(){
	*fish = 'F'; //since nothing changes fish outputs F in same spot
}

void goRight(){
	if(col != millRows - 1){
		*fish = '*'; //outputs mill char to replace fish char
		fish++; // increments fish location
		
		*fish = 'F'; //outputs fish char to replace mill char after increment fish
		col++; //to go with new column of the fish location
	}
}

void goLeft(){
	//same thing as above in goRight but since we are going left, we need to decrement
	//for opposite direction
	if(col != 0){
		*fish = '*';
		fish--;
		
		*fish = 'F';
		col--;
	}

}

int findPellet(){
	
	//length of fish search at a time is 1
	int search = 1;
	
	//loop to traverse through rows starting by fish
	for(int i = 0; i < (millCols - 1); i++){
	
		//left and right pointers of the same column as the fish starting point
		char *rightSide = fish - millRows * i;
		char *leftSide = fish - millRows * i;
		
		//so fish stays if it is in same column as the pellet
		if(*leftSide == '!'){
			return(0);
		}
		
		//loop so fish moves left or right depending on where the pellet is
		for(int j = 0; j < search; j++){
			//if statement to make sure the fish stays within mill from left
			if((leftSide - shm) % millRows != 0){
				leftSide--;
				if(*leftSide == '!'){
					return(1);
				}
			}
			//if statement to make sure fish stays within the mill from right
			if((rightSide - shm) % millRows != 9){
				rightSide++;
				if(*rightSide == '!'){
					return(-1);
				}
			}
		}
		
		search++;
		
		//this is a check so swim _mill rows dont go above the defined 9 
		if(search > millRows / 2){
			search = millRows / 2;
		}
		
		
	}
	return(col - 4);
}


