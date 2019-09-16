#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
extern char **getlex();
extern int getCount(); 
void exec_cmd(char** cmd, int numArgs);
void exec_file(char** args, char* filename, bool append);
void read_file(char** args, char* filename); 
void exec_pipe(char** args);
void exec_pipe_2(char** args);
int main() {
	int i, j, k, prev; //Initialize iterators 
	int numArgs; 
	int numPipes; 
	bool isPiping; 
	char **args, **args2; 
	printf("> "); 
	
	//Main while loop
	while(1) { 
		args = getlex();
		//get count to malloc
		numArgs = getCount(); 
		prev = 0;
		numPipes = 1;
		isPiping = false;
		//iterate through args
		for(i = 0; args[i] != NULL; i++) {
			k = 0; 
			//if piping, count number of pipes
			if(strcmp(args[i], "|") == 0){
				numPipes++;
			        isPiping = true; 	
			}
			//malloc secondary array
			args2 = malloc((numArgs) * sizeof(char*)); 
			//if there are semicolons, execute all of the individual commands
			if (strcmp(args[i], ";" ) == 0 && args[i+1] != NULL){
				for(j = prev; j < i; j++){
					args2[k] = args[j];
					k++;
				}
				prev = i + 1; 
				exec_cmd(args2, numArgs); 	
				if(args2 != NULL){
					for(int x = 0; args2[x] != NULL; x++){
						args2[x] = 0;  
					}
				}
			}
			//if only one command, or last in semicolon sequence
			if(args[i+1] == NULL && !isPiping){
				for (j = prev; j <= i; j++){
					args2[k] = args[j]; 
					k++;
				}
				exec_cmd(args2, numArgs); 	
				if(args2 != NULL){
					for(int x = 0; args2[x] != NULL; x++){
						args2[x] = 0;
					}		 	
				}
			}
			free(args2); //free secondary array
		}
		//if two commands w/ piping, redirect to two pipe function
		if(numPipes == 2){
			exec_pipe_2(args); 
		}
		//if three commands w. piping, redirect to three pipe function
		else if(numPipes == 3){
			exec_pipe(args); 
		}else if(numPipes > 3) {
			fprintf(stderr, "Too many pipes"); 
		}

		printf("> "); 
	}	
}

//Main execution function
void exec_cmd(char** cmd, int numArgs){
	//Flags for writing, appending, or reading a file
	bool to_file, append_file, in_file;
	to_file = false;
	append_file = false;
	in_file = false;
	int i;
	int at;
	char **command; 
	char *filename; 
	at = 0;
	//exit command
	if(cmd != NULL){
		if(strcmp(cmd[0], "exit") == 0){
			exit(1);
		}
	}	
	//cd command
	if(strcmp(cmd[0], "cd") == 0){
		if(chdir(cmd[1]) != 0){
			perror(cmd[1]);
			return;
		}
		return;
	}	
	//check if redirecting or reading from file
	for(i = 0; cmd[i] != NULL; i++) {
		if((strcmp(cmd[i], "<") == 0)){
			in_file = true; 
			at = i;	
			break;
		}	
		if((strcmp(cmd[i], ">") == 0) && (strcmp(cmd[i+1], ">") != 0)){
			to_file = true;
			at = i;
			break;	
		}
		if((strcmp(cmd[i], ">") == 0) && (strcmp(cmd[i+1], ">") == 0)){
			append_file = true;
			at = i; 
			break;	
		}
	}
	//if no redirection, standard execution
	if(!to_file && !append_file && !in_file){
		if (fork() != 0){
			wait(&errno);

		}else{
			if(execvp(cmd[0], cmd) == -1){
				printf("%s: Command not found.\n", cmd[0]);
			}
			exit(errno);
		}
	}
	//redirect to file method
	if(to_file){
		command = malloc((numArgs) * sizeof(char*));	
		filename = cmd[at+1]; 

		for(i = 0; i < at; i++){
			command[i] = cmd[i];
		}
		command[at] = 0; 
		exec_file(command, filename, append_file);
		for(i = 0; i < at; i++){
			command[i] = 0;
		}
		free(command);
	}
	//redirect to file method
	if(append_file){
		command = malloc((numArgs) * sizeof(char*)); 
		for(i = 0; i < at; i++){
			command[i] = cmd[i];
		}
		exec_file(command, cmd[at+2], append_file);
		for(i = 0; i < at; i++){
			command[i] = 0;
		}
		free(command);
	}
	//redirect to read file method
	if(in_file){
		command = malloc((numArgs) * sizeof(char*)); 
		for(i = 0; i < at; i++){
			command[i] = cmd[i];
		}
		read_file(command, cmd[at+1]);
		for(i = 0; i < at; i++){
			command[i] = 0;
		}
		free(command);
	}
}

//method to write to files
void exec_file(char **command, char *filename, bool append){
	int file_desc;
	int dup_stdout;
	if(fork() != 0){
		waitpid(-1, NULL, 0); 
	}else{
		if(append){
			file_desc = open(filename, O_RDWR | O_CREAT | O_APPEND); //append
		}else{
			file_desc = open(filename, O_RDWR | O_CREAT | O_TRUNC); //new file
		}
		dup_stdout = dup2(file_desc, 1); 
		close(file_desc); 
		if(execvp(command[0], command) == -1){
			fprintf(stderr, "%s: Command not found.\n", command[0]);
		}
		close(dup_stdout); 
		exit(1); 
	}
}

//method to read from files
void read_file(char **args, char *filename){
	int file_desc; 
	int dup_stdin; 
	if(fork() != 0){
		waitpid(-1, NULL, 0); 
	}else{
		file_desc = open(filename, O_RDONLY);
		if(file_desc == -1){
			perror(filename); 
			return;
		}	
		dup_stdin = dup2(file_desc, 0); 
		close(file_desc);
		if(execvp(args[0], args) == -1){
			fprintf(stderr, "%s: Command not found.\n", args[0]);
		}
		close(dup_stdin); 
		exit(1); 
	}
}

//function to execute piping w/ two commands
void exec_pipe_2(char** args){
	char** cmd1; 
	char** cmd2;
	int i;
        int j; 	
	int pipeIndex = 0;
	int numArgs = 0; 	
	for(i = 0; args[i] != NULL; i++){
		numArgs++; 
		if(strcmp(args[i], "|") == 0){
			pipeIndex = i;
		}
	}

        cmd1 = malloc(numArgs * sizeof(char*));	
	cmd2 = malloc(numArgs * sizeof(char*)); 
	for(i = 0; i < pipeIndex; i++){
		cmd1[i] = args[i]; 
	}
	j = 0; 
	for(i = pipeIndex+1; args[i] != NULL; i++){
		cmd2[j] = args[i];
		j++;
	}

        int fd[2];
	pid_t pid;
        pipe(fd); 
	pid = fork(); 	
	if(pid == 0){	// Run first command if child 
		dup2(fd[1], 1); 
		close(fd[0]); 
		close(fd[1]); 
		//execvp(cmd1[0], cmd1);
		exec_cmd(cmd1, 10); 
	        exit(1); 	
	}else{		
		pid = fork(); 
		if(pid == 0){ //If child, run second command
			dup2(fd[0], 0); 
			close(fd[0]); 
			close(fd[1]); 
			//execvp(cmd2[0], cmd2);
			exec_cmd(cmd2, 10); 
			exit(1); 
		}else{	// Parent waits and closes pipe
			int status; 
			close(fd[1]); 
			close(fd[0]); 
			waitpid(pid, &status, 0); 
		}
	}
	for (i = 0; cmd1[i] != NULL; i++){
		cmd1[i] = 0; 
	}
	for (i = 0; cmd2[i] != NULL; i++){
		cmd2[i] = 0; 
	}

	free(cmd1); 
	free(cmd2); 
}

//method to execute three commands w/ piping
void exec_pipe(char** args){
	char** cmd1, cmd2, cmd3; 
	int i, j; 	
	int pipeIndex1 = 0;
	int pipeIndex2 = 0; 
	int numArgs = 0; 	
	for(i = 0; args[i] != NULL; i++){
		numArgs++; 
		if(strcmp(args[i], "|") == 0){
			if(pipeIndex1 == 0){
				pipeIndex1 = i;
			}else{
				pipeIndex2 = i;
			}	
		}	
	}

        cmd1 = malloc(numArgs * sizeof(char*));	
	cmd2 = malloc(numArgs * sizeof(char*)); 
	cmd3 = malloc(numArgs * sizeof(char*)); 

	//fill in arrays with the three commands
	for(i = 0; i < pipeIndex1; i++){
		cmd1[i] = args[i]; 
	}

	j = 0;
        
	//find what index pipes are at	
	for(i = pipeIndex1+1; i < pipeIndex2; i++){
		cmd2[j] = args[i];
		j++;
	}

	j = 0; 
	for(i = pipeIndex2 + 1; args[i] != NULL; i++){
		cmd3[j] = args[i]; 
		j++;
	}
	
	int status; 
	int fd[2];
	int fd2[2]; 
	if(pipe(fd) != 0) 
		perror("failed to create pipe"); 
	pid_t pid;
        pid = fork();	
	if(pid == 0){
		if(pipe(fd2) != 0)
		       perror("pipe2 failed"); 
		pid = fork();	
		if(pid == 0){
			//execute first command here
			dup2(fd2[1], 1); 
			close(fd2[0]); 
			close(fd2[1]);
		        exec_cmd(cmd1, numArgs); 	
			exit(1); 
		}else{
			//execute second command here
			waitpid(pid, &status, 0); 
			dup2(fd2[0], 0); 
			dup2(fd[1], 1); 
			close(fd2[0]); 
			close(fd2[1]);
			exec_cmd(cmd2, numArgs); 	
			exit(1);
		}
		exit(1); 
	}else{  
		pid = fork(); 
		if(pid == 0){
			//execute third command here
			dup2(fd[0], 0);
			close(fd[0]); 
			close(fd[1]); 
			exec_cmd(cmd3, numArgs);
		        exit(1); 	
		}else{
		close(fd[0]);  
		close(fd[1]); 
		waitpid(pid, &status, 0); 
		}
	}
	for (i = 0; cmd1[i] != NULL; i++){
		cmd1[i] = 0; 
	}
	for (i = 0; cmd2[i] != NULL; i++){
		cmd2[i] = 0; 
	}

	for (i = 0; cmd3[i] != NULL; i++){
		cmd3[i] = 0; 
	}

	free(cmd1); 
	free(cmd2); 
	free(cmd3); 
}
