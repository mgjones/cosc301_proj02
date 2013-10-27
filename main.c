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

int mode = 0; // mode is set to sequential

char **tokenify(const char *str,int swap) {
	const char *sep;

	if(swap == 0){
		sep = " \t\n";
	}
	else {
		sep = ";";
	}

	char *temp, *c;
	char *s = strdup(str)
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

int check_mode(char **line){
	
	char** command = tokenify(line[0],0);
	if (strncmp(command[0],"mode",4) == 0) {
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

int check_exit(char** cmd){
	if(strncmp(cmd[0],"exit",4) == 0){
		 return -1;
	}
	return 0;
}

int get_size(char** array){
	int i;
	for(i=0;array[i] != NULL;i++){}
	return i;
}

char *remove_hash(char *buffer){
	int i = 0;
	while (buffer[i] != '\0'){
		if (buffer[i] == '#'){
			buffer[i] = '\0';
		}
		i++;
	}
	
	return buffer;

}

int parallel(char** cmd_list){
	int isExit = 0;
	pid_t child_pids[get_size(cmd_list)+1];
	int i = 0;
	// array of struct nodes -- linked list	

	while(cmd_list[i] != NULL){ // for each cmd
		char **cmd = tokenify(cmd_list[i],0); // list -> command
		if(check_exit(cmd) == -1)
			isExit = -1;
	 
		check_mode(cmd); 		
		
		pid_t child = fork();
		if(child == 0){
			// create new struct node -- malloc
			// put struct node in a array
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

	// wait on children
	while(i < wait_count){
		waitpid(child_pids[i], status,0);
		// check if it's dead -- if yes print prompt
		i++;
	}
	return isExit;

}

int sequential(char **cmd_list){

	int isExit = 0;
    int i = 0;
    while (cmd_list[i] != NULL){
		char **cmd = tokenify(cmd_list[i],0);
		if (check_exit(cmd) == -1){
			isExit = -1;
		}

		char** mode_cmd = &cmd_list[i];
		check_mode(mode_cmd);

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


int main(int argc, char* argv[]){

	char *prompt = "mjng$ ";
	int isExit = 0;
	int path_switch = 0; // 0 - expecting no path with command
						 // 1 - expecting path with command
	printf("%s", prompt);

	fflush(stdout);
	char buffer[1024];

	while (fgets(buffer, 1024, stdin) != NULL){
		char* str = remove_hash(buffer);
	  	char** cmd_list = tokenify(str,1);
	   	

		char** cmd = &cmd_list[0];			// checks if command is mode
		if(check_mode(cmd) == 0){			// if not then run command
			printf("%s", prompt);			// if it is then skip unnecessary
			continue;						// exec.
		}
/*
		while(!path_switch){
			-- read each line of shell-config
			-- append shell-config line before command given
			-- test if command is in path (given by line) with stat
			-- if it isn't in path continue to next line
			-- if it is then assign cmd to "path"+command and break
				-- now seq/par will get command with path
			-- if no more lines to read prompt user that it couldn't find
			 - t	
		}
*/
		if(mode == 0){ // sequential
			isExit = sequential(cmd_list);

			if( isExit == -1 && mode == 0)
				break;
	  		else if (isExit == -1 && mode == 1){
	   			mode = 1;
		   		break;
			} else if (isExit == 0 && mode == 1){
				mode = 1;
	   		}
		} else { // parallel
			isExit = parallel(cmd_list);
		
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
    return 0;
}

