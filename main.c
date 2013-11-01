//	//	//	//	//	//	//	//	//	//	//
//										//
//	By Mariah Jones and Nolan Gonzalez	//
//			   COSC 301				    //
//										//
//	//	//	//	//	//	//	//	//	//	//



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

int mode = 0; // global variable mode is set to sequential

int get_size(char** array){			// get's the size of an array (char**)
	int i;
	for(i=0;array[i] != NULL;i++){}
	return i;
}

char *remove_hash(char *buffer){	// puts a null turmination
	int i = 0;						// char where hash is
	while (buffer[i] != '\0'){
		if (buffer[i] == '#')
			buffer[i] = '\0';
		i++;
	}
	
	return buffer;

}

void split(char* line){ // assuming shell-config is in correct format
	int i = 0;
	while(line[i] != '\n'){i++;}
	line[i] = '\0';
	return;

}

char **tokenify(const char *str,int swap) { // separates the command line into 'tokens'
	const char *sep;

	if(swap == 0){
		sep = " \t\n";
	}
	else {
		sep = ";";
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

int check_mode(char **line){ // check to see if mode is an input parameter of the command line

	char** command = tokenify(line[0],0);

	if(command[0] == NULL){
		return -1;
	} else if (strncmp(command[0],"mode",4) == 0) {
			if (command[1] == NULL){
				if (mode == 0) {
					printf("mode: Sequential\n");
					return 0;
				} else {
					printf("mode: Parallel\n");
					return 0;
				}
			} else {
				// there is a parameter
				if (strncmp(command[1], "sequential", 10) == 0 || strncmp(command[1], "s", 1) == 0){
					mode = 0;
					return 0;
				} else if (strncmp(command[1], "parallel", 8) == 0 || strncmp(command[1], "p", 1) == 0){
					mode = 1;
					return 0;				
				}
			}
	}
	
	return -1;
}

int check_exit(char** cmd){ // checks to see if exit is a parameter in the command line
	if(cmd[0] == NULL){
		return 0;
	} else if(strncmp(cmd[0],"exit",4) == 0){
		 return -1;
	}
	return 0;
}

// define check_pause function
//

// define check_resume function
//


// check_stat returns a new concatted string with path if stat
// is successful. If not then it just returns the cmd that was passed
char* check_stat(char** cmd, char** directory){
	if(*cmd == NULL){
		return *cmd;
	} 

	char* new = NULL;
	struct stat statresult; 
	int i = 0;
	int rv;
	while( directory[i] != NULL){
		char* new_str = strdup(directory[i]);
		strcat(new_str, "/");
		strcat(new_str, cmd[0]);
		rv = stat(new_str, &statresult);
		if (rv < 0){
			i++;
			free(new_str);
			continue;
		} else {
			new = new_str;
			break;
		}
	}// end while
	
	if( directory[i] == NULL){
		return *cmd;
	}

	int k = 1;
	while( cmd[k] != NULL){
		strcat(new, " ");
		strcat(new, cmd[k]);
		k++;
	}
	free(*cmd);
	return new;
	
}



int parallel(char** cmd_list, char** directory){

	int rv;
	int isExit = 0;
	pid_t child_pids[get_size(cmd_list)+1];	// array of struct nodes -- linked list	
	int i = 0;

	while(cmd_list[i] != NULL){ // for each cmd
		
		// check for "exit"
		char **cmd = tokenify(cmd_list[i],0); // list -> command
		if(check_exit(cmd) == -1){
			isExit = -1;
			i++;
			continue;
		}

		// check for "mode"
		char** mode_cmd = &cmd_list[i];
		int isMode = check_mode(mode_cmd);
		if (isMode == 0){
			i++;
			continue;
		}
		
		// check for "pause"



		// check for "resume"
		
	
		char* new_cmd;
		struct stat statresult;
		rv = stat(*cmd, &statresult);
		if(rv < 0){
			
			new_cmd = check_stat(cmd,directory);
			if(new_cmd == NULL){
				i++;
				continue;
			}
			if(new_cmd[0] !='/' && new_cmd[0] != '.'){ // if no path match found
				printf("command %s was not found: type full path for command\n",new_cmd);
				i++;
				continue;
			} else { // if path match found	
				cmd = tokenify(new_cmd,0);
			}

		} // end if


		pid_t child = fork();
		if(child == 0){
			execv(cmd[0],cmd);
			exit(1);
		} else {
			child_pids[i] = child;
		}
		i++;	
	}

	child_pids[i] = 0; 
	int wait_count = i;
	i = 0;
	int* status = NULL;
	int* astatus = NULL;
	int astat = 0;

	pid_t accident = fork(); // the accident child will do the waiting for the other children
							 // while the parent exits and goes back to user prompt
	if(accident == 0){
		// wait on children
		while(i < wait_count){
			while(1){
				astat = waitpid(child_pids[i], status,0);
				if(astat > 0){								
					printf("Process %d completed.\n",child_pids[i]);
					break;
				}
			}
			i++;
		}
	} else { // parent
		waitpid(accident, astatus, WNOHANG);
	}
	return isExit;

}

int sequential(char **cmd_list, char** directory){
	
	int isExit = 0;
    int i = 0; 												// increment for each command
	int rv;
    while (cmd_list[i] != NULL){							// for every command

		char **cmd = tokenify(cmd_list[i],0); 				// tokenify for params

		// check for "exit"
		if (check_exit(cmd) == -1){
			isExit = -1;
			i++;
			continue;
		}

		// check for "mode"
		char** mode_cmd = &cmd_list[i];
		int isMode = check_mode(mode_cmd);
		if (isMode == 0){
			i++;
			continue;
		}

		// check for "pause"



		// check for "resume"
		

		char* new_cmd;
		struct stat statresult;
		rv = stat(*cmd, &statresult);
		if(rv < 0){
			
			new_cmd = check_stat(cmd,directory);
			if(new_cmd == NULL){
				i++;
				continue;
			}
			if(new_cmd[0] !='/' && new_cmd[0] != '.'){ // if no path match found
				printf("command %s was not found: type full path for command\n",new_cmd);
				i++;
				continue;
			} else { // if path match found	
				cmd = tokenify(new_cmd,0);
			}

		} // end if
		
		
		pid_t child = fork();

		if (child == 0){
			execv(cmd[0], cmd);
            exit(1);
		}else{
			int *status = NULL;
            waitpid(child, status, 0);
		}
        i++;

	}
	return isExit;
}

int isBuilt_in(char *cmd){
	if(cmd == NULL){
		return 0;
	} else if (strncmp(cmd, "mode", 4) == 0 || strncmp(cmd,"exit", 4) == 0 || strncmp(cmd, "pause", 5) == 0 || strncmp(cmd, "resume", 6) == 0){
		return 1;
	} else {
     	return 0;
	}
}
 



int main(int argc, char* argv[]){

	char *prompt = "mjng$ ";
	int isExit = 0;
	

	// reading in file
	FILE *file;
	file = fopen("shell-config","r");
	if(file == NULL){	
		printf("could not find file \"shell-config\": input commands using full path name\n");
	}

	// putting lines of directory in char** //
	char line[1024];
	int i = 0;
	char** directory = malloc(1024 * 7);
	while(fgets(line,1024,file) != NULL){
		split(line);
		char* new_line = strdup(line);
		directory[i] = new_line;
		i++;
	}
	directory[i] = NULL;
	fclose(file);
	//	//	//	//	//	//	//	//	//	//	//

	printf("%s", prompt);

	fflush(stdout);
	char buffer[1024];

	while (fgets(buffer, 1024, stdin) != NULL){
		char* str = remove_hash(buffer);
	  	char** cmd_list = tokenify(str,1);
	   	

		char** cmd = &cmd_list[0];			// checks if command is mode	  //
		if(check_mode(cmd) == 0){			// if not then run command		  //
			printf("%s", prompt);			// if it is then skip unnecessary //
			continue;						// exec.						  //
		}


		if(mode == 0){ // sequential
			isExit = sequential(cmd_list,directory);

			if( isExit == -1 && mode == 0)
				break;
	  		else if (isExit == -1 && mode == 1){
	   			mode = 1;
		   		break;
			} else if (isExit == 0 && mode == 1){
				mode = 1;
	   		}
		} else { // parallel
			isExit = parallel(cmd_list,directory);
		
			if ( isExit == -1 && mode == 1)
		  		break;
		 	else if (isExit == -1 && mode == 0){
		     	mode = 0;
		       	break;
		  	}else if ( isExit == 0 && mode == 0){
		    	mode = 0;
		 	}	   	
		}
		printf("%s", prompt);
	}
	i = 0;
	while(directory[i] != NULL){	// free all strings in directory //
		free(directory[i]);			// before exiting out of program //
		i++;
	}
    return 0;
}

/*
   We didn't want to send this in really late so we compensated with this.
   Hopefuly this is enough to show our hardwork over 2 weeks of effort.

   If we would've implemented background jobs:
   We were going to implement a linked list of struct "nodes" to keep track
   of all the processes that were being executed (aside from mode, exit, pause
   and resume). A "jobs" function would see if a user typed in jobs and just go
   through the linked list of process printing out all the information about each
   process (which is stored within each node, ie. command_name, pid, paused_state).

   a pause function would see if the user put in "pause". If so then we would
   do kill(pid, SIGSTOP) on the pid that was paused...same thing for resume function
   but with kill(pid, SIGCONT).



*/
