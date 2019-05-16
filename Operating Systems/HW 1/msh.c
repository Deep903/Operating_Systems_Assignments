/*

	Name: Deep Patel
	ID: 1001354110

*/

#define _GNU_SOURCE

#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <signal.h>

#define WHITESPACE " \t\n"      // We want to split our command line up into tokens
                                // so we need to define what delimits our tokens.
                                // In this case  white space
                                // will separate the tokens on our command line

#define MAX_COMMAND_SIZE 255    // The maximum command-line size

#define MAX_NUM_ARGUMENTS 5     // Mav shell only supports five arguments


	static void handle_signal_shell (int sig) //Responds to ctrl+c or z from a keyboard. Does nothing to shell.
	{
	//Does nothing because we are in the shell
	}

	static void handle_signal_process (int sig) //Meant to respond to ctrl+c from keyboard. Stops process.
	{
	int pid = getpid();
	kill(pid, SIGSTOP);
	//fflush(NULL); //We are in a process. Stop the process.
	//exit(0);
	}
	

int main()
{

  char * cmd_str = (char*) malloc( MAX_COMMAND_SIZE );
  int history = -1;							//Used to remember where to store next history item
  int pid_history[15] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};  //Stores pid history
  char cmd_history[15][100];						//Stores cmd history 
  int count = 0;							//For looping history arrays
  int history_count = 0;						//Tracks how many cmds are in history
  while( 1 )
  {
	struct sigaction act; //create struct for when ctrl+c is typed
	memset (&act, '\0', sizeof(act)); //0 out act
	act.sa_handler = &handle_signal_shell; //set handler to use function handle_signal_shell
	
	if(sigaction(SIGINT, &act, NULL) <0) //Install signal handler and check the return value
	{
		perror("sigaction: ");
		return 1;
	}

	struct sigaction act2; //create struct for when ctrl+z is typed
	memset (&act2, '\0', sizeof(act2)); //0 out act
	act.sa_handler = &handle_signal_shell; //set handler to use function handle_signal_shell
	
	if(sigaction(SIGTSTP, &act, NULL) <0) //Install signal handler and check the return value
	{
		perror("sigaction: ");
		return 1;
	}



    // Print out the msh prompt
    printf ("msh> ");

    // Read the command from the commandline.  The
    // maximum command that will be read is MAX_COMMAND_SIZE
    // This while command will wait here until the user
    // inputs something since fgets returns NULL when there
    // is no input
    while( !fgets (cmd_str, MAX_COMMAND_SIZE, stdin) );

    /* Parse input */
    char *token[MAX_NUM_ARGUMENTS];

    int   token_count = 0;                                 
                                                           
    // Pointer to point to the token
    // parsed by strsep
    char *arg_ptr;                                         
                                                           
    char *working_str  = strdup( cmd_str );                

    // we are going to move the working_str pointer so
    // keep track of its original value so we can deallocate
    // the correct amount at the end
    char *working_root = working_str;

    // Tokenize the input stringswith whitespace used as the delimiter
    while ( ( (arg_ptr = strsep(&working_str, WHITESPACE ) ) != NULL) && 
              (token_count<MAX_NUM_ARGUMENTS))
    {
      token[token_count] = strndup( arg_ptr, MAX_COMMAND_SIZE );
      if( strlen( token[token_count] ) == 0 )
      {
        token[token_count] = NULL;
      }
        token_count++;
    }
	


	//Read token 0 for commands
	if(token[0] == NULL) //User entered a space so we ignore
		{
		
		} 
	else if(strcmp(token[0], "quit") == 0 || strcmp(token[0], "exit") == 0) //User entered exit command
		{
			exit(0);
		}


	else if(strcmp(token[0], "cd") == 0)  //Command is change directory command
		{
			chdir(token[1]);      //Change directory to what user specified
		}

	else
	{
		pid_t child_pid = fork();    //Fork to try and run command
		int status;

	if(child_pid == 0) //This means were in the child process
		{

			act.sa_handler = &handle_signal_process; //set handler to use function handle_signal_process
	
		if(sigaction(SIGINT, &act, NULL) <0) //Install the handler and check the return value
			{
			perror("sigaction: ");
			return 1;
			}


        if(strcmp(token[0], "cd") == 0)  //Command is change directory command
			{	
			fflush(NULL);	//We do not need the child process to perform anything
			exit(0); //exit to avoid fork bombs or zombies
			} 


		char pathmax[255];				//Prepare parameters for execl current dir
		char *path = getcwd(pathmax, sizeof(pathmax));
		char *cmd;
		cmd = (char*) malloc(255);
		strcpy(cmd,path);
		strcat(cmd, token[0]);


		if(execv(cmd, token) != -1) //Command is in current directory
			{
			execv(cmd, token);
			fflush(NULL);
			exit(0); //exit to avoid fork bombs or zombies
			}

		path = "/usr/local/bin/";	       //Prepare parameters for excel in usr/local/bin
		free(cmd);
		cmd = (char*) malloc(255);
		strcpy(cmd,path);
		strcat(cmd, token[0]);

		if(execv(cmd, token) != -1) //Command is in usr local bin directory
			{
			execv(cmd, token);
			fflush(NULL);
			exit(0); //exit to avoid fork bombs or zombies
			}

		path = "/usr/bin/";		      //Prepare parameters for usr/bin
		free(cmd);
		cmd = (char*) malloc(255);
		strcpy(cmd,path);
		strcat(cmd, token[0]);

		if(execv(cmd, token) != -1) //Command is in usr bin directory
			{
			execv(cmd, token);
			fflush(NULL);
			exit(0); //exit to avoid fork bombs or zombies
			}


		path = "/bin/";		      //Prepare parameters for bin
		free(cmd);
		cmd = (char*) malloc(255);
		strcpy(cmd,path);
		strcat(cmd, token[0]);

		if(execv(cmd, token) != -1) //Command is in bin directory
			{
			execv(cmd, token);
			fflush(NULL);
			exit(0); //exit to avoid fork bombs or zombies
			}
		
		if(strcmp(token[0], "bg") == 0)  //Command is bg command, ignore, main handles
			{
			fflush(NULL);
			exit(0); //exit to avoid fork bombs or zombies
			}

		if(strcmp(token[0], "listpids") == 0) //Command is listpids, ignore, main handles
			{
			fflush(NULL);
			exit(0); //exit to avoid fork bombs or zombies
			}

		if(strcmp(token[0], "history") == 0) //Command is history, ignore, main handles
			{
			fflush(NULL);
			exit(0); //exit to avoid fork bombs or zombies
			}


		printf("%s: Command not found.\n", token[0]); //User entered an unsuppored command
		fflush(NULL);
		exit(0); //exit to avoid fork bombs or zombies

		} 
	pid_t last_pid;
	if(strcmp(token[0], "bg") == 0)		//If the token is bg, background the last pid.
		kill(last_pid, SIGCONT);	//Continue the suspended process

	last_pid = child_pid;			//Save the last pid

	history = history + 1;			//Move cmd and pid history up 1.
						//The following saves the pids for listpids
	if(history == 15)			//If array hits 15 reset it to 0.
	{					//This is so we can overwrite old pids/cmds (15 stored)
		history = 0;			
	}


	pid_history[history] = child_pid;	//Actually saves the child pid to the pid_history array
	strcpy(cmd_history[history], token[0]); 	

	if(strcmp(token[0], "listpids") == 0)		//If the token is listpids, list last 15 pids 
	{
		count = 0;
		while(count < 15)		//Iterate through the pid_history array
		{	
			if(pid_history[count] != 0)  	//Only print if a valid pid is stored
			printf("%d: %d\n", count, pid_history[count]);
			count++;	
		}
	}

	history_count++;			//History_count counts to 15 and stays at 15 onces there
	if(history_count >= 15)			//This is so my cmd_history array only prints valid history
	{
	   history_count = 15;
	}
	if(strcmp(token[0], "history") == 0)
	{
		count = 0;
		while(count < history_count)
		{
			if(cmd_history[count] != NULL)
			printf("%d: %s\n", count, cmd_history[count]);
			count++;
		}
	}





		waitpid(child_pid, &status, 0); //If it is, then background the process and keep going.
	}


    free( working_root );

  }
  return 0;
}