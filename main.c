#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <sys/resource.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <poll.h>
#include <signal.h>

void parallelize (char ***, int);
void copy_stuff (char ***, char **, int);


int main(int argc, char **argv) {

	int modesp = 2; //sets mode to default sequential

	char ***exec_ready = (char ***)malloc((128)*(sizeof(char **)));  // to be passed into execv
	char ***parallel_arr = (char ***)malloc((128)*(sizeof(char **)));
	

	while(1) {
		char cmds[1024];   //array of input

		printf("newShell $ "); //prints prompt
		fflush(stdout);

                const char *hash = "#";
                const char *sep = " \t\n";
		const char *semi = ";";
                char *temp, *temp2, *word, *line, *prog;
		int BigCount = 0;  //counter for exec_ready

		// READING IN LINE, PARSING AND ORGANIZING INTO EXEC_READY
		if (fgets(cmds, 1024, stdin) != NULL) {
			if (cmds[0] == 4) {}
			if (cmds[0] == 10) {}			
			else if (cmds[0] != 35) {
				for(line = strtok_r(cmds, hash, &temp); line != NULL; line = strtok_r(NULL, hash, &temp)) {
					for(prog = strtok_r(line, semi, &temp); prog != NULL; prog = strtok_r(NULL, semi, &temp)) {
                                                char *tmp[128];
                                                int count = 0;
						char *temp_prog = prog;

						for(word = strtok_r(temp_prog, sep, &temp2); word != NULL; word = strtok_r(NULL, sep, &temp2)) {
							tmp[count] = word;
							count++;
						}
						tmp[count] = '\0';
						copy_stuff(exec_ready, tmp, BigCount);
						BigCount++;
         				}
				}
			}
		} //end if fgets()

		exec_ready[BigCount] = '\0';
		char **current;
		int k = 0;
		int leave = 0; // exit = 0, don't exit = 1
		//char ***parallel_arr = (char ***)malloc((128)*(sizeof(char **)));
		int parallel_c = 0;
		int c = 0;

		// CASES: EXIT --- MODE S OR P --- SEQUENTIAL?! --- parallel below...
		while (exec_ready[k] != '\0') {
			current = exec_ready[k];
			// CASE EXIT 
			if (strcasecmp(current[0],"exit") == 0) {
				leave = 1;
				k++;
				continue;					
			} 
			
			// CASE PARALLEL -- switch modes
			if (strcasecmp(current[0],"mode") == 0  && ((strcasecmp(current[1],"parallel") == 0) || (strcasecmp(current[1],"p") == 0))) {
				modesp = 1;
				printf("Now in parallel mode.\n");
				fflush(stdout);
				k++;
				continue;
			}

			// CASE SEQUENTIAL -- switch modes
			if (strcasecmp(current[0],"mode") == 0  && ((strcasecmp(current[1],"sequential") == 0) || (strcasecmp(current[1],"s") == 0))) {
				modesp = 2;
				printf("Now in sequential  mode.\n");
				fflush(stdout);
				k++;
				continue;
			}	

			// IN SEQUENTIAL MODE -- executing fork() and execv() for each k in exec_ready
			char *currword;
			currword = current[0];
			if ((modesp == 2) && (currword[0] == '/')) { 
				pid_t child_pid = fork();
				current = exec_ready[k];
				if (child_pid == 0) {
					if (execv (current[0], current) < 0) {
						fprintf (stderr, "execv failed %s\n",strerror(errno));
					} 
				}
				else {
					int result = 0; 
					waitpid(child_pid, &result, 0);
				}
			}
			//IN PARALLEL MODE --
			else if ((modesp == 1) && (currword[0] == '/')) {
				parallel_arr[c] = exec_ready[k];
				c++;
			} 		

			else {
				fprintf (stderr, "Invalid input. %s\n",strerror(errno));

			}

			parallel_arr[c] = '\0';
			parallel_c = c;	

			k++;
		}   // end while	
		
		parallelize(parallel_arr, parallel_c);

		if (leave == 1) {
			char **p;
			int j = 0;
			int h = 0;
			while (exec_ready[h] != '\0') {
				p = exec_ready[h];
				while (p[j] != '\0') {
					free (p[j]);
					j++;
				}
				free(exec_ready[h]);
				h++;
			}
			free(exec_ready);
			free(parallel_arr);

			exit(0);
		}

	} //ends main while(1)

    return 0;
}

//***********************PARALLEL FUNCTION***************************
void parallelize (char ***array, int count) {
	int i = 0;
	char **current;
	for (; i < count; i++) {
		current = array[i];
		//printf("%s\n",current[i]);
		//printf("%d\n", i);
		pid_t pid = fork();
		if (pid == 0) {
			if (execv (current[0], current) < 0) {
				fprintf (stderr, "execv failed%s\n", strerror(errno));
			}
		}
	}
	i = 0;
	int status = -1;
	for (; i < count; i++) {
		waitpid(-1,&status,0);
	}
} 

//***********************COPY FUNCTION ***************************
void copy_stuff (char ***exec_array, char **temp, int index) {
	char **copier = (char **)malloc((128)*(sizeof(char *)));	

	int i = 0;	

	while (temp[i] != '\0') {
		copier[i] = strdup(temp[i]);
		i++;
		
	} 
	copier[i] = '\0';
	exec_array[index] = copier;
	
}
