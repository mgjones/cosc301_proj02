#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <ctype.h>
#include <sys/resource.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <poll.h>
#include <signal.h>


int main(int argc, char **argv) {

    printf("main.c, so lonely.  no code here yet :-(\n");
	printf ("%s", prompt );
	fflush ( stdout ); /* if you want the prompt to immediately appear ,
						call fflush . it ’flushes ’ the output to screen */

	char buffer [1024];
	while ( fgets(buffer , 1024 , stdin) != NULL) {
	// process current command line in buffer

	
	}

    return 0;
}

