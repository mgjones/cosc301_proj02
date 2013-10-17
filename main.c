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

char **tokenify(const char *str) {

	const char *sep = ";";
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

int main(int argc, char **argv) {
	char* prompt = "mjng$ ";

	printf ("%s", prompt );
	fflush ( stdout ); /* if you want the prompt to immediately appear ,
						call fflush . it ’flushes ’ the output to screen */

	char buffer [1024];
	while ( fgets(buffer , 1024 , stdin) != NULL) {
	// process current command line in buffer
		

	
	}

    return 0;
}

