#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <strings.h>
#include <ctype.h>
#include <sys/resource.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <poll.h>
#include <signal.h>

char **tokenify(const char *str,int swap) {
	const char *sep;

	if(swap == 0){
		sep = " \t\n";
	}
	else if(swap == 1){
		sep = ";";
	} else {
		sep = "#";
	}

	char *temp, *c;
	char *s = strdup(str);

	char **words = malloc(strlen(s) * sizeof(char));

	int count = 0;

	for (c = strtok_r(s, sep, &temp); c != NULL; c = strtok_r(NULL, sep, &temp)){
		words[count] = strdup(c);
		count++;
	}

	words[count] = NULL;
	free(s);
	return words;
}

int check_mode(char **line, int mode_choice){
	
	char** command = tokenify(line[0],0);
	if (strncmp(command[0],"mode",4) == 0) {
			if (command[1] == NULL){
				if (mode_choice == 0) {
					printf("mode: Sequential\n");
				} else {
					printf("mode: Parallel\n");
				}
			} else {
				//there is a parameter
				if (strncmp(command[1], "sequential", 10) == 0 || strncmp(command[1], "s", 1) == 0){
					mode_choice = 0;
				} else if (strncmp(command[1], "parallel", 10) == 0 || strncmp(command[1], "p", 1) == 0){
					mode_choice = 1;				
				}
			}// end else
	}// end first if
	return mode_choice;
}



void sequential(char **line) {

	pid_t child;
	int i = 0;
	while(line[i] != NULL){
		char** command = tokenify(line[i],0); // parses a command			
		child = fork();

		if (child == 0){
			// this is child process
			execv(command[0],command);
		} else {
			// this is parent process
			int *status = NULL;
			waitpid(child,status,0);
		}
		i++;
	}
	return;
}

void parallel(char **line) {
	// code goes here
	printf("we're inside god's kangaroo pouch\n");
	return;
}


int main(int argc, char **argv) {
	char* prompt = "mjng$ ";
	int mode_choice = 0;

	printf ("%s", prompt );
	fflush ( stdout ); /* if you want the prompt to immediately appear ,
						call fflush . it ’flushes ’ the output to screen */

	char buffer [1024];
	while ( fgets(buffer , 1024 , stdin) != NULL) {
		// process current command line in buffer

		char** line = tokenify(buffer,3); //cleans all comments out
		line = tokenify(line[0],1); // parses line into commands
		mode_choice = check_mode(line, mode_choice);
		
		// choose mode to run processes
		if (mode_choice == 0){
			sequential(line);		
		} else {
			parallel(line);	
		}

		printf ("%s", prompt );


	}

    return 0;
}

