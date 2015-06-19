/*
 * @file
 * @brief This file contains the implementation of the storage server
 * interface as specified in storage.h.
 */ 

#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include "storage.h"
#include "utils.h"
#define LOGGING 0
#include <math.h>
/**
 * @brief variable which stores current client info
 */
bool client_info[2] = {false};
/**
 * @brief Function that parses whether an input adheres to the alphanumeric and size rules specified.
 * @param no_cmd This determines whether only alphanumeric values are to be accepted or whether space is accepted too. 1 accepts only alphanumerics and 2 accepts both alphanumerics and space.
 * @param size This is what the size of input should be.
 * @param ptr This points to the string that is being evaluated.
 * @ret If the character is alphanumeric and/or space (according to the command number), return true. If not, return false. Also, if the size of the input is greater than the size speficied for the value, return false.
 */
bool client_parser(int no_cmd, int size, char* ptr) {

        int i;
         //only alphanumeric characters allowed
        if(no_cmd == 1) {
            for(i = 0;(ptr[i] != '\0') ; i++) {
                if(!is_alpha(ptr[i]))
                return false;
            }

            if(size < (i))
                    return false;
        }
        else if(no_cmd == 2) {
                for(i = 0;(ptr[i] != '\0'); i++) {
                    if(!is_alpha(ptr[i]) && !(ptr[i] == ' '))
                    return false;
                }
                if(size <= (i))
                    return false;
                return true;
        }
        
        return true;
}
/**
 * @brief Function that parses valid alphanumeric values and takes out invalid characters.
 * @param str Pointer pointing to the input string which might contain invalid characters.
 * @ret The function returns the pointer to the new character array which contains the parsed input.
 */
char*  parser_string (char *str) {
    
    char *tmp_ptr = malloc(sizeof(char)*(100));
    int i;
    int k = 0;
    for(i = 0; str[i] && (i < MAX_KEY_LEN); i ++) {
    
        if(is_alpha(str[i])) {
            tmp_ptr[k] = str[i];
            k++;
        }
    }
    tmp_ptr[k] = '\0';
    return tmp_ptr;
}

/**
 * @brief Creates a connection to the storage server.
 * @param hostname the name of the host.
 * @param port the port number.
 * @return void* sock pointer if successful, else returns NULL pointer.
 */
void* storage_connect(const char *hostname, const int port)
{
	//check for correct hostname
	if(!hostname || !strcmp(hostname,"") ||!client_parser(1,MAX_HOST_LEN,hostname)) {
		errno = ERR_INVALID_PARAM;
		printf("\n>Invalid Parameter was entered. Error code: %d\n\n",errno);
		return NULL;
	}
	
	//check for correct size for port no.
	if(port >= pow(10,8)) {
		errno = ERR_INVALID_PARAM;
		printf("\n>Invalid Parameter was entered. Error code: %d\n\n",errno);
		return NULL;
	}
	
	/// Create a socket.
	int sock = socket(PF_INET, SOCK_STREAM, 0);
	if (sock < 0) {
		//set errno to ERR_CONNECTION_FAIL due to a failure in the connection
		errno = ERR_CONNECTION_FAIL;
		printf("\n>Cannot connect to server @ %s:%d. Error code: %d.\n\n",
		hostname, port, errno);
		return NULL;
    	}

	/// Get info about the server.
	struct addrinfo serveraddr, *res;
	memset(&serveraddr, 0, sizeof serveraddr);
	serveraddr.ai_family = AF_UNSPEC;
	serveraddr.ai_socktype = SOCK_STREAM;
	char portstr[MAX_PORT_LEN];
	
	if(snprintf(portstr, sizeof portstr, "%d", port) < 0) {
	///unknown error might happen due to a failed attempt to create a string with certain with the port number
	errno = ERR_UNKNOWN;
	printf("\n>Cannot connect to server @ %s:%d. Error code: %d.\n\n",
			hostname, port, errno);
	return NULL;
	}
	
	int status = getaddrinfo(hostname, portstr, &serveraddr, &res);
	if (status != 0) {
		///unable to return a pointer to a linked list of addrinfo
		errno = ERR_CONNECTION_FAIL;
		printf("\n>Cannot connect to server @ %s:%d. Error code: %d.\n\n",
				hostname, port, errno);
		return NULL;
	}

	/// Connect to the server.
	status = connect(sock, res->ai_addr, res->ai_addrlen);
	if (status != 0) {
		///unable to connect to server based on file descriptor of socket
		errno = ERR_CONNECTION_FAIL;
		printf("\n>Cannot connect to server @ %s:%d. Error code: %d.\n\n",
				hostname, port, errno);
	
		return NULL;
	}
	
	client_info[0] = true;
	return (void*) sock;
}


/**
 * @brief Authenticates username and password.
 * @param username the name of the user.
 * @param password the password of the user.
 * @param conn the connection socket.
 * @return 0 if successful, -1 if fail.
 */

int storage_auth(const char *username, const char *passwd, void *conn)
{

	if(!conn) {
		errno = ERR_INVALID_PARAM;
		printf("\n>Invalid Parameter. Error code: %d\n\n",errno);
		return -1;
	}
	//check for correct username
	if(!username || !strcmp(username,"") ||!client_parser(1,MAX_USERNAME_LEN,username)) {
		errno = ERR_INVALID_PARAM;
		printf("\n>Invalid Parameter was entered. Error code: %d\n\n",errno);
		return -1;
	}
	
	if(!passwd || !strcmp(passwd,"")) {
		errno = ERR_INVALID_PARAM;
		printf("\n>Invalid Parameter was entered. Error code: %d\n\n",errno);
		return -1;
	}

	if(!client_info[0]) {
		errno = ERR_CONNECTION_FAIL;
		printf("\n>Connection Fail. Error code: %d\n\n",errno);
		return -1;
	}

	/// Connection is really just a socket file descriptor.
	int sock = (int)conn;

	/// Send some data.
	char buf[MAX_CMD_LEN];
	memset(buf, 0, sizeof buf);
	char *encrypted_passwd = generate_encrypted_password(passwd, NULL);

	//check for correct password
	if(strlen(encrypted_passwd) > MAX_ENC_PASSWORD_LEN) {
		errno = ERR_INVALID_PARAM;
		printf("\n>Invalid Parameter was entered. Error code: %d\n\n",errno);
		return -1;
	}
	
	if(snprintf(buf, sizeof buf, "AUTH %s %s\n", username, encrypted_passwd) < 0 ){
	errno = ERR_UNKNOWN;
	printf("\n>Failed to authenticate with username '%s' and password '%s'. Error code: %d.\n\n", username, passwd, errno);
	return -1;		
	}
	
	if (sendall(sock, buf, strlen(buf)) == 0 && recvline(sock, buf, sizeof buf) == 0) {
		if(!strcmp(buf,"_1")){
			errno = ERR_AUTHENTICATION_FAILED;
			printf("\n>Failed to authenticate with username '%s' and password '%s'. Error code: %d.\n\n", username, passwd, errno);
			return -1;		
		}
		else if(!strcmp(buf,"_0")){
			errno = ERR_UNKNOWN;
			printf("\n>Failed to authenticate with username '%s' and password '%s'. Error code: %d.\n\n", username, passwd, errno);
			return -1;		
		}
		client_info[1] = true;
		return 0;
	}
	
	errno = ERR_CONNECTION_FAIL;
	printf("\n>Failed to authenticate with username '%s' and password '%s'. Error code: %d.\n\n", username, passwd, errno);
	return -1;
}

/**
 * @brief gives command to retrieve a certain record stored in server.
 * @param table is the particular table containing the record.
 * @param key is the table[key] containing the linked list containing the record.
 * @param record is the record node inside linked list.
 * @param conn is the connection socket.
 * @return 0 if successful, -1 if fail.
 */
int storage_get(const char *table, const char *key, struct storage_record *record, void *conn)
{

	if(!conn) {
		errno = ERR_INVALID_PARAM;
		printf("\n>Invalid Parameter. Error code: %d\n\n",errno);
		return -1;
	}

	//check for correct table name
	if(!table || !strcmp(table,"")||!client_parser(1,MAX_TABLE_LEN,table)) {
		errno = ERR_INVALID_PARAM;
		printf("\n>Invalid Parameter was entered. Error code: %d\n\n",errno);
		return -1;
	}
	//check for correct key format
	else if(!key || !strcmp(key,"")||!client_parser(1,MAX_KEY_LEN,key)) {
			errno = ERR_INVALID_PARAM;
			printf("\n>Invalid Parameter was entered. Error code: %d\n\n",errno);
			return -1;
		}
	//check for correct key format
	else if(!record) {
		errno = ERR_INVALID_PARAM;
		printf("\n>Invalid Parameter was entered. Error code: %d\n\n",errno);
		return -1;
	}
	if(!client_info[0]) {
		errno = ERR_CONNECTION_FAIL;
		printf("\n>Connection Fail. Error code: %d\n\n",errno);
		return -1;
	}
	
	if(!client_info[1]) {
		errno = ERR_NOT_AUTHENTICATED;
		printf("\n>Authentication Fail. Error code: %d\n\n",errno);
		return -1;
	}


	/// Connection is really just a socket file descriptor.
	int sock = (int)conn;

	/// Send some data.
	char buf[MAX_CMD_LEN];
	memset(buf, 0, sizeof buf);
	if(snprintf(buf, sizeof buf, "GET %s %s\n", table, key) < 0) {
		errno = ERR_UNKNOWN;
		printf("\n>Failed to get. Error code:%d",  errno);
		return -1;
	}
	if (sendall(sock, buf, strlen(buf)) == 0 && recvline(sock, buf, sizeof buf) == 0) {
		strncpy(record->value, buf, sizeof record->value);
		//check for what type of error, or even if there was an error
		if(!strcmp(buf,"_2")){
			errno = ERR_KEY_NOT_FOUND;
			printf("\n>Failure to get value. Key not Found! Error code: %d.\n\n", errno);
			return -1;
		}
		else if(!strcmp(buf,"_3")){
			errno = ERR_TABLE_NOT_FOUND;
			printf("\n>Failure to get value. Table not Found! Error code: %d.\n\n", errno);
			return -1;
		}	
		else if(!strcmp(buf,"_0")){
			errno = ERR_UNKNOWN;
			printf("\n>Failed to get value. Error code: %d.\n\n", errno);
			return -1;		
		}
		
		char *tmp = strtok(buf,",");
		memset(record->metadata,0,sizeof(record->metadata));
		*(record->metadata) = atoi(tmp);
		char *tmp2 = strtok(NULL,"\0");
		strncpy(record->value, tmp2, sizeof (record->value));
		return 0;
	}

	errno = ERR_CONNECTION_FAIL;;
	printf("\n>Failure to get value. Error code: %d.\n\n", errno);
	return -1;
}

       
/**
 * @brief gives command to set a certain record stored in server.
 * @param table is the particular table containing the record.
 * @param key is the table[key] containing the linked list containing the record.
 * @param record is the record node inside linked list.
 * @param conn is the connection socket.
 * @return 0
 * returns 0 if successful, -1 if fail.
 */
int storage_set(const char *table, const char *key, struct storage_record *record, void *conn)
{
	
	if(!conn) {
		errno = ERR_INVALID_PARAM;
		printf("\n>Invalid Parameter. Error code: %d\n\n",errno);
		return -1;
	}
	//check for correct table name
	if(!table || !strcmp(table,"") || !client_parser(1,MAX_TABLE_LEN,table)) {
		errno = ERR_INVALID_PARAM;
		printf("\n>Invalid Parameter was entered. Error code: %d\n\n",errno);
		return -1;
	}
	//check for correct key format
	else if(!key || !strcmp(key,"") || !client_parser(1,MAX_KEY_LEN,key)) {
		errno = ERR_INVALID_PARAM;
		printf("\n>Invalid Parameter was entered. Error code: %d\n\n",errno);
		return -1;
	}
	if(!client_info[0] || !conn) {
		errno = ERR_CONNECTION_FAIL;
		printf("\n>Connection Fail. Error code: %d\n\n",errno);
		return -1;
	}
	
	if(!client_info[1]) {
		errno = ERR_NOT_AUTHENTICATED;
		printf("\n>Authentication Fail. Error code: %d\n\n",errno);
		return -1;
	}

	/// Connection is really just a socket file descriptor.
	int sock = (int)conn;

	char *tmp_ptr;
	if(!record)
		tmp_ptr = "_";
	else 
		tmp_ptr = record->value;
	/// Send some data.
	char buf[MAX_CMD_LEN];
	memset(buf, 0, sizeof buf);
	if(!record){
	if(snprintf(buf, sizeof buf, "SET 0,%s %s,%s\n",table, key, tmp_ptr) < 0 ) {
	errno = ERR_UNKNOWN;
	printf("\n> Failure to set value. Error code:%d", errno);
	return -1;
	}
	}
	else
		snprintf(buf, sizeof buf, "SET %d,%s %s,%s\n", *(record->metadata),table, key, tmp_ptr);
	if (sendall(sock, buf, strlen(buf)) == 0 && recvline(sock, buf, sizeof buf) == 0) {
		
		if(!strcmp(buf,"_2")){
			errno = ERR_KEY_NOT_FOUND;
			printf("\n>Failure to set value. Key not Found! Error code: %d.\n\n", errno);
			return -1;
		}
		else if(!strcmp(buf,"_3")){
			errno = ERR_TABLE_NOT_FOUND;
			printf("\n>Failure to set value '%s'. Table not Found! Error code: %d.\n\n",record->value, errno);
			return -1;
		}
		else if(!strcmp(buf,"_4")){
			errno = ERR_INVALID_PARAM;
			printf("\n>Invalid Parameter was entered. Error code: %d\n\n",errno);
			return -1;
		}
		else if(!strcmp(buf,"_0")){
			errno = ERR_UNKNOWN;
			printf("\n>Failed to set value. Error code: %d.\n\n",errno);
			return -1;		
		}
		else if(!strcmp(buf,"_5")){
			errno = ERR_TRANSACTION_ABORT;
			printf("\n>Failed to set value. Error code: %d.\n\n",errno);
			return -1;
		}
		
		return 0;
	}
	
	errno = ERR_UNKNOWN;
	printf("\n>Failure to set value. Error code: %d.\n\n", errno);
	return -1;
}
int storage_query(const char *table, const char *predicates, char **keys, const int max_keys, void *conn) {

	if(!conn) {
			errno = ERR_INVALID_PARAM;
			printf("\n>Invalid Parameter. Error code: %d\n\n",errno);
			return -1;
		}
		//check for correct table name
		if(!table || !strcmp(table,"") || !client_parser(1,MAX_TABLE_LEN,table)) {
			errno = ERR_INVALID_PARAM;
			printf("\n>Invalid Parameter was entered. Error code: %d\n\n",errno);
			return -1;
		}
		//check for correct predicates format
		else if(!predicates || !strcmp(predicates,"")) {
			errno = ERR_INVALID_PARAM;
			printf("\n>Invalid Parameter was entered. Error code: %d\n\n",errno);
			return -1;
		}
		if(max_keys < 0) {
			errno = ERR_INVALID_PARAM;
			printf("\n>Invalid Parameter was entered. Error code: %d\n\n",errno);
			return -1;	
		}
		if(!client_info[0] || !conn) {
			errno = ERR_CONNECTION_FAIL;
			printf("\n>Connection Fail. Error code: %d\n\n",errno);
			return -1;
		}
		
		if(!client_info[1]) {
			errno = ERR_NOT_AUTHENTICATED;
			printf("\n>Authentication Fail. Error code: %d\n\n",errno);
			return -1;
		}

		/// Connection is really just a socket file descriptor.
		int sock = (int)conn;

		/// Send some data.
		char buf[MAX_CMD_LEN];
		memset(buf, 0, sizeof buf);
		if(snprintf(buf, sizeof buf, "QUERY %s,%s*%d\n", table, predicates, max_keys) < 0 ) {
		errno = ERR_UNKNOWN;
		printf("\n> Failure to set value. Error code:%d", errno);
		return -1;
		}
		char buf2[MAX_CMD_LEN];
		memset(buf2, 0, sizeof buf2);
		if (sendall(sock, buf, strlen(buf)) == 0 && recvline(sock, buf2, sizeof(buf2)) == 0) {
			//check for what type of error, or even if there was an error
			if(!strcmp(buf2,"_2")){
				errno = ERR_KEY_NOT_FOUND;
				printf("\n>Failure to set value. Key not Found! Error code: %d.\n\n", errno);
				return -1;
			}
			else if(!strcmp(buf2,"_3")){
				errno = ERR_TABLE_NOT_FOUND;
				printf("\n>Failure to set value. Table not Found! Error code: %d.\n\n", errno);
				return -1;
			}
			else if(!strcmp(buf2,"_4")){
				errno = ERR_INVALID_PARAM;
				printf("\n>Invalid Parameter was entered. Error code: %d\n\n",errno);
				return -1;
			}
			else if(!strcmp(buf2,"_0")){
				errno = ERR_UNKNOWN;
				printf("\n>Failed to query. Error code: %d.\n\n", errno);
				return -1;		
			}
			
			//parse all the data and put it into the char** for the user
			int y;
			char copy_buf[1024];
			char copy_no[1024];
			strncpy(copy_buf,buf2,strlen(buf2));
			snprintf(copy_no,1024,"%c\0",copy_buf[0]);
			int number = atoi(copy_no);
			int d = 1;
			for (y = 0; y < number; y++){
				d++;
				int t = 0;
				char tmp_char[1024];
				memset(tmp_char,0,sizeof(tmp_char));
				if(copy_buf[d] == '\0')
					break;
				for(d; copy_buf[d] != '\0' && copy_buf[d] != ',';d++){
					tmp_char[t] = copy_buf[d];
					t++;
				}
				keys[y] = tmp_char;
			}	

			return number;
		}
		
		errno = ERR_UNKNOWN;
		printf("\n>Failure to set value. Error code: %d.\n\n", errno);
		return -1;

}
int storage_disconnect(void *conn)
{
	if(!conn) {
	errno = ERR_INVALID_PARAM;
	printf("\n>Invalid Parameter. Error code: %d.\n\n", errno);
	return -1;
	}
	/// Cleanup
	else if(!client_info[0]){
		errno = ERR_CONNECTION_FAIL;
		printf("\n>Failure Connect. Error code: %d.\n\n", errno);
		return -1;
	}
	int sock = (int)conn;
	close(sock);
	client_info[0] = false;
	client_info[1] = false;
	return 0;
	
}

