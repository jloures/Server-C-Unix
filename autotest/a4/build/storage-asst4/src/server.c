/**
 * @file
 * @brief This file implements the storage server.
 *
 * The storage server should be named "server" and should take a single
 * command line argument that refers to the configuration file.
 * 
 * The storage server should be able to communicate with the client
 * library functions declared in storage.h and implemented in storage.c.
 */
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <string.h>
#include <assert.h>
#include <signal.h>
#include "utils.h"
#include <time.h>
#include <stdbool.h>
#include <pthread.h>

#define LOGGING 0
#define MAX_LISTENQUEUELEN 20	///< The maximum number of queued connections.
struct set_params sparams;
struct query_params qparams;
struct config_params params;
table_list List;
FILE *file;
int *connfdp;
int handle_command(FILE *file, int sock, char *cmd,  table_list * List, struct config_params params, struct set_params sparams, struct query_params qparams);
/**
 * @brief Structure storing information for each thread
 * 
 */
struct _ThreadInfo { 
  struct sockaddr_in clientaddr;
  socklen_t clientaddrlen; 
  int clientsock; 
  pthread_t theThread; 
}; 
typedef struct _ThreadInfo *ThreadInfo;

/*  Thread buffer, and circular buffer fields */ 
ThreadInfo runtimeThreads[MAX_CONNECTIONS]; 
unsigned int botRT = 0, topRT = 0;

/* Mutex initializations*/ 
pthread_mutex_t  printMutex    = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t conditionMutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t handle_cmd_lock = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t  conditionCond  = PTHREAD_COND_INITIALIZER;

/**
 * @brief Function accessing threadinfo struct
 * 
 * @param void There're no parameter inputs
 * @ret Returns a pointer to the threadinfo struct
 */
ThreadInfo getThreadInfo(void) { 
  ThreadInfo currThreadInfo = NULL;

  /* Wait as long as there are no more threads in the buffer */ 
  pthread_mutex_lock( &conditionMutex ); 
  while (((botRT+1)%MAX_CONNECTIONS)==topRT)
    pthread_cond_wait(&conditionCond,&conditionMutex); 
  
  /* At this point, there is at least one thread available for us. We
     take it, and update the circular buffer  */ 
  currThreadInfo = runtimeThreads[botRT]; 
  botRT = (botRT + 1)%MAX_CONNECTIONS;

  /* Release the mutex, so other clients can add threads back */ 
  pthread_mutex_unlock( &conditionMutex ); 
  
  return currThreadInfo;
}

/**
 * @brief Function called when thread is about to finish -- unless it is
   called, the ThreadInfo assigned to it is lost
 * 
 * @param Threadinfo the pointer to the threadinfo struct is passed in
 * @ret void There's no return 
 */
void releaseThread(ThreadInfo me) {
  pthread_mutex_lock( &conditionMutex ); 
  assert( botRT!=topRT ); 
 
  runtimeThreads[topRT] = me; 
  topRT = (topRT + 1)%MAX_CONNECTIONS; 

  /* tell getThreadInfo a new thread is available */ 
  pthread_cond_signal( &conditionCond ); 

  /* Release the mutex, so other clients can get new threads */ 
  pthread_mutex_unlock( &conditionMutex ); 
}

void * threadCallFunction(void *arg) { 
	ThreadInfo tiInfo = (ThreadInfo)arg;
	int clientsock = tiInfo->clientsock;
	pthread_detach(pthread_self());
	free(arg);
	int wait_for_commands = 1;
	do {
		// Read a line from the client.
		char cmd[MAX_CMD_LEN];
		memset(cmd,0,MAX_CMD_LEN);
		int status = recvline(clientsock, cmd, MAX_CMD_LEN);
		if (status != 0) {
		// Either an error occurred or the client closed the connection.
		wait_for_commands = 0;
		} else {
			
			// Handle the command from the client.
			pthread_mutex_lock(& handle_cmd_lock);
			status = handle_command(file, clientsock, cmd, &List, params, sparams, qparams);
			pthread_mutex_unlock(& handle_cmd_lock);
			if (status != 0)
				wait_for_commands = 0; // Oops.  An error occured.
		}
	} while (wait_for_commands);

	close(tiInfo->clientsock);
	releaseThread( tiInfo ); 
	return NULL;
}


/**
 * @brief Process a command from the client.
 *
 * @param sock The socket connected to the client.
 * @param cmd The command received from the client.
 * @return Returns 0 on success, -1 otherwise.
 */
int handle_command(FILE *file, int sock, char *cmd,  table_list * List, struct config_params params, struct set_params sparams, struct query_params qparams)
{
	
		char output2[1024];
	char *command = strtok(cmd," ");

	if(command) {

		if(!strcmp(command,"AUTH")) {
			char *username = strtok(NULL," ");
			char *password = strtok(NULL,"\0");
			sprintf(output2,"server_auth - username:%s and password: %s\n", username,password);
			logger(file,output2);
			if(!strcmp(username, params.username) && !strcmp(password,params.pass_))	{
				if(sendall(sock, "0\n", 2)){
					errno = ERR_UNKNOWN;
					
					return -1;
				}
				
				return 0;
				}

				errno = ERR_AUTHENTICATION_FAILED;
				if(sendall(sock, "_1\n", 3)){
					errno = ERR_UNKNOWN;
					return -1;
				}
			
			return 0;
		}
	
		else if(!strcmp(command,"SET"))

		{	//read data and insert it into respective variable
			//variable to be used
			char *meta = strtok(NULL,",");
			char *table_ = strtok(NULL," ");
			char *key = strtok(NULL,",");
			char *value = strtok(NULL,"\0");
			
			sprintf(output2,"server_set - Table:%s, Key:%s and Value:%s\n",table_, key, value);
			logger(file,output2);
			//checking whether table exists	
			if (find_table(table_,List)){
				
				//checking whether user wants key to be deleted
				if(!strcmp(value, "_")){

					int d = delete_record(List,table_,key);
					//key doesn't exist
					if(d != 0)
					{
						//table doesn't exist
						if(d == -2) {
							errno = ERR_KEY_NOT_FOUND;
							if(sendall(sock, "_2\n", 3)){
								
								return -1;
							}
							
							return 0;
						}

						sendall(sock, "_0\n", 3);
						
						return 0;
					}
					
					//key exists and it has been deleted
					if(sendall(sock, "_\n", 2)){
						
						return -1;
					}
					
					return 0;
						
				}
				//converting necessary parameters to be used in logger function
				char* parsed_value = set_parsing (value, &sparams);
				if (parsed_value == NULL)
				{
					errno = ERR_INVALID_PARAM;
					if(sendall(sock, "_4\n", 3)){
						errno = ERR_UNKNOWN;
						
						return -1;
					}
					
					return 0;
				}
				
				char value2[1024];
				snprintf(value2,sizeof(value2),"%s",parsed_value);
				//Setting the record
				int meta2;
				if(meta)
				meta2 = atoi(meta);
				else meta2 = 0;
				int s2 = insert_key(table_,key,value2,List, params,meta2);
				if(s2 != 0)
				{
					   //table doesn't exist
					if(s2 == -3) {
						errno = ERR_TABLE_NOT_FOUND;
						if(sendall(sock, "_3\n", 3)){
							
							return -1;
						}
						
						return 0;
					}
										
					if(s2 == -4) {
					//invalid parameter
					if(sendall(sock, "_4\n", 3)){
						
						return -1;
				}
				}
				  if(s2 == -5) {
					//transaction abort
					if(sendall(sock, "_5\n", 3)){
						
						return -1;
					}
				}
				
				return 0;
			}

			sendall(sock, "_\n", 2);
			
			return 0;
	}
								
			
			//table doesn't exist
			errno = ERR_TABLE_NOT_FOUND;
			if(sendall(sock, "_3\n", 3)){
				errno = ERR_UNKNOWN;
				
				return -1;
			}
			
			return 0;
			
		}

		else if(!strcmp(command,"GET")) 
	
		{
			//read data and insert it into respective variable
			//variable to be used
			char *table_ = strtok(NULL," ");
			char *key = strtok(NULL,"\0");
			sprintf(output2,"server_get - Table:%s and Key:%s\n", table_,key);
			logger(file,output2);
			char append[1024];
			memset(append,0,sizeof(append));
			//check if table exits
			if(find_table(table_,List)) {
				int metadata_ptr = 0;
				int status = record_get(table_, key, List, append,&metadata_ptr);
				
				if(status != 0) {
					//table doesn't exist
					if(status == -2) {
						errno = ERR_KEY_NOT_FOUND;
						if(sendall(sock, "_2\n", 3)){
							
							return -1;
						}
						
						return 0;
					}
					sendall(sock, "_0\n", 3);
					
					return 0;
				}					
					char new[1024];
					memset(new,0,sizeof(new));
					snprintf(new,1024, "%d,%s\n",metadata_ptr,append);
					if(sendall(sock, new, strlen(new))){
						errno = ERR_UNKNOWN;
						
						return -1;
					}
					
					return 0;
								
				}
				//table doesn't exist
			errno = ERR_TABLE_NOT_FOUND;
			if(sendall(sock, "_3\n", 3)){
				errno = ERR_UNKNOWN;
				
				return -1;
			}
			
			return 0;
			
								
		}
		
		else if(!strcmp(command,"QUERY")) 
			
				{
					//read data and insert it into respective variable
					//variable to be used
					char *table_ = strtok(NULL,",");
					char predicates[1024];
					strncpy(predicates,strtok(NULL,"*"),sizeof(predicates));
					char *no_keys = strtok(NULL,"\0");
					int max_keys = atoi(no_keys);
					//parsing predicates
					char *parsed_ = query_parsing(predicates, &qparams);
										
					if (parsed_ == NULL)
					{
						errno = ERR_INVALID_PARAM;
						if(sendall(sock, "_4\n", 3)){
							
							return -1;
						}
						
						return 0;
					}
					
					char parsed_predicates[1024];
					strncpy(parsed_predicates,parsed_,sizeof(parsed_predicates));
					
					sprintf(output2,"server_query - Table:%s , predicates:%s and max keys:%s\n", table_,parsed_predicates,no_keys);
					logger(file,output2);
					//checking whether table exists
					if(find_table(table_,List)) {
						
						char new[1024];
						snprintf(new,1024,"%s",parsed_predicates);
						char keys_found[1024];
						memset(keys_found,0,sizeof(keys_found));
						int status = record_query(table_,new,max_keys,keys_found,List);
						
						if(status < 0) {
							
							if(status == -2) {
								errno = ERR_KEY_NOT_FOUND;
								if(sendall(sock, "_2\n", 3)){
									
									return -1;
								}
								
								return 0;
							}
							
							if(status == -4) {
								//invalid parameter
								if(sendall(sock, "_4\n", 3)){
									
									return -1;
							    }
								
								return 0;
							}
							
							sendall(sock, "_0\n", 3);
							
							return 0;
						}
					
						char send[1024];
						memset(send,0,sizeof(send));
						snprintf(send,1024,"%d,%s\n",status,keys_found);
						if(sendall(sock, send, strlen(send))){
							errno = ERR_UNKNOWN;
							
							return -1;
						}
						
						return 0;
					}
						//table doesn't exist
						errno = ERR_TABLE_NOT_FOUND;
						if(sendall(sock, "_3\n", 3)){
						errno = ERR_UNKNOWN;
						
						return -1;
					}
					
					return 0;

				}

	}
	
	else 
	{
	errno = ERR_UNKNOWN;
	sendall(sock,"_0\n",3);
	
	return 0;
	}

}

/**
 * @brief Start the storage server.
 *
 * This is the main entry point for the storage server.  It reads the
 * configuration file, starts listening on a port, and proccesses
 * commands from clients.
 */
int main(int argc, char *argv[])
{	
	
	memset(&params, 0, sizeof(params));
	params.concurrency = -1;
	
	memset(&List,0,sizeof(table));
	char output2[1000],output3[1000],output4[1000];
	
	//Query parameters
	
	memset(&qparams, 0, sizeof(qparams));
	
	//Set parameters

	memset(&sparams, 0, sizeof(sparams));

	if (LOGGING == 0) {

	file = NULL;

	}
	
	else if(LOGGING == 1) {

	file = stdout;

	}
	else if(LOGGING == 2) {
	
	time_t curr_time;
	char filename[31];
	struct tm *curr_tm;
	time(&curr_time);
	curr_tm = localtime(&curr_time);
	strftime(filename, 31, "Server-%Y-%m-%d-%H-%M-%S.log",curr_tm);
	file = fopen(filename, "w");

	}

	// Process command line arguments.
	// This program expects exactly one argument: the config file name.
	assert(argc > 0);
	
	if (argc != 2) {
		printf("Usage %s <config_file>\n", argv[0]);
		exit(EXIT_FAILURE);
	}
	char *config_file = argv[1];
	
	// Read the config file.
	int status = read_config(config_file, &params);
	if(params.concurrency == 2 || params.concurrency == 3)
		exit(2);
	if (status != 0) {
		printf("Error processing config file.\n");
		exit(EXIT_FAILURE);
	}
	
	if(insert_table(params, &List) < 0) {
		exit (-2);
	}
	
	//converting necessary parameters to be used in logger function
	sprintf(output2,"Server on %s:%d\n", params.server_host, params.server_port);
	logger(file,output2); 
	
	// Create a socket.
	int listensock = socket(PF_INET, SOCK_STREAM, 0);
	if (listensock < 0) {
		printf("Error creating socket.\n");
		exit(EXIT_FAILURE);
	}
	
	// Allow listening port to be reused if defunct.
	int yes = 1;
	status = setsockopt(listensock, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof yes);
	if (status != 0) {
		printf("Error configuring socket.\n");
		exit(EXIT_FAILURE);
	}

	// Bind it to the listening port.
	struct sockaddr_in listenaddr;
	memset(&listenaddr, 0, sizeof listenaddr);
	listenaddr.sin_family = AF_INET;
	listenaddr.sin_port = htons(params.server_port);
	inet_pton(AF_INET, params.server_host, &(listenaddr.sin_addr)); // bind to local IP address
	status = bind(listensock, (struct sockaddr*) &listenaddr, sizeof listenaddr);
	if (status != 0) {
		printf("Error binding socket.\n");
		exit(EXIT_FAILURE);
	}

	// Listen for connections.
	status = listen(listensock, MAX_LISTENQUEUELEN);
	if (status != 0) {
		printf("Error listening on socket.\n");
		exit(EXIT_FAILURE);
	}
	
	if(params.concurrency == 1) {
	
	int i;
	  /* First, we allocate the thread pool */ 
	for (i = 0; i!=MAX_CONNECTIONS; ++i)
		runtimeThreads[i] = malloc( sizeof( struct _ThreadInfo ) ); 

	// Listen loop.
	int wait_for_connections = 1;
	while (wait_for_connections) {
		// Wait for a connection.
			ThreadInfo tiInfo = getThreadInfo(); 
		    tiInfo->clientaddrlen = sizeof(struct sockaddr_in); 
		    tiInfo->clientsock = accept(listensock, (struct sockaddr*)&tiInfo->clientaddr, &tiInfo->clientaddrlen);
		    if (tiInfo->clientsock < 0) {
		      pthread_mutex_lock( &printMutex ); 
		      printf("ERROR in connecting to %s:%d.\n", 
			     inet_ntoa(tiInfo->clientaddr.sin_addr), tiInfo->clientaddr.sin_port);
		      pthread_mutex_unlock( &printMutex ); 
		    } else {
		      pthread_create( &tiInfo->theThread, NULL, threadCallFunction, (void *)tiInfo); 
		    }
	}		
	  /* At the end, wait until all connections close */ 
	  for (i=topRT; i!=botRT; i = (i+1)%MAX_CONNECTIONS)
		pthread_join(runtimeThreads[i]->theThread, 0 ); 
	
	  /* Deallocate all the resources */ 
	  for (i=0; i!=MAX_CONNECTIONS; i++)
		free( runtimeThreads[i] ); 	
	}
	  else {
  
  
  // Listen loop.
	int wait_for_connections = 1;
	while (wait_for_connections) {
		// Wait for a connection.
		struct sockaddr_in clientaddr;
		socklen_t clientaddrlen = sizeof clientaddr;
		int clientsock = accept(listensock, (struct sockaddr*)&clientaddr, &clientaddrlen);
		if (clientsock < 0) {
			printf("Error accepting a connection.\n");
			exit(EXIT_FAILURE);
		}
		sprintf(output3,"Got a connection from %s:%d.\n", inet_ntoa(clientaddr.sin_addr), clientaddr.sin_port);
		logger(file,output3);	
		
		// Get commands from client.
		int wait_for_commands = 1;
		do {
			// Read a line from the client.
			char cmd[MAX_CMD_LEN];
			int status = recvline(clientsock, cmd, MAX_CMD_LEN);
			if (status != 0) {
				// Either an error occurred or the client closed the connection.
				wait_for_commands = 0;
			} else {
				// Handle the command from the client.
				int status = handle_command(file, clientsock, cmd, &List, params, sparams, qparams);
				if (status != 0)
					wait_for_commands = 0; // Oops.  An error occured.
			}
		} while (wait_for_commands);
		
		// Close the connection with the client.
		close(clientsock);
		sprintf(output4,"Closed connection from %s:%d.\n", inet_ntoa(clientaddr.sin_addr), clientaddr.sin_port);
		logger(file,output4);
	}
	  }
	  
	if(file)
	fclose(file);
	// Stop listening for connections.
	close(listensock);

	return EXIT_SUCCESS;
}
