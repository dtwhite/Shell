#include  <stdio.h>
#include  <sys/types.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdbool.h>

#define MAX_INPUT_SIZE 1024
#define MAX_TOKEN_SIZE 64
#define MAX_NUM_TOKENS 64

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

bool isBackgroundCommand(char **command, int lastTokenIndex){
	if(strcmp(command[lastTokenIndex], "&") == 0)
		return true;
	else
		return false;
}

int grabCommand(char **tokens, int basePointer, char *delimiter){
	int counter = basePointer;
	int i;
	while(tokens[counter] != NULL && strcmp(tokens[counter], delimiter) != 0){
		counter++;
	}
	return counter;
}

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

void executeShellBuiltin(char** command){
	execvp(command[0], command);
	printf("Shell: Incorrect Command\n");
	exit(1);
}

bool cdCommand(char **command, int index){
	if(strcmp(command[index], "cd") == 0){ // This checks if the given command is cd and makes the chdir system call.
		int result = chdir(command[index+1]);
		if(result == -1){
			perror("Shell");
		}
		return true;
	}
	else 
		return false;
}

int countNumberParallelCommands(char ** command){
	int counter = 0;
	int i;
	for(i = 0; command[i] != NULL; i++){
		if(strcmp(command[i], "&&&") == 0)
			counter++;	
	} 
	return counter;
}


void reapDeadChildren(int *pidArray, int length){
	int i;
	for(i = 0; i < length; i++){
		if(pidArray[i] != -1){
			int status;
			int result = waitpid(pidArray[i], &status, WNOHANG);
			if(result != 0){
				printf("process [%d] is done\n", pidArray[i]);
				pidArray[i] = -1;
			}
		}
	}
}

int main(int argc, char* argv[]) {
	char  line[MAX_INPUT_SIZE];            
	char  **tokens;              
	int i;
	FILE* fp;
	if(argc == 2) {
		fp = fopen(argv[1],"r");
		if(fp < 0) {
			printf("File doesn't exists.");
			return -1;
		}
	}
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
		while(tokens[basePointer] != NULL){
			//int *globalProcessTable = (int *)malloc(sizeof(int)*) 
			char delim[]= "&&";
			int futurePointer = grabCommand(tokens, basePointer, delim);
			char **command = copyTokens(tokens, basePointer, futurePointer);
			if(tokens[futurePointer] != NULL && strcmp(tokens[futurePointer], "&&") == 0)
				futurePointer++;
			int ds;
			for(ds = 0; command[ds] != NULL; ds++){}
			ds--;
			int numberOfParallelCommands = countNumberParallelCommands(command);
			if(isParallelCommand(command)){
				char delimiter[] = "&&&";
				int indexPointer = 0;
				bool lastCommand = false;
				int *pidArray = (int *)malloc(sizeof(int) * numberOfParallelCommands); 
				int pidArrayIndex= 0; 
				
				while(command[indexPointer] != NULL){
			        int newIndex = grabCommand(command, indexPointer, delimiter);
					char **copyCommand = copyTokens(command, indexPointer, newIndex);
					printf("The value of the newIndex is %d\n", newIndex);
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
							if(lastCommand){
								int v;
								for(v = 0; v < numberOfParallelCommands; v++){
									printf("The process id for all processes is %d\n", pidArray[v]);
								}
								int pid = retval; 
								wait(&pid);
							}
							else{
								pidArray[pidArrayIndex] = retval;
								pidArrayIndex++;
							}
							reapDeadChildren(pidArray, numberOfParallelCommands);
						}
					}
					indexPointer = newIndex;
				}
			}
			else if(isBackgroundCommand(command, ds)){
				bool background = true;
				int commandBoundaries = grabCommand(command, 0, "&");
				char **execCommand = copyTokens(command, 0, commandBoundaries); 
				int cs;
				for(cs = 0; execCommand[cs] != NULL; cs++){
					printf("The component of the command is %s\n", execCommand[cs]);
				}
				if(cdCommand(command, 0) == true){ }
				else{
				    int retval = fork();
				    if(retval == 0)
					    executeShellBuiltin(execCommand);
				    else{ 
				    	printf("Executing process [%d] in the background\n", retval);
				    }
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
		waitpid(-1, &cstatus, WNOHANG);	
		// Freeing the allocated memory	
		for(i=0;tokens[i]!=NULL;i++){
			free(tokens[i]);
		}
		free(tokens);
	}
	return 0;
}




