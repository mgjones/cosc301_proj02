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
				// there is a parameter
				if (strncmp(command[1], "sequential", 10) == 0 || strncmp(command[1], "s", 1) == 0){
					mode_choice = 0;
				} else if (strncmp(command[1], "parallel", 8) == 0 || strncmp(command[1], "p", 1) == 0){
					mode_choice = 1;				
				}
			}// end else
	}// end first if
	
	return mode_choice;
}



void sequential(char **line, int* mode_choice) {

	pid_t child;
	int i = 0;
	while(line[i] != NULL){
		char** command = tokenify(line[i],0); // parses a command			
		child = fork();

		if (child == 0){
			// this is child process
			int mode_before = *mode_choice;
			char** new_line = &line[i];
			*mode_choice = check_mode(new_line, *mode_choice);

			if (mode_before != *mode_choice){
				char** new_line = &line[i+1];
				parallel(new_line, mode_choice);
			}

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

int get_size(char** array){

	int i;
	for(i=0;array[i] != NULL;i++){}
	return i;
}

void parallel(char **line, int* mode_choice) {

	pid_t child;
	int i = 0;
	pid_t child_pids[get_size(line)];
	while(line[i] != NULL){
		char** command = tokenify(line[i],0); // parses a command			
		child = fork();

		if (child == 0){
			// this is child process
			int mode_before = *mode_choice;
			*mode_choice = check_mode(line, *mode_choice);
			
			if (mode_before != *mode_choice){ 
				char** new_line = &line[i+1];
				sequential(new_line, mode_choice);
			}

			execv(command[0],command);
			exit(1); 
		} else {
			// this is parent -- storing child pid into array
			child_pids[i] = child;
		}						
		i++;
	}
	child_pids[i] = NULL;
	i = 0;
	int* status = NULL;
	while(line[i] != NULL){
		waitpid(child_pids[i],status,0);
		i++;
	}	
	return;
}


int main(int argc, char **argv) {
	char* prompt = "mjng$ ";
	int mode_choice = 0	;
	int* mode_choicep = &mode_choice;

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
			sequential(line,mode_choicep);
		} else {
			parallel(line,mode_choicep);	
		}
	
		
		printf ("%s", prompt );


	}

    return 0;
}



//								 //
// SMALL PROBLEMS WE HAVE TO FIX // 
//								 //
//
// line 56/line 58 -- if user types in mode p sequentialllyyl..it still registers because it only
//					  compares if first 10 characters (size of sequential) in the user input.
//					  what should we do.
//
// general -- does parallel really work?


