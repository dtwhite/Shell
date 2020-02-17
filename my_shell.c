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

int grabCommand(char **tokens, int basePointer){
	int counter = basePointer;
	int i;
	for(i=0;tokens[i]!=NULL;i++){
		printf("found token %s (remove this debug output later)\n", tokens[i]);
	}
	while(tokens[counter] != NULL && strcmp(tokens[counter], "&&") != 0){
		counter++;
	}
	return counter;
	/*while(*tokens != NULL && strcmp(tokens[counter], "&&") != 0){
		counter++;
	}
	printf("The counter is %d", counter);
	tokens[counter] = NULL;*/
	return counter;
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
			int futurePointer = grabCommand(tokens, basePointer);
			printf("The future pointer is %d\n", futurePointer);

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
					execvp(tokens[basePointer], tokens);
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
			}
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
