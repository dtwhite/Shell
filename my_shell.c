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
	int counter = 0;
	while(command[counter] != NULL){
		if(strcmp(command[counter], "&&&") == 0)
			return true;
	}
	return false;
}

bool isBackgroundCommand(char **command){
	if(strlen(line) != 0 && line[strlen(line) - 1] == '&')
		return true;
	else
		return false;
}

int grabCommand(char **tokens, int basePointer, char **delimiter){
	int counter = basePointer;
	int i;
	while(tokens[counter] != NULL && strcmp(tokens[counter], *delimiter) != 0){
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
		bool background = false;

		if(strlen(line) != 0 && line[strlen(line) - 1] == '&'){
			background = true;
			line[strlen(line) -1] = '\n';
		}
		else{
			line[strlen(line)] = '\n';
		}
		tokens = tokenize(line);

		if(*tokens == NULL){
			continue;
		}
		int basePointer = 0;
		while(tokens[basePointer] != NULL){
			char delim[]= "&&";
			int futurePointer = grabCommand(tokens, basePointer, &delim);
			char **command = copyTokens(tokens, basePointer, futurePointer);
			if(tokens[futurePointer] != NULL && strcmp(tokens[futurePointer], "&&") == 0)
				futurePointer++;
			if(strcmp(tokens[basePointer], "cd") == 0){
				int result = chdir(tokens[basePointer+1]);
				if(result == -1)
					perror("Shell");
			}
			else{
				int retval = fork();
				if(retval == 0){
					if(isParallelCommand(command)){
						char delimiter[] = "&&&";
						int indexPointer = 0;
						while(command[indexPointer] != NULL){
							int newIndex = grabCommand(command, indexPointer, &delimiter);
							char **copyCommand = copyTokens(command, index, newIndex);
							if(command[newIndex] != NULL && strcmp(command[newIndex], delimiter) == 0)
								newIndex++;
							executeShellBuiltin(copyCommand);
							indexPointer = newIndex;
							if()
						}
					}
					else if(isBackgroundCommand(command)){
						background = true;
						executeShellBuiltin(command);
					}
					else{
						executeShellBuiltin(command);
					}
				}
				else{
					int status;
					int pid = retval;
					if(!background || !lastParallel){
						waitpid(pid, &status, 0);
					}	
				}
			}
			for(i=0;command[i]!=NULL;i++){
				free(command[i]);
			}
			free(command);
			basePointer = futurePointer;	
				
		}
		/*else if(strcmp(tokens[0], "cd")==0){
			int result = chdir(tokens[1]);
			if(result == -1)
				perror("Shell");
		}
		else{
		//do whatever you want with the commands, here we just print them
			int retval = fork();
			if(retval == 0){
				execvp(tokens[0], tokens);
				printf("Shell: Incorrect Command\n");
				exit(1);
			}
			else{
				int status;
				int pid = retval;
				if(!background){
					waitpid(pid, &status, 0);
				}
				
			}
			/*for(i=0;tokens[i]!=NULL;i++){
				printf("found token %s (remove this debug output later)\n", tokens[i]);
			}
		}*/
		// Freeing the allocated memory	
		for(i=0;tokens[i]!=NULL;i++){
			free(tokens[i]);
		}
		free(tokens);
	}
	return 0;
}
