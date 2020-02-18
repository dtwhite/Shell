int retval = fork();
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