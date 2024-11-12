#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

int value = 5;

int main(){
	pid_t pid;
    char aux;
	pid = fork();
	
	if (pid == 0) { /* child process */
		value += 15;
		aux = getchar();
		return 0;
	}
	else if (pid > 0) { /* parent process */
		wait(NULL);
		printf ("PARENT: value = %d\n",value); /* LINE A */
		return 0;
	}
}