// 4348.003 - Project 1
// Tristen Even (tge160130) & Daniel ....

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include <fcntl.h>

// global variables
void interactive_mode();
void batch_mode(char*);
void printErrorMessage();
char *saveptr_zero = NULL, *saveptr_one = NULL, *saveptr_two = NULL, *saveptr_three = NULL; 
char delim[] = " \t\r\n\v\f";
char *readOutSymbol = ">";
char *ampersandSymbol = "&\n";
char slashSymbol[] = "/";
char error_message[25] = "Something went wrong...\n";

// function to print error
void printErrorMessage()
{	
	write(STDERR_FILENO, error_message, strlen(error_message));
}
// function to read input
char *reader()
{
	char *inputString = NULL;
	size_t buff_size = 0;
	int result;
	result = getline(&inputString, &buff_size, stdin);
	if (result == -1)
	{
		printErrorMessage();
		exit(1);
	}
	return inputString;
}
// main function decides which mode
int main(int argc, char **argv)
{	
	if(argc > 2) 
	{
		printErrorMessage();
		exit(1);
	}
	if(argc == 1) 
		interactive_mode();
	if(argc == 2) 
		batch_mode(argv[1]);
	
	return 0;
}
int contains(char *string1, char *string2)
{
	if (strstr(string1, string2) != NULL)
		return 1;
	else
		return 0;
	
}

// interactive_mode mode
void interactive_mode()
{
	// allocate memory, we have to cast it as a char pointer because if we 
	// dont than the compiler has warnings. malloc is a function that creates 
	// memory and parameter it takes is how big the space is to be	
	char **pathArray = malloc(512);
	pathArray[0] = "/bin";
	pathArray[1] = NULL;
	// true variable
	int loop = 1;
	while(loop)
    {
		printf("dash> ");
		fflush(stdout);
		// strtok takes in a string and a delimeter, the delimeter is what seperates the strings
		// It returns a token, the first param is the string to be broken up into tokens
		// The second parameter is what is seperating each token
		char *inputString = reader();
		char *ampersandSymbolc = strtok_r(inputString, ampersandSymbol, &saveptr_zero);
		while(ampersandSymbolc != NULL)
		{
			char *command = malloc(strlen(ampersandSymbolc) + 1);
			strcpy(command, ampersandSymbolc);
			// check for '>'
			if(contains(command, readOutSymbol) == 1)
			{
				char *tokenRd = strtok_r(command, readOutSymbol, &saveptr_one);
				if(contains(saveptr_one, readOutSymbol) == 1)
				{
					printErrorMessage();
					ampersandSymbolc = strtok_r(NULL, ampersandSymbol, &saveptr_zero);
					continue;
				}
				char *remainder = malloc(strlen(tokenRd) + 1);
				strcpy(remainder, tokenRd);

				char *temp = malloc(strlen(remainder) + 1);
				strcpy(temp, command);

				int num_commands = 0;
				char *token = strtok_r(temp, delim, &saveptr_two);
				if(token == NULL)
				{
					ampersandSymbolc = strtok_r(NULL, ampersandSymbol, &saveptr_zero);
					continue;
				}
				while((token = strtok_r(NULL, delim, &saveptr_two)) != NULL)
					num_commands++;
				// appends the string pointed to by the first parameter, it appends
				// whatever is in the second paramter
				tokenRd = strtok_r(NULL, strcat(delim, readOutSymbol), &saveptr_one);
				if(tokenRd == NULL)
				{
					printErrorMessage();
					ampersandSymbolc = strtok_r(NULL, ampersandSymbol, &saveptr_zero);
					continue;
				}
				char *output = malloc(strlen(tokenRd) + 1);
				strcpy(output, tokenRd);
				
				tokenRd = strtok_r(NULL, strcat(delim, readOutSymbol), &saveptr_one);
				if(tokenRd != NULL)
				{
					printErrorMessage();
					ampersandSymbolc = strtok_r(NULL, ampersandSymbol, &saveptr_zero);
					continue;
				}
				
				// set output for redirection
				// printf("FileNameOut = %s\n", fileNameOut);
				// printf("FileNameError = %s\n", fileNameError);
				
				// opens a file, if it successfully open/create file than it will return 0 otherwise -1
				// CREATE - creates the file, 0_TRUNC - overrides the file
				// RDWR - open forread and write to file, IRUSR - read permission, 
				// IWUSR - write permission
				int out = open(output, O_TRUNC|O_CREAT|O_WRONLY, 0600);
				if (out == -1)
					printErrorMessage();

				// catch errors and close output
				int save_out = dup(fileno(stdout));
				int save_err = dup(fileno(stderr));
				if (dup2(out, fileno(stdout)) == -1 || dup2(out, fileno(stderr)) == -1)
					printErrorMessage();

				fflush(stdout);
				fflush(stderr);
				close(out);

				// check for builtin commands
				token = strtok_r(remainder, delim, &saveptr_one);
				if (strcmp(token,"exit") == 0)
				{
					if(num_commands != 0)
					{
						printErrorMessage();
						ampersandSymbolc = strtok_r(NULL, ampersandSymbol, &saveptr_zero);
						continue;
					}
					else
						exit(0);
				}
				else if (strcmp(token,"cd") == 0)
				{
					if(num_commands != 1)
					{
						printErrorMessage();
						ampersandSymbolc = strtok_r(NULL, ampersandSymbol, &saveptr_zero);
						continue;
					}
					else
					{
						token = strtok_r(NULL, delim, &saveptr_one);
						if(token != NULL)
							chdir(token);
					}
				}		
				else if (strcmp(token,"path") == 0)
				{
					pathArray = malloc(8 * (num_commands + 1));
					int i = 0;
					token = strtok_r(NULL, delim, &saveptr_one);
					while(token != NULL)
					{
						pathArray[i] = malloc(strlen(token) + 1);
						strcpy(pathArray[i], token);
						i++;
					}
					pathArray[i] = NULL;
				}
				else
				{
					// execute command
					int i = 0;
					int checkPath;
					while(pathArray[i] != NULL)
					{
						checkPath = 0;
						char *pathBin;
						pathBin = malloc(strlen(token) + strlen(pathArray[i]) + 2);
						strcpy(pathBin, pathArray[i]);
						// appends the string pointed to by the first parameter, it appends
						// whatever is in the second paramter
						strcat(pathBin, slashSymbol);
						strcat(pathBin, token);
						if(access(pathBin, X_OK) == 0)
							checkPath = 1;
						else
						{
							i++;
							continue;
						}

						int status;
						char *args[num_commands + 2];
						args[0] = malloc(strlen(token) + 1);
						strcpy(args[0], token);
						int j = 1;
						while((token = strtok_r(NULL, delim, &saveptr_one)) != NULL)
						{
							args[j] = malloc(1+strlen(token));
							strcpy(args[j], token);
							j++;
						}
						args[j] = NULL;
						// returns a -1 if fork failed, 0 if on child process and 1 if on parent process
						// when a child process is created it basically means that it is a copy of its parents
						if(fork() == 0)
						{
							execv(pathBin, args);
							printErrorMessage();
						}
						else
							wait(&status);
						dup2(save_out, fileno(stdout));
						dup2(save_err, fileno(stderr));

						close(save_out);
						close(save_err);
						i++;
						if(checkPath == 1) 
							break;
					}
					// inconsistent path, return error
					if(checkPath == 0)
					{
						printErrorMessage();
						ampersandSymbolc = strtok_r(NULL, ampersandSymbol, &saveptr_zero);
						continue;
					}
				}
			}
			else
			{
				// Runs the other commands if it is in the path
				// Parameters which is the command the user types in
				char *p1, *p2;
				char *temp = malloc(strlen(command) + 1);
				strcpy(temp, command);
				int num_commands = 0;
				char *token = strtok_r(temp, delim, &p2);
				if(token == NULL)
				{
					ampersandSymbolc = strtok_r(NULL, ampersandSymbol, &saveptr_zero);
					continue;
				}
				while((token = strtok_r(NULL, delim, &p2)) != NULL)
					num_commands++;
				token = strtok_r(command, delim, &p1);
				// builtin commands
				if (strcmp(token,"exit") == 0)
				{
					if(num_commands != 0)
					{
						printErrorMessage();
					}
					else
						exit(0);
				}
				else if (strcmp(token,"cd") == 0)
				{
					if(num_commands != 1)
					{
						printErrorMessage();
						ampersandSymbolc = strtok_r(NULL, ampersandSymbol, &saveptr_zero);
						continue;
					}
					else
					{
						token = strtok_r(NULL, delim, &p1);
						if(token != NULL)
							chdir(token);
					}
				}
				else if (strcmp(token,"path") == 0)
				{
					pathArray = malloc(sizeof(char *)*(num_commands + 1));
					int j = 0;
					while((token = strtok_r(NULL, delim, &p1)) != NULL)
					{
						pathArray[j] = malloc(strlen(token) + 1);
						strcpy(pathArray[j], token);
						j++;
					}
					pathArray[j] = NULL;
				}
				else
				{
					int i = 0;
					int checkPath = 0;
					while(pathArray[i] != NULL)
					{
						checkPath = 0;
						char* pathBin;
						pathBin = malloc(strlen(token) + strlen(pathArray[i]) + 2);
						strcpy(pathBin, pathArray[i]);
						strcat(pathBin, slashSymbol);
						strcat(pathBin, token);
						if(access(pathBin, X_OK) == 0)
							checkPath = 1;
						else
						{
							i++;
							continue;
						}
						
						int status;
						char *args[num_commands + 2];
						args[0] = malloc(strlen(token) + 1);
						strcpy(args[0], token);
						int j = 1;
						while((token = strtok_r(NULL, delim, &p1)) != NULL)
						{
							args[j] = malloc(strlen(token) + 1);
							strcpy(args[j], token);
							j++;
						}
						args[j] = NULL;
						// returns a -1 if fork failed, 0 if on child process and 1 if on parent process
						// when a child process is created it basically means that it is a copy of its parents
						if(fork() == 0)
						{
							execv(pathBin, args);
							printErrorMessage();
						}
						else
							wait(&status);
						i++;
						if(checkPath == 1) 
							break;
					}
					if(checkPath == 0)
					{
					  printErrorMessage();
					}
				}
			}
			ampersandSymbolc = strtok_r(NULL, ampersandSymbol, &saveptr_zero);
		}
    }
}
// batch mode for files
void batch_mode(char *file)
{
		//allocate memory for sizeof char*
		char **pathArray = malloc(512);
		pathArray[0] = "/bin";
		pathArray[1] = NULL;
		FILE *fptr;
		fptr = fopen(file, "r");
		if(fptr == NULL)
		{
			printErrorMessage();
			exit(1);
		}
		char *inputString = NULL;
		size_t buff_size = 0;
		int result;
		result = getline(&inputString, &buff_size, fptr);
		while(result != -1)
		{
			if(inputString[0] == '\n') 
				continue;
			char *ampersandSymbolc = strtok_r(inputString, ampersandSymbol, &saveptr_zero);
			while(ampersandSymbolc != NULL)
			{
				char *command = malloc(strlen(ampersandSymbolc) + 1);
				strcpy(command, ampersandSymbolc);
				if(contains(command, readOutSymbol) == 1)
				{
					char *tokenRD = strtok_r(command, readOutSymbol, &saveptr_one);
					char *remainder = malloc(strlen(tokenRD) + 1);
					strcpy(remainder, tokenRD);
					char *temp = malloc(strlen(remainder) + 1);
					strcpy(temp, remainder);
					
					

					if(contains(saveptr_one, readOutSymbol) == 1)
					{
						printErrorMessage();
						ampersandSymbolc = strtok_r(NULL, ampersandSymbol, &saveptr_zero);
						continue;
					}
					tokenRD = strtok_r(NULL, strcat(delim, readOutSymbol), &saveptr_one);
					if(tokenRD == NULL)
					{
						printErrorMessage();
						ampersandSymbolc = strtok_r(NULL, ampersandSymbol, &saveptr_zero);
						continue;
					}

					char *output = malloc(1+strlen(tokenRD));
					strcpy(output, tokenRD);

					if((tokenRD = strtok_r(NULL, strcat(delim, readOutSymbol), &saveptr_one)) != NULL)
					{
						printErrorMessage();
						ampersandSymbolc = strtok_r(NULL, ampersandSymbol, &saveptr_zero);
						continue;
					}

					int num_commands = 0;
					char *token = strtok_r(remainder, delim, &saveptr_three);
					if(token == NULL)
					{
						ampersandSymbolc = strtok_r(NULL, ampersandSymbol, &saveptr_zero);
						continue;
					}
					while((token = strtok_r(NULL, delim, &saveptr_three)) != NULL)
						num_commands++;
					
					//printf("FileNameOut = %s\n", fileNameOut);
					//printf("FileNameError = %s\n", fileNameError);
					
					// opens a file, if it successfully open/create file than it will return 0 otherwise -1
					// CREATE - creates the file, 0_TRUNC - overrides the file
					// RDWR - open forread and write to file, IRUSR - read permission, 
					// IWUSR - write permission
					int out = open(output, O_TRUNC|O_CREAT|O_WRONLY, 0600);
					if (out == -1)
					{
						printErrorMessage();
						ampersandSymbolc = strtok_r(NULL, ampersandSymbol, &saveptr_zero);
						continue;
					}

					int save_out = dup(fileno(stdout));
					int save_err = dup(fileno(stderr));
					if (dup2(out, fileno(stdout)) == -1)
					{	
						printErrorMessage();
						ampersandSymbolc = strtok_r(NULL, ampersandSymbol, &saveptr_zero);
						continue;
					}
					if (dup2(out, fileno(stderr)) == -1)
					{
						printErrorMessage();
						ampersandSymbolc = strtok_r(NULL, ampersandSymbol, &saveptr_zero);
						continue;
					}

					fflush(stdout);
					fflush(stderr);
					close(out);

					token = strtok_r(temp, delim, &saveptr_two);
					if (strcmp(token,"exit") == 0)
					{
						if( num_commands != 0 )
						{
							printErrorMessage();
							ampersandSymbolc = strtok_r(NULL, ampersandSymbol, &saveptr_zero);
							continue;
						}
						else
						exit(0);
					}
					else if (strcmp(token,"cd") == 0)
					{
						//changeDirectory(*token, ampersandSymbol, delim, saveptr_zero, saveptr_two, num_commands);
						if(num_commands != 1)
						{
							printErrorMessage();
							ampersandSymbolc = strtok_r(NULL, ampersandSymbol, &saveptr_zero);
							continue;
						}
						else
						{
							token = strtok_r(NULL, delim, &saveptr_two);
							chdir(token);
						}
					}
					else if (strcmp(token,"path") == 0)
					{
						pathArray = malloc(sizeof(char *) * (num_commands + 1));
						int i = 0;
						while((token = strtok_r(NULL, delim, &saveptr_two)) != NULL)
						{
							pathArray[i] = malloc(strlen(token) + 1);
							strcpy(pathArray[i], token);
							i++;
						}
						pathArray[i] = NULL;
					}
					else
					{
						int i = 0;
						int checkPath = 0;
						while(pathArray[i] != NULL)
						{
							checkPath = 0;
							char *pathBin;
							pathBin = malloc(strlen(pathArray[i]) + strlen(token) + 2);
							strcpy(pathBin, pathArray[i]);
							strcat(pathBin, slashSymbol);
							strcat(pathBin, token);
							if(access(pathBin, X_OK) == 0)
								checkPath = 1;
							else
							{
								i++;
								continue;
							}

							int status;
							char *args[num_commands + 2];
							args[0] = malloc(strlen(token) + 1);
							strcpy(args[0], token);
							int j = 1;
							while((token = strtok_r(NULL, delim, &saveptr_two)) != NULL)
							{
								args[j] = malloc(strlen(token) + 1);
								strcpy(args[j], token);
								j++;
							}
							args[j] = NULL;
							// returns a -1 if fork failed, 0 if on child process and 1 if on parent process
							// when a child process is created it basically means that it is a copy of its parents
							if(fork() == 0)
							{
								execv(pathBin, args);
								printErrorMessage();
							}
							else
								wait(&status);
							dup2(save_out, fileno(stdout));
							dup2(save_err, fileno(stderr));

							close(save_out);
							close(save_err);
							i++;
							if(checkPath == 1) 
								break;
						}
						if(checkPath == 0)
						{
							printErrorMessage();
						}
					}
				}
				else
				{
					char *temp = malloc(strlen(command) + 1);
					strcpy(temp, command);
					int num_commands = 0;
					char *token = strtok_r(command, delim, &saveptr_two);
					if(token == NULL)
					{
						ampersandSymbolc = strtok_r(NULL, ampersandSymbol, &saveptr_zero);
						continue;
					}
					while((token = strtok_r(NULL, delim, &saveptr_two)) != NULL)
						num_commands++;
					token = strtok_r(temp, delim, &saveptr_three);
					if (strcmp(token,"exit") == 0)
					{
						if(num_commands != 0)
						{
							printErrorMessage();
						}
						else
							exit(0);
					}
						else if (strcmp(token,"cd") == 0)
						{
							if(num_commands != 1)
							{
								printErrorMessage();
							}
							token = strtok_r(NULL, delim, &saveptr_three);
							if(token != NULL)
								chdir(token);
						}
					else if (strcmp(token,"path") == 0)
					{
						pathArray = malloc(sizeof(char *) * (num_commands + 1));
						int i = 0;
						while((token = strtok_r(NULL, delim, &saveptr_three)) != NULL)
						{
							pathArray[i] = malloc(strlen(token) + 1);
							strcpy(pathArray[i], token);
							i++;
						}
						pathArray[i] = NULL;
					}
					else
					{
						int i = 0;
						int checkPath = 0;
						while(pathArray[i] != NULL)
						{
							checkPath = 0;
							char* pathBin;
							pathBin = malloc(strlen(token) + strlen(pathArray[i]) + 2);
							strcpy(pathBin, pathArray[i]);
							strcat(pathBin, slashSymbol);
							strcat(pathBin, token);
							if(access(pathBin, X_OK) == 0)
								checkPath = 1;
							else
							{
								i++;
								continue;
							}

							int status;
							char *args[num_commands + 2];
							args[0] = malloc(strlen(token) + 1);
							strcpy(args[0], token);
							int j = 1;
							while((token = strtok_r(NULL, delim, &saveptr_three)) != NULL)
							{
								args[j] = malloc(strlen(token) + 1);
								strcpy(args[j], token);
								j++;
							}
							args[j] = NULL;
							if(fork() == 0)
							{
								execv(pathBin, args);
								printErrorMessage();
							}
							else
								wait(&status);
							i++;
							if(checkPath == 1) break;
						}
						if(checkPath == 0)
						{
							printErrorMessage();
						}
					}
				}
				ampersandSymbolc = strtok_r(NULL, ampersandSymbol, &saveptr_zero);
			}
		}
}