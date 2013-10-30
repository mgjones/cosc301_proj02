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

int check_mode(char **line){
	
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
				if (strncmp(command[1], "sequential", 10) == 0 || strncmp(command[1], "s", 1) == 0){ // test this throughly
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
	if(cmd[0] == NULL){
		return 0;
	} else if(strncmp(cmd[0],"exit",4) == 0){
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
		
		// check for "exit"
		char **cmd = tokenify(cmd_list[i],0); // list -> command
		if(check_exit(cmd) == -1)
			isExit = -1;

		// check for "mode"
	 	char** mode_cmd = &cmd_list[i]; // two tokenifies -- one for check_exit
		check_mode(mode_cmd); 			// and another for check_mode -- fix!
		
		// check for "pause"



		// check for "resume"
		

		// stat for current command -- if stat fails as is
		// then go through list of directories and append to cmd
		// then stat each command with new path appended
		// if path+cmd is found -- exec that one immediately
		// if not found -- prompt " file not found"
		// then continue to next command


		pid_t child = fork();
		if(child == 0){
			// create new struct node -- malloc
			// put struct node in a array
			execv(cmd[0],cmd);
			// free cmd after exec
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

int sequential(char **cmd_list, char** directory){

	int isExit = 0;
    int i = 0;
    while (cmd_list[i] != NULL){

		//char** super = &cmd_list[i];
		char **cmd = tokenify(cmd_list[i],0);

		// check for "exit"
		if (check_exit(cmd) == -1){
			isExit = -1;
			i++; 			// new lines: so we don't exec "exit" command
			continue;		// new lines: ""							""
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
		
		//	//	//	//	//	//	//	//	//	//	//	//	//	//	//	//		
		
		struct stat statresult;
		int rv = stat(*cmd, &statresult);
		
		if (rv < 0){
			while (directory[i] != NULL){
				char *path_cmd = malloc(strlen(directory[i]) + strlen(*cmd) + 1);
				strcpy(path_cmd,directory[i]);
				strcat(path_cmd,*cmd);
				int ru = stat(path_cmd, &statresult);
				if (ru == 0){
					char *old_cmd = *cmd;
					cmd = &path_cmd;
					free(old_cmd);
					break;
				}
				i++;
				free(path_cmd);
			}

			if (directory[i] == NULL){
				printf("The file does not exist.");
				return isExit;
			}

		}
		//	//	//	//	//	//	//	//	//	//	//	//	//	//	//	//
		
		
		pid_t child = fork();

		if (child == 0){
			execv(cmd[0], cmd);
			// free cmd
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
		// might still be a segfault if user types in something like "asd" or "as" etc.
		return 1;
	} else {
     	return 0;
	}
}
 

void split(char* line){ // assuming shell-config is in correct format
	int i = 0;
	while(line[i] != '\n'){i++;}
	line[i] = '\0';
	return;

}

int main(int argc, char* argv[]){

	char *prompt = "mjng$ ";
	int isExit = 0;
	

	// reading in file
	FILE *file;
	file = fopen("shell-config","r");
	if(file == NULL){	
		printf("could not find file \"shell-config\": input commands using full path name\n");
		// path_switch = 1;		
	}

	char line[1024];
	int i = 0;

	//char* directory[1024];
	while(fgets(line,1024,file) != NULL){
		split(line);
		directory[i] = line;
		i++;
	}
	directory[i] = NULL;

	fclose(file);
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

