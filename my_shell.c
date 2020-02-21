#include  <stdio.h>
#include  <sys/types.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdbool.h>
#include "uthash.h"
#include <signal.h>

#define MAX_INPUT_SIZE 1024
#define MAX_TOKEN_SIZE 64
#define MAX_NUM_TOKENS 64
#define FOREGROUND_PROCESS_GROUP_ID 10
#define BACKGROUND_PROCESS_GROUP_ID 20
#define PARENT_PROCESS_GROUP_ID 1

/**
 * This is a hashtable that keeps track of the processs
 * running in the background. The implementation of 
 * the hashtable was done by the University of Texas which
 * was imported from the uthash package.
 * */
struct process {
	int pid; // the key for the hashtable.
	int value; 
	UT_hash_handle hh;
};

struct process *processTable; //Global processTable

/**
 * This method adds the process to the global hashtable.
*/
void addProcessToTable(int pid){
	struct process *proc;

    proc = malloc(sizeof(struct process));
    proc->pid = pid;
    proc->value = 0;
    HASH_ADD_INT( processTable, pid, proc );  /* id: name of key field */
}
/**
 * This method removes the background process from 
 * the hashtable.
*/
void removeProcessFromTable(struct process *proc){
	HASH_DEL(processTable, proc);
	free(proc);
}

/**
 * This method iterates through the process hashtable and checks 
 * and clears away any dead processes.
 * */
void clearProcessTable(){
	struct process *proc, *tmp;
	int status;
	HASH_ITER(hh, processTable, proc, tmp){
		if(waitpid(proc->pid, &status, WNOHANG)> 0){
			printf("Shell:  Background process finished\n");
    		removeProcessFromTable(proc);
    	}
	}
}
/**
 * This method kills and reaps all the processes out of
 * the process hashtable.
*/
void forceClearProcessTable(){
	struct process *proc, *tmp;
	int status;
	HASH_ITER(hh, processTable, proc, tmp){
		kill(proc->pid, SIGKILL);
		waitpid(proc->pid, &status, 0);
		removeProcessFromTable(proc);
	}
}
/**
 * This is the Ctrl-C signal handler that kills the foreground
 * processes. 
*/
void ctrlCHandler(int sig_num) 
{ 	
    printf("\n");
} 

/* Splits the string by space and returns the array of tokens
*
*/
char **tokenize(char *line)
{
  char **tokens = (char **)malloc(MAX_NUM_TOKENS * sizeof(char *));
  char *token = (char *)malloc(MAX_TOKEN_SIZE * sizeof(char));
  int i, tokenIndex = 0, tokenNo = 0;

  for(i =0; i < strlen(line); i++){

    char readChar = line[i];

    if (readChar == ' ' || readChar == '\n' || readChar == '\t'){
      token[tokenIndex] = '\0';
      if (tokenIndex != 0){
		tokens[tokenNo] = (char*)malloc(MAX_TOKEN_SIZE*sizeof(char));
		strcpy(tokens[tokenNo++], token);
		tokenIndex = 0; 
      }
    }
	else {
      token[tokenIndex++] = readChar;
    }
  }
 
  free(token);
  tokens[tokenNo] = NULL ;
  return tokens;
}
/** 
 * This method checks to see if the given command is to be 
 * run in parallel mode. It achieves this by seeing if the 
 * command contains a '&&&' token.
*/
bool isParallelCommand(char **command){
	int i;
	for(i = 0; command[i] != NULL; i++){
		if(strcmp(command[i], "&&&") == 0)
			return true;
	}
	return false;
}
/**
 * This method checks to see if the given command is to
 * be run in background mode. It achieves this by seeing 
 * if the command contains a '&' token.
*/
bool isBackgroundCommand(char **command, int lastTokenIndex){
	if(strcmp(command[lastTokenIndex], "&") == 0)
		return true;
	else
		return false;
}
/**
 * This method iterates over the given the command and returns 
 * a index pointing to the end of the command. 
*/
int grabCommand(char **tokens, int basePointer, char *delimiter){
	int counter = basePointer;
	int i;
	while(tokens[counter] != NULL && strcmp(tokens[counter], delimiter) != 0){
		counter++;
	}
	return counter;
}
/**
 * This method creates an array of tokens for the given command.
*/
char **copyTokens(char **tokens, int basePointer, int endPointer){
	int i;
	char **command = (char **)malloc(MAX_NUM_TOKENS * sizeof(char *));
	int counter = 0;
	while(basePointer < endPointer){
		command[counter] = (char*)malloc(MAX_TOKEN_SIZE*sizeof(char));
		strcpy(command[counter], tokens[basePointer]);
		basePointer++;
		counter++;
	}
	command[counter] = NULL;
	return command;
}
/**
 * This method executes the given command.
*/
void executeShellBuiltin(char** command){
	execvp(command[0], command);
	printf("Shell: Incorrect command\n");
	exit(1);
}
/**
 * This method executes the cd command.
*/
bool cdCommand(char **command, int index){
	if(strcmp(command[index], "cd") == 0){ // This checks if the given command is cd and makes the chdir system call.
		int result = chdir(command[index+1]);
		if(result == -1){
			printf("Shell: Incorrect command\n");
		}
		return true;
	}
	else 
		return false;
}

int main(int argc, char* argv[]) {
	char  line[MAX_INPUT_SIZE];            
	char  **tokens;              
	int i;
	processTable = NULL;
	FILE* fp;
	if(argc == 2) {
		fp = fopen(argv[1],"r");
		if(fp < 0) {
			printf("File doesn't exists.");
			return -1;
		}
	}
	signal(SIGINT, ctrlCHandler); 
	while(1) {	
		int cstatus;		
		/* BEGIN: TAKING INPUT */
		bzero(line, sizeof(line));
		if(argc == 2) { // batch mode
			if(fgets(line, sizeof(line), fp) == NULL) { // file reading finished
				break;	
			}
			line[strlen(line) - 1] = '\0';
		} else { // interactive mode
			printf("$ ");
			scanf("%[^\n]", line);
			getchar();
		}
		//printf("Command entered: %s (remove this debug output later)\n", line);
		/* END: TAKING INPUT */

		line[strlen(line)] = '\n';
		tokens = tokenize(line);
		if(*tokens == NULL){
			continue;
		}
		int basePointer = 0;
		char delim[]= "&&";
		while(tokens[basePointer] != NULL){
			clearProcessTable(); // Clears all the dead background child processes.
			int futurePointer = grabCommand(tokens, basePointer, delim);
			char **command = copyTokens(tokens, basePointer, futurePointer);
			if(tokens[futurePointer] != NULL && strcmp(tokens[futurePointer], "&&") == 0)
				futurePointer++;
			int ds;
			for(ds = 0; command[ds] != NULL; ds++){} //Gets the last token of the command, which helps the isBackground method. 
			ds--;
			if(strcmp(command[0], "exit") == 0){ //Handles the exit command case.
				forceClearProcessTable();
				exit(0);
			}
			else if(isParallelCommand(command)){ //This block of code runs if the command is to be run in parallel.
				char delimiter[] = "&&&";
				int indexPointer = 0;
				bool lastCommand = false;
				
				while(command[indexPointer] != NULL){ // Loops through the parallel commands and executes them.
			        int newIndex = grabCommand(command, indexPointer, delimiter);
					char **copyCommand = copyTokens(command, indexPointer, newIndex);
					if(command[newIndex] != NULL && strcmp(command[newIndex], delimiter) == 0)
						newIndex++;

					if(cdCommand(command, indexPointer) == true){}
					else{
						if(command[newIndex] == NULL){
							lastCommand = true;
						}
						int retval = fork();
						if(retval == 0){
							executeShellBuiltin(copyCommand);
						}
						else{
							if(lastCommand){ // Waits for the last command to finish before moving on.
								int status;
								int pid = retval;
								waitpid(pid, &status, 0);
							}
							else{
								addProcessToTable(retval);
							}
						}
					}
					indexPointer = newIndex;
				}
			}
			else if(isBackgroundCommand(command, ds)){ // This block executes if the command is to be run in the background.
				bool background = true;
				int commandBoundaries = grabCommand(command, 0, "&");
				char **execCommand = copyTokens(command, 0, commandBoundaries); 
				if(cdCommand(command, 0) == true){ }
				else{
				    int retval = fork();
				    setpgid((pid_t)retval, (pid_t)retval);
				    if(retval == 0)
					    executeShellBuiltin(execCommand);
					else
						addProcessToTable(retval);
				 }
			}
			else{
				if(cdCommand(command, 0) == true){ }
				else{
				    int retval = fork();
				    if(retval == 0)
				    	executeShellBuiltin(command);
				    else{
						int pid = retval;
						waitpid(pid, &cstatus, WUNTRACED);
					}
				}
			}	
			
			for(i=0;command[i]!=NULL;i++){
				free(command[i]);
			}
			free(command);
			basePointer = futurePointer;
		}	
		// Freeing the allocated memory	
		for(i=0;tokens[i]!=NULL;i++){
			free(tokens[i]);
		}
		free(tokens);
	}
	return 0;
}




