/**
 * @file
 * @brief This file declares various utility functions that are
 * can be used by the storage server and client library.
 */
/*
 * Chaed by Htut, 11 Feb 00:43
 * Added more parameters to config_params struct
 */
//changes by Vincent Aulia, Tue 4 Feb, 17.03
/**
* added struct RecordElement to store key, data, and pointer to next RecordElement
*/

#ifndef	UTILS_H
#define UTILS_H

#include <stdio.h>
#include "storage.h"
#include <stdbool.h>

extern FILE *file_ptr;


/**
 * @brief Any lines in the config file that start with this character 
 * are treated as comments.
 */
static const char CONFIG_COMMENT_CHAR = '#';

/**
 * @brief The max length in bytes of a command from the client to the server.
 */
#define MAX_CMD_LEN (1024 * 8)

/**
 * @brief A macro to log some information.
 *
 * Use it like this:  LOG(("Hello %s", "world\n"))
 *
 * Don't forget the double parentheses, or you'll get weird errors!
 */
#define LOG(x)  {printf x; fflush(stdout);}

/**
 * @brief A macro to output debug information.
 * 
 * It is only enabled in debug builds.
 */
#ifdef NDEBUG
#define DBG(x)  {}
#else
#define DBG(x)  {printf x; fflush(stdout);}
#endif


//struct of data column containing int and char value, use whichever needed.
//use char type of data stored in column is char, else use int.
//contains pointer to next column

typedef struct column_struct column_struct;
typedef struct record_struct record_struct;

	struct column_struct {
	char column_name[MAX_COLUMNS_PER_TABLE][MAX_COLNAME_LEN];//COLUMN NAME
        char value[MAX_COLUMNS_PER_TABLE][MAX_VALUE_LEN];
        int type[MAX_COLUMNS_PER_TABLE];
	};

//struct containing record
//record struct contains 	key
//				pointer to next record
//				pointer to first column data
	struct record_struct {
	char key[MAX_KEY_LEN]; //RECORD KEY
	int metadata; // metadata
	record_struct *next; //POINTER TO NEXT RECORD KEY
	column_struct column; //column struct containing all the columns
	};



//struct defining a table
	typedef struct table{
	//should use MAX_TABLE_LEN but server.c doesnt include storage.h so put 20
	char table_name[MAX_TABLE_LEN];
	record_struct* RecordStruct[1000];
	}table;

//struct to store list of tables already generated
	typedef struct table_list{
	//should use MAX_TABLES but server.c doesnt include storage.h so put 100
	table Table[100];
	}table_list;
	
	


/**
 * @brief hashing function
 * @param index The key to be inserted 
 * @return Returns hashed int key
 */
int keymaker(const char* index);

typedef struct table_params table_params;
/**
 * @brief struct for storing information about column names in tables in configuration file
 * 
 *
 */
struct table_params {
	//table name
	char table_name[MAX_TABLE_LEN];
	//table columns
	char table_columns[MAX_COLUMNS_PER_TABLE][MAX_COLNAME_LEN];
	//column types
	int column_types[MAX_COLUMNS_PER_TABLE];
};
typedef struct config_params config_params;
/**
 * @brief struct for storing information about the current configuration file
 * 
 * */

struct config_params {
	/// The hostname of the server.
	char server_host[MAX_HOST_LEN];

	/// The listening port of the server.
	int server_port;
	
	/// Concurrency setting number
	int concurrency;

	/// The storage server's username
	char username[MAX_USERNAME_LEN];
	
	/// The user's password
	char pass_[MAX_ENC_PASSWORD_LEN];
	
	char table_name[MAX_TABLES][MAX_TABLE_LEN];
	
	struct table_params tablepara[MAX_TABLES];	
		
	bool duplicateparam;
		
	bool duplicatetable;
	/// The directory where tables are stored.
//	char data_directory[MAX_PATH_LEN];
};

/**
 * @brief Function that determines whether a character is alphanumeric character or not by using ASCII values.
 * @param str_ptr The character that's being evaluated.
 * @ret If the character is alphanumeric, return true. If not, return false.
 */
bool is_alpha(char str_ptr);
/**
 * @brief Exit the program because a fatal error occured.
 *[i+1
 * @param msg The error message to print.
 * @param code The program exit return value.
 */
static inline void die(char *msg, int code)
{
	printf("%s\n", msg);
	exit(code);
}

/**
 * @brief Keep sending the contents of the buffer until complete.
 * @return Return 0 on success, -1 otherwise.
 *
 * The parameters mimic the send() function.
 */
int sendall(const int sock, const char *buf, const size_t len);

/**
 * @brief Receive an entire line from a socket.
 * @return Return 0 on success, -1 otherwise.
 */
int recvline(const int sock, char *buf, const size_t buflen);

/**
 * @brief Read and load configuration parameters.
 *
 * @param config_file The name of the configuration file.
 * @param params The structure where config parameters are loaded.
 * @return Return 0 on success, -1 otherwise.
 */
int read_config(const char *config_file, struct config_params *params);

/**
 * @brief Generates a log message.
 * 
 * @param file The output stream
 * @param message Message to log.
 */
void logger(FILE *file, char *message);

/**
 * @brief Default two character salt used for password encryption.
 */
#define DEFAULT_CRYPT_SALT "xx"

/**
 * @brief Generates an encrypted password string using salt CRYPT_SALT.
 * 
 * @param passwd Password before encryption.
 * @param salt Salt used to encrypt the password. If NULL default value
 * DEFAULT_CRYPT_SALT is used.
 * @return Returns encrypted password.
 */
char *generate_encrypted_password(const char *passwd, const char *salt);

/**
 * @brief struct for storing information about the query parameters
 * 
 * */
struct query_params
{
    char column_names[MAX_COLUMNS_PER_TABLE][MAX_COLNAME_LEN];
    char operator[MAX_COLUMNS_PER_TABLE];
    char value[MAX_COLUMNS_PER_TABLE][MAX_VALUE_LEN];
};

/**
 * @brief This is a function that parses the query command from client side to be used in server side
 * 
 * @param unparsed_ pointer to the unparsed string
 * @param query_params *qparams pointer to the struct where query parameters are stored
 * @return pointer to the parsed string
 */
char* query_parsing(char* unparsed_, struct query_params *qparams);

/**
 * @brief struct for storing information about the set parameters
 * 
 * */
struct set_params
{
    char column_names[MAX_COLUMNS_PER_TABLE][MAX_COLNAME_LEN];
    char value[MAX_COLUMNS_PER_TABLE][MAX_VALUE_LEN];
};

/**
 * @brief This is a function that parses the set command from client side to be used in server side
 * 
 * @param unparsed_ pointer to the unparsed string
 * @param set_params *sparams pointer to the struct where set parameters are stored
 * @return pointer to the parsed string
 */
char* set_parsing(char* unparsed_, struct set_params *sparams);

int record_query(char* tableName, char* predicates, int max_keys, char* keys_found, table_list* list);
int insert_table(config_params params,table_list *table_ptr);
int insert_key(const char *table, const char *key, const char* value, table_list *table_ptr, config_params params, int metadata);
int create_record(int option,int i,char *key, char *value, table_list *List, config_params params, int metadata);
int delete_record(table_list* list, char* table, char* key);
int record_get(char* table, char* key, table_list* list, char *append, int *metadata);
bool find_table(char* table, table_list* table_ptr);
record_struct* find_record(char *table, char *key, table_list *list);

#endif
