/**
 * @file
 * @brief This file implements a "very" simple sample client.
 * 
 * The client connects to the server, running at SERVERHOST:SERVERPORT
 * and performs a number of storage_* operations. If there are errors,
 * the client exists.
 */

#include <errno.h>
#include <stdio.h>
#include <string.h>
#include "storage.h"
#include <stdbool.h>
#include <time.h>
#include <stdlib.h>
#include <math.h>
#define LOGGING 0
#define TESTTIMEOUT	10		// How long to wait for each test to run.
#define SERVEREXEC	"./server"	// Server executable file.
#define SERVEROUT	"default.serverout"	// File where the server's output is stored.
#define SERVEROUT_MODE	0666		// Permissions of the server ouptut file.
#define ONETABLE_CONF			"conf-onetable.conf"	// Server configuration file with one table.
#define SIMPLETABLES_CONF		"conf-simpletables.conf"	// Server configuration file with simple tables.
#define COMPLEXTABLES_CONF		"conf-complextables.conf"	// Server configuration file with complex tables.
#define DUPLICATE_COLUMN_TYPES_CONF     "conf-duplicatetablecoltype.conf"        // Server configuration file with duplicate column types.
#define BADTABLE	"bad table"	// A bad table name.
#define BADKEY		"bad key"	// A bad key name.
#define KEY		"somekey"	// A key used in the test cases.
#define KEY1		"somekey1"	// A key used in the test cases.
#define KEY2		"somekey2"	// A key used in the test cases.
#define KEY3		"somekey3"	// A key used in the test cases.
#define KEY4		"somekey4"	// A key used in the test cases.
#define VALUESPC	"someval 4"	// A value used in the test cases.
#define INTCOL		"col"		// An integer column
#define INTVALUE	"22"		// An integer value
#define INTCOLVAL	"col 22"	// An integer column name and value

// These settings should correspond to what's in the config file.
#define SERVERHOST	"localhost"	// The hostname where the server is running.
#define SERVERPORT	4848		// The port where the server is running.
#define SERVERUSERNAME	"admin"		// The server username
#define SERVERPASSWORD	"dog4sale"	// The server password
//#define SERVERPUBLICKEY	"keys/public.pem"	// The server public key
//#define DATADIR		"./mydata/"	// The data directory.
#define TABLE		"inttbl"	// The table to use.
#define INTTABLE	"inttbl"	// The first simple table.
#define FLOATTABLE	"floattbl"	// The second simple table.
#define STRTABLE	"strtbl"	// The third simple table.
#define THREECOLSTABLE	"threecols"	// The first complex table.
#define FOURCOLSTABLE	"fourcols"	// The second complex table.
#define SIXCOLSTABLE	"sixcols"	// The third complex table.
#define MISSINGTABLE	"missingtable"	// A non-existing table.
#define MISSINGKEY	"missingkey"	// A non-existing key.

/*
 * @brief ptr to a file which will be used in logger function
 * 
 * */
FILE *file_ptr;


/**
 * @brief Start a client to interact with the storage server.
 *
 * If connect is successful, the client performs a storage_set/get() on
 * TABLE and KEY and outputs the results on stdout. Finally, it exists
 * after disconnecting from the server.
 */

int main(int argc, char *argv[]) {

	//global variables
	void *test_conn;
	test_conn = storage_connect("localhost",5628);
	storage_auth("admin","dog4sale",test_conn);
	char **test_keys = (char**)malloc(sizeof(char*)*40);
	int i;
	for (i = 0; i < MAX_RECORDS_PER_TABLE; i++) {
		test_keys[i] = (char*)malloc(MAX_KEY_LEN); 
		strncpy(test_keys[i], "", sizeof(test_keys[i]));
	}
	struct storage_record record;
	struct storage_record record1;
	struct storage_record record2;
	int status;
	memset(&record, 0, sizeof record);
	strncpy(record.value, "col -2", sizeof record.value);
	status = storage_set(INTTABLE, KEY1, &record, test_conn);
	strncpy(record.value, "col 2", sizeof record.value);
	status = storage_set(INTTABLE, KEY2, &record, test_conn);
	strncpy(record.value, "col 4", sizeof record.value);
	status = storage_set(INTTABLE, KEY3, &record, test_conn);

	strncpy(record.value, "col -2", sizeof record.value);
	status = storage_set(FLOATTABLE, KEY1, &record, test_conn);
	strncpy(record.value, "col 2", sizeof record.value);
	status = storage_set(FLOATTABLE, KEY2, &record, test_conn);
	strncpy(record.value, "col 4", sizeof record.value);
	status = storage_set(FLOATTABLE, KEY3, &record, test_conn);

	strncpy(record.value, "col abc", sizeof record.value);
	status = storage_set(STRTABLE, KEY1, &record, test_conn);
	strncpy(record.value, "col def", sizeof record.value);
	status = storage_set(STRTABLE, KEY2, &record, test_conn);
	strncpy(record.value, "col abc def", sizeof record.value);
	status = storage_set(STRTABLE, KEY3, &record, test_conn);
	
    strncpy(record1.value, "", sizeof record1.value);
    status = storage_get(INTTABLE, KEY2, &record1, test_conn);
	
    memset(&record2, 0, sizeof record2);
    strncpy(record2.value, "col 8", sizeof record2.value);
    status = storage_set(INTTABLE, KEY2, &record2, test_conn);
    
    strncpy(record1.value, "col 88", sizeof record1.value);
    status = storage_set(INTTABLE, KEY2, &record1, test_conn);
    
	return 0;
	
}
