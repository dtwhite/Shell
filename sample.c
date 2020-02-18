bool cdCommand(char **command, int index){
	if(strcmp(command[index], "cd") == 0){ // This checks if the given command is cd and makes the chdir system call.
		int result = chdir(command[index+1]);
		if(result == -1)
			perror("Shell");
		}
		return true;
	}
	else 
		return false;
}

int countNumberParallelCommands(char ** command){
	int counter = 0; 
	int commandIndex = 0;
	while(command[commandIndex] != NULL){
		if(strcmp(comand[commandIndex], "&&&") == 0)
			counter++;
	}
	return counter;
}


void reapDeadChildren(int *pidArray, int length){
	int i;
	for(i = 0; i < length; i++){
		if(pidArray[i] != -1){
			int status
			int result = waitpid(pidArray[i], &status, WNOHANG);
			if(result != 0)
				pidArray[i] = -1;
		}
	}
}

int basePointer = 0;
while(tokens[basePointer] != NULL){
	char delim[]= "&&";
	int futurePointer = grabCommand(tokens, basePointer, &delim);
	char **command = copyTokens(tokens, basePointer, futurePointer);
	if(isParallelCommand(command)){
		char delimiter[] = "&&&";
		int indexPointer = 0;
		bool lastCommand = false;
		int *pidArray = (int *)malloc(sizeof(int) * countNumberParallelCommands(command)); 
		int pidArrayIndex= 0; 
		
		while(command[indexPointer] != NULL){
	        int newIndex = grabCommand(command, indexPointer, &delimiter);
			char **copyCommand = copyTokens(command, index, newIndex);
			if(command[newIndex] != NULL && strcmp(command[newIndex], delimiter) == 0)
				newIndex++;
			if(cdCommand(command, newIndex) == true)
				continue;
			else{
				if(command[newIndex] == NULL)
					lastCommand = true;
				int retval = fork();
				if(retval == 0){
					executeShellBuiltin(copyCommand);
				}
				else{
					if(lastCommand){
						int pid = retval; 
						wait(&pid);
					}
					else{
						pidArray[index] = retval;
						index++;
					}
					reapDeadChildren(pidArray);
				}
			}
			indexPointer = newIndex;
		}
	}
	else if(isBackgroundCommand(command)){
		background = true;
	    int retval = fork()
	    if(retval == 0)
		    executeShellBuiltin(command);
	    else{
	        
	    }
	}
	else{
	    int retval = fork();
	    if(retval == 0)
		    executeShellBuiltin(command);
	    else{
			int status;
			int pid = retval;
			if(!background || !lastParallel)
				waitpid(pid, &status, 0);
		}
	}	
}		
                
                
                
                
                
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