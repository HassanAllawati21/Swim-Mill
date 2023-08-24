all: swim_mill fish pellet

swim_mill: swim_mill.c
	gcc -pthread -o swim_mill swim_mill.c

fish: fish.c
	gcc -pthread -o fish fish.c 

pellet: pellet.c
	gcc -pthread -o pellet pellet.c 
