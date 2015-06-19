/**
 * @file
 * @brief This file implements various utility functions that are
 * can be used by the storage server and client library. 
 */
/*
 * Changed by Htut, 11 February 00:41
 * Added modifed read_config and process_config_line functions
 */

#define _XOPEN_SOURCE

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include "utils.h"
#include <errno.h>
#include "storage.h"


record_struct* find_record(char *table, char *key, table_list *list)
{
    //return -1 if key cannot be found
    //else return 0
    int i;//loop counter for table
    int j;//loop counter for record_struct
    int keyInt = keymaker(key);
    for(i=0;i<100;i++)
    {
        if(list->Table[i].table_name[0] != '\0')//if tablename is not null
        {//then go in
            if(!strcmp(list->Table[i].table_name, table))//if name is equal
            {
                    if(list->Table[i].RecordStruct[keyInt] != NULL)//if record struct pointer is not null
                    { 
                        record_struct* copystruct = list->Table[i].RecordStruct[keyInt];
                        while(copystruct != NULL)//while its not the end of the key linked list
                        {
                            if(!strcmp(copystruct->key, key))//if key is equal
                                {
                                return copystruct;
                                }
                            else//if not the same keep continue traverse node
                                {
                                copystruct = copystruct->next;
                                }
                        }
                        
                    } 
                    
              return NULL; //record not found, return NULL  
            }
            
        }
    }
     return NULL; //record not found, return NULL
}
bool is_number(char* value)
{
    bool isNumber = true;
    int i = 0;
    while(isNumber && value[i] != '\0')
    {
        if(i == 0 && value[i] == '-')
        {
            isNumber = true;
            i++;
        }
        else if(i == 0 && value[i] == '+')
        {
            isNumber = true;
            i++;
        }
        else if(value[i] >= '0' && value[i] <= '9')
        {
            isNumber = true;
            i++;
        }
        else if(value[i] == '-' || value[i] == '+')
        {
            isNumber = false;
        }
        else isNumber = false;
    }
    return isNumber;
}

/**
 * @brief This is a function to generate a key from a string.
 * @param key String to be hashed into an integer key
 * @return Returns the integer key
 */
int keymaker(const char* key)
{
	int address_record; ///Address to the record
	int total = 0;
        int first_two=0;
	int len = strlen(key);
	///Get a certain number from multiplication of ascii char
	int n;
        int m;
	for(n = 0; n < len; n++)
	{
		if(key[n]!= '\0')
		{
                    int value = key[n];
			total = total+value;
		} 
	}
        
        for(m = 0; m < 2; m++)
	{
		if(key[m]!='\0')
		{
                    int value = key[m];
			first_two = first_two+value;
		} 
	}
        int first_two_mod = (first_two % 4 + 1);
        ///Makes a key from the result of both above
        address_record = (total*first_two_mod) % 997;
	return address_record;

}


bool find_table(char* table, table_list* table_ptr)
{
    int i; //integer which holds the current index location while traversing through table_ptr
    for(i = 0; table_ptr->Table[i].table_name[0] != NULL && i < MAX_TABLES; i++) {
        //check whether we are at the correct spot by looking at table names
        if(!strcmp(table_ptr->Table[i].table_name,table)) {
            //table found
            return true;
            
        }
    }//else table not found
    return false;
}
int record_get(char* table, char* key, table_list* list, char *append, int *metadata_ptr)
{
    bool found_table = find_table(table, list);
    if(found_table)
    {
        record_struct* copystruct = find_record(table, key, list);
        if(copystruct != NULL)//found record struct
        {
	    *metadata_ptr = copystruct->metadata;	
            int i;
            for(i=0;i<MAX_COLUMNS_PER_TABLE && copystruct->column.column_name[i][0] != '\0';i++)
            {
                    strcat(append, copystruct->column.column_name[i]);//if the value here is not null
                    strcat(append, " ");
                    strcat(append, copystruct->column.value[i]);
                    if(copystruct->column.value[i+1][0] != '\0')//check if there is more value later
                    {
                    strcat(append, " ");
                    strcat(append, ",");
                    strcat(append, " ");
                    }
            }
            return 0;
        }
        else
        {
            return -2;
        }
    }
    else
    {
        return -3;
    }
    
}
int delete_record(table_list* list, char* table, char* key)
{
    //return -1 if key cannot be found
    //else return 0
    int i;//loop counter for table
    int j;//loop counter for record_struct
    int keyInt = keymaker(key);
    for(i=0;i<100 && list->Table[i].table_name[0] != '\0';i++)
    {
       //then go in
            if(!strcmp(list->Table[i].table_name, table))//if name is equal
            {
                    if(list->Table[i].RecordStruct[keyInt] != NULL)//if record struct pointer is not null
                    {
                        //make copy pointer to key struct and the one next to it
                        record_struct* headstruct = list->Table[i].RecordStruct[keyInt];
                        record_struct* copycopystruct = headstruct;
                        if(!strcmp(list->Table[i].RecordStruct[keyInt]->key, key))//delete head
                                {
                                list->Table[i].RecordStruct[keyInt] = copycopystruct->next;
                                free(copycopystruct); //free main struct
                                return 0;
                                }
                              record_struct* copystruct = headstruct->next;
                        while(copystruct != NULL)//while its not the end of the key linked list
                        {
                            if(!strcmp(copystruct->key, key))//if key is equal
                                {
                                copycopystruct->next = copystruct->next;
                                free(copystruct); //free main struct
                                return 0;
                                }
                            else//if not the same keep continue traverse node
                                {
                                copystruct = copystruct->next;
                                copycopystruct = copycopystruct->next;
                                }
                        }
                        
                    } return -2;
                    
                
            } 
    }
    return -3;
}
int create_record(int option,int i,char *key, char *value, table_list *List, config_params params, int metadata){
			char tmp_d[1024];
	        memset(tmp_d,0,sizeof(tmp_d));
	        strncpy(tmp_d,value,sizeof(tmp_d));
	        char *tmp_chard = strtok(tmp_d," ");//gives me the column name
	        int d;
	        for(d = 0; d < MAX_COLUMNS_PER_TABLE && params.tablepara[i].table_columns[d][0] != NULL && tmp_chard;d++) {
	                //create column name
	                if(strcmp(tmp_chard,params.tablepara[i].table_columns[d])) //check whether column exists and its in order
	                    return -4;
	                tmp_chard = strtok(NULL,","); //get the value for the column
	                tmp_chard = strtok(NULL," ");
	        }     
	        if (tmp_chard != NULL || params.tablepara[i].table_columns[d][0] != NULL)
	            return -4;
	        char tmp_c[1024];
	        strncpy(tmp_c,value,sizeof(tmp_c));
	        int j;
	        bool insert_head = false;
	        int hash = keymaker(key);
	        record_struct *tmp_record = NULL;
	        record_struct *ptr_record = NULL;
	        char *tmp_char = strtok(tmp_c," ");//gives me the column name
	        if(option == 2) {
	                ptr_record = find_record(List->Table[i].table_name,key,List);
	        }
	        else {
	            //if record doesn't exist, create it and parse for invalid input parameters
	            tmp_record =  List->Table[i].RecordStruct[hash];
	            if(! List->Table[i].RecordStruct[hash]){//insert at the head
	                List->Table[i].RecordStruct[hash] = (record_struct*)malloc(sizeof(record_struct));//insert at the head  
	                List->Table[i].RecordStruct[hash]->next = NULL;
	                insert_head = true;
	            }
	            else {
	                while(tmp_record->next) 
	                    tmp_record = tmp_record->next;
	                tmp_record->next = (record_struct*)malloc(sizeof(record_struct));//insert at the end 
	                tmp_record->next->next = NULL;
	                
	            }
	        }
	        if(option == 1) {
	        	
	        	if(insert_head){
	            ptr_record =  List->Table[i].RecordStruct[hash];
	            strncpy(ptr_record->key,key,MAX_KEY_LEN);
	        	}
	        	else {
	        		ptr_record = tmp_record->next;
	        		strncpy(ptr_record->key,key,MAX_KEY_LEN);
	        	}
			ptr_record->metadata = 1;
	        }
		else if(option == 2) {

		if(metadata == 0)
			ptr_record->metadata++;
			//don't do anything		
		else if(ptr_record->metadata != metadata) 
			return -5;
		
		else ptr_record->metadata++;
		
		}
	        for(j = 0; j < MAX_COLUMNS_PER_TABLE && params.tablepara[i].table_columns[j][0] != NULL && tmp_char;j++) {
	                //create column name
	                if(strcmp(tmp_char,params.tablepara[i].table_columns[j])) //check whether column exists and its in order
	                    return -4;
	                strncpy(ptr_record->column.column_name[j],params.tablepara[i].table_columns[j], MAX_COLNAME_LEN);
	                //check for the type of value for the column
	                tmp_char = strtok(NULL,","); //get the value for the column
	                if(params.tablepara[i].column_types[j] != -1){
	                    //check if value has the according size
	                    if(params.tablepara[i].column_types[j] < strlen(tmp_char))
	                            return -4; //invalid param. string doesn't abide by the size
	                    //copy value to the key
	                    strncpy(ptr_record->column.value[j],tmp_char,800);  
	                    ptr_record->column.type[j] = params.tablepara[i].column_types[j];
	                }
	                //it is a character, malloc for 2^32
	               else {
	               //check if the value is a number
	                        if(!is_number(tmp_char))
	                            return -4;
	                            //copy value to the key
	                            strncpy(ptr_record->column.value[j],tmp_char,800);
	                            ptr_record->column.type[j] = params.tablepara[i].column_types[j];
	                    }
	                    tmp_char = strtok(NULL," ");//get the column name
	                } 
	        
	         return 0;
             
}
int insert_key(const char *table, const char *key, const char* value, table_list *table_ptr, const config_params params, int metadata) {
	    //create hashing key for value
	    int hash_key =  keymaker(key);  
	    bool found = false; //true if table found false otherwise
	    char tmp_c[1024];
	    strncpy(tmp_c,value,1024); //copy of value
	    int j; //integer for keeping track of the columns being searched
	    //loop through table list until table is found
	    int i; //integer which holds the current index location while traversing through table_ptr
	    for(i = 0; table_ptr->Table[i].table_name[0] != 0 && i < MAX_TABLES; i++) {
	        //check whether we are at the correct spot by looking at table names
	        if(!strcmp(table_ptr->Table[i].table_name,table)) {
	            //table found, now insert key
	            found = true;
	            break;
	        }
	    }
	    
	    if(!found) //table not found
	        return -3;
	     //create columns and insert values
	     record_struct * tmp = find_record(table,key,table_ptr);
	     if(tmp)
	         //update value for the record
	         return create_record(2,i,key,value,table_ptr,params,metadata);
	    else {
	        //insert new value
	        return create_record(1,i,key,value,table_ptr,params,metadata);
	    }
	    return -1;
}
//function for inserting tables with their respective columns
int insert_table(config_params params,table_list *table_ptr) {
	    //while loop for checking the right spot to insert Table
	    int i; //this gives you the index inside params for looking at tables to be created
	    //find the correct spot for inserting the table, this will be given by i
	    for(i = 0; params.tablepara[i].table_name[0] && i < MAX_TABLES; i++) {
	        //copy that name into the table_list struct
	        strncpy(table_ptr->Table[i].table_name,params.tablepara[i].table_name,MAX_TABLE_LEN); 
	    }
	    
	    //check for running out of memory
	    if(i == MAX_TABLES)
	        return -4;
	    //everything worked fine
	    else
	        return 0;      
}

int record_query(char* tableName, char* predicates, int max_keys, char* keys_found, table_list* list)
{
    int ColumnName = 0;
    bool foundtable = find_table(tableName, list);
    if(foundtable)//if table exists
    {
        int foundkeys=0;
        int i;
        char *temp = NULL;
        char columnName[MAX_COLUMNS_PER_TABLE][MAX_COLNAME_LEN];//stores name
        memset(columnName,0,sizeof(columnName));
        char columnOp[MAX_COLUMNS_PER_TABLE][MAX_COLNAME_LEN];//stores operator
        memset(columnOp,0,sizeof(columnOp));
        char columnVal[MAX_COLUMNS_PER_TABLE][MAX_COLNAME_LEN];//stores value
        memset(columnVal,0,sizeof(columnVal));
        int columnValType[MAX_COLUMNS_PER_TABLE];//stores type of value
        memset(columnValType,0,sizeof(columnValType));
        temp = strtok(predicates, " ,");
        for(i=0;i<10;i++)
        {
                int counter = 1;//reset counter
                while(temp != NULL && counter < 4)
                {
                        if(counter == 1)
                        {//means temp contains column name
                                strcpy(columnName[i], temp);
                                temp = strtok(NULL, " ,");
                                counter++;
                        }
                        else if(counter == 2)
                        {//means temp contains operator
                                strcpy(columnOp[i], temp);
                                temp = strtok(NULL, ",");
                                counter++;
                        }
                        else if(counter == 3)
                        {//means temp contains column value
                                bool isNumber = is_number(temp);//check if int or string
                                if(isNumber)
                                {
                                        columnValType[i] = -1;
                                }
                                else if(!isNumber)
                                {
                                        columnValType[i] = strlen(temp);
                                }
                                strcpy(columnVal[i], temp);
                                temp = strtok(NULL, " ,");
                                counter++;
                        }
                }//finished 1 predicate, reset counter
        }//end of interpreting predicate, have all values

        bool found = false;
        record_struct* pointer = NULL; //returns pointer to right table
        //integer which holds the current index location while traversing through table_ptr
        int a;
        for(a = 0; list->Table[a].table_name[0] != 0 && i < MAX_TABLES; a++) {
            //check whether we are at the correct spot by looking at table names
            if(!strcmp(list->Table[a].table_name,tableName)) {
                //table found
                found = true;
                break;
            }
        }
        if(!found)
            return -3;
        int j;
        //check for columns
        int k;
        int l;
        for(j=0;j<1000;j++)
        {
            pointer = list->Table[a].RecordStruct[j];//pointer to records in j
            if(pointer != NULL)//if not empty record go inside
            {
                record_struct* copypointer = pointer;
                
                while(copypointer != NULL)//check for end of array
                {

                    bool isFound = false;
                    for(k=0; k<10 && copypointer->column.column_name[k][0] != '\0';k++)//loop to go to each column name in data
                    {
                            for(l=0;l<10;l++)//loop to go through each column name in predicates
                            {
                                if(columnName[l][0] != '\0')//if column name in predicate is not empty
                                {
                                    if(!strcmp(copypointer->column.column_name[k],columnName[l]))//if the name is equal
                                    {
                                        ColumnName++;
                                        if(columnValType[l] == -1 && copypointer->column.type[k] == -1)//then its an int
                                        {                                            
                                            if(columnOp[l][0] == '=')
                                            {
                                                int temp1 = atoi(copypointer->column.value[k]);
                                                int temp2 = atoi(columnVal[l]);
                                                if(temp1 == temp2)
                                                isFound = true;
                                                else {
							isFound = false;
							goto end;}
                                            }
                                            else if(columnOp[l][0] == '>')
                                            {
                                                int temp1 = atoi(copypointer->column.value[k]);
                                                int temp2 = atoi(columnVal[l]);
                                                if(temp1 > temp2)
                                                isFound = true;
                                                else {
							isFound = false;
							goto end;}
                                            }
                                            else if(columnOp[l][0] == '<')
                                            {
                                                
                                                int temp1 = atoi(copypointer->column.value[k]);
                                                int temp2 = atoi(columnVal[l]);
                                                if(temp1 < temp2)
                                                isFound = true;
                                               else {
							isFound = false;
							goto end;}
                                            }
                                            else return -4;
                                        }
                                        else if(columnValType[l] > -1 && copypointer->column.type[k] > -1)
                                        {
                                            if(columnOp[l][0] == '=')
                                            {
                                                if(!strcmp(copypointer->column.value[k],columnVal[l]))
                                                isFound = true;
                                                else {
							isFound = false;
							goto end;}
                                            }
                                            else return -4;
                                        }
                                }
                            }
                        }
                    }//finish checking for each column of a key, if still true
		end:                    
                    if (isFound) {
                                if(ColumnName == 0)
                                     return -4;
                                else
                                {
                                                        foundkeys++;
                                                        if(foundkeys < max_keys && foundkeys == 1)
                                                        {
                                                            strcat(keys_found, copypointer->key);
                                                        }
                                                        else if(foundkeys <= max_keys)
                                                        {
                                                            strcat(keys_found, ",");
                                                            strcat(keys_found, copypointer->key);
                                                        }

                                }
                    }
                    copypointer = copypointer->next;
                }
            }//done checking this recordstruct array, go to next
        }
        return foundkeys;
        
    }
    return -3;
   
   
   
}

/**
 * @brief Function that determines whether a character is alphanumeric character or not by using ASCII values.
 * @param str_ptr The character that's being evaluated.
 * @ret If the character is alphanumeric, return true. If not, return false.
 */
bool is_alpha(char str_ptr) {

    int i = (int)(str_ptr);
    if(((i >= 48) && (i <= 57)) || ((i >= 65) && (i <= 90)) || ((i >= 97) && (i <= 122)))
        return true;
    else return false;

}
/**
 * @brief Keep sending the contents of the buffer until complete.
 * @param sock Connection socket
 * @param buf String buffer
 * @param len Length of buffer
 * @return Return 0 on success, -1 otherwise.
 *
 * The parameters mimic the send() function.
 */
int sendall(const int sock, const char *buf, const size_t len)
{
	size_t tosend = len;
	while (tosend > 0) {
		ssize_t bytes = send(sock, buf, tosend, 0);
		if (bytes <= 0) 
			break; /// send() was not successful, so stop.
		tosend -= (size_t) bytes;
		buf += bytes;
	};

	return tosend == 0 ? 0 : -1;
}

/**
 * @brief Reads the stream one byte at a time.
 * @param sock Connection socket
 * @param buf String buffer
 * @param buflen Length of buffer
 * @return Return 0 on success, -1 otherwise.
 * In order to avoid reading more than a line from the stream,
 * this function only reads one byte at a time.  This is very
 * inefficient, and you are free to optimize it or implement your
 * own function.
 */
int recvline(const int sock, char *buf, const size_t buflen)
{
	int status = 0; /// Return status.
	size_t bufleft = buflen;

	while (bufleft > 1) {
		/// Read one byte from scoket.
		ssize_t bytes = recv(sock, buf, 1, 0);
		if (bytes <= 0) {
			/// recv() was not successful, so stop.
			status = -1;
			break;
		} else if (*buf == '\n') {
			/// Found end of line, so stop.
			*buf = 0; // Replace end of line with a null terminator.
			status = 0;
			break;
		} else {
			/// Keep going.
			bufleft -= 1;
			buf += 1;
		}
	}
	*buf = 0; /// add null terminator in case it's not already there.

	return status;
}

/**
 * @brief Read and load configuration parameters.
 *
 * @param line Line of input read from file.
 * @param params The structure where config parameters are loaded.
 * @return Return 0 on success, -1 otherwise.
 */
int process_config_line(char *line, struct config_params *params)
{	
	/// Commented lines ignored.
	if (line[0] == CONFIG_COMMENT_CHAR || line[0] == '\n')
		return 0;
	/// Extract config parameter name and value.
	char name[MAX_CONFIG_LINE_LEN];
	char value[MAX_CONFIG_LINE_LEN];
	int items = sscanf(line, "%s %s\n", name, value);

	/// Line wasn't as expected.
	if (items != 2)
		return -1;

	/// Process this line.
	if (strcmp(name, "server_host") == 0) 
	{
		if (params->server_host[0] != NULL)
		{
			params->duplicateparam = true;
			return -1;
		}
		strncpy(params->server_host, value, sizeof params->server_host);
	} 
	
	else if (strcmp(name, "server_port") == 0) 
	{
		if (params->server_port != 0)
		{
			params->duplicateparam = true;
			return -1;
		}
		params->server_port = atoi(value);
	}
	
	else if (strcmp(name, "concurrency") == 0) 
	{
		
		if (params->concurrency != -1)
		{
			params->duplicateparam = true;
			return -1;
		}
		if (!strcmp(value, "0") || !strcmp(value, "1") || !strcmp(value, "2") || !strcmp(value, "3"))
		{
			params->concurrency = atoi(value);
			return 0;
		}
		else
			return -1;
	}	
	
	else if (strcmp(name, "username") == 0) 
	{
		if (params->username[0] != NULL)
		{	
			params->duplicateparam = true;
			return -1;
		}
		strncpy(params->username, value, sizeof params->username);
	} 
	else if (strcmp(name, "password") == 0) 
	{
		if (params->pass_[0] != '\0')
		{	
			params->duplicateparam = true;
			return -1;
		}
		strncpy(params->pass_, value, sizeof params->pass_);
	}
	
	else if (strcmp(name, "table") == 0)
	{
		int i;
		int columncount = 0;
		
		//checking whether table name is alphanumeric
		int tn;
		for (tn = 0; tn< strlen(value); tn++)
		{
			int tnv = (int)(value[tn]);
			if(((tnv >= 48) && (tnv <= 57)) || ((tnv >= 65) && (tnv <= 90)) || ((tnv >= 97) && (tnv <= 122)))
					;
			else
			{
			printf("\nError: Invalid parameters in configuration file.\n");
			return -1;
			}
		}
		
		
		for (i = 0; i< MAX_TABLES; i++) //checking whether there're duplicate tables
				{	
					if (params->table_name[i][0] == '\0') //found empty space
						break;
					
					
					if (strcmp(value, params->table_name[i]) == 0) ///if duplicate table name already exists
					{
						params->duplicatetable = true;
						return -1;
					}
				}
		
		//if there're no duplicate tables and valid, start storing them
		strncpy(params->table_name[i], value, MAX_TABLE_LEN );
		strncpy(params->tablepara[i].table_name, value, MAX_TABLE_LEN );
		//printf("Table name stored is %s\n", params->tablepara[i].table_name);
		
		//looping through to count number of commas
		int cc;
		int commacount = 0;
		for(cc = 0; (line[cc]!= '\n'); cc++ )
		{
			int ccv = (int)(line[cc]);
			if (ccv == 44)
				commacount++;
		}
		//printf("Comma count is %d\n", commacount);		
		
		//moving onto getting table schema
		char *token, *token_end;
		
		token = strtok_r(line, " \n", &token_end ); //first token is table
		//printf("%s1\n", token);
		int tokencount = 1;
		
		token = strtok_r(NULL, " \n", &token_end ); //second token is table name
		//printf("%s2\n", token);
		tokencount++;
		
		char*token_endcopy = (char *)malloc (strlen(token_end)+1); //this is the unparsed string with newline character that contains column pairs
		strcpy(token_endcopy, token_end);
		
		token = strtok_r(NULL, " \n", &token_end ); //this is just to check whether the table has a schema
		if (token == NULL && tokencount == 2)
		{
			printf("\nError: Missing parameters in the configuration file.\n");
				return -1;
		}
		
		char *tokenA, *tokenA_end;
		tokenA = strtok_r(token_endcopy, ",\n", &tokenA_end ); //token A is the first pair of table column and type, definitely not NULL
		tokencount++; //token count is now 3 with the first pair
		//printf("%s3\n", tokenA);
		
		//Beginning of parsing the first pair of column
		char *tokenB, *tokenB_end;
		tokenB = strtok_r(tokenA, " \n", &tokenB_end); //Cutting out white spaces
		//printf("%s4\n", tokenB);
		
		char*tokenBcopy = (char *)malloc (strlen(tokenB)+1);
		strcpy (tokenBcopy, tokenB);				
						
		tokenB = strtok_r(NULL, " \n", &tokenB_end);
						
		if (tokenB == NULL) //Meaning there's no spaces within the column name and type pair
		{					
				//Cutting away between column name and type. Also check for extra or missing commas
				char *tokenC, *tokenC_end;
				tokenC = strtok_r(tokenBcopy, ":\n", &tokenC_end); //This should be the first column name
				//printf("%s5\n", tokenC);
				int tokenCcount = 1; //Gonna make sure there're only two tokens in one pair
												
				//Check for duplicate column names
				int ka;
				for (ka = 0; ka < MAX_COLUMNS_PER_TABLE; ka++)
				{
				if (params->tablepara[i].table_columns[ka][0] == '\0')
					break;
				if (strcmp(tokenC, params->tablepara[i].table_columns[ka]) == 0) ///if duplicate column name already exists
					{
						printf("\nError: Duplicate table column names in configuration file.\n");
							return -1;
					}	
				}
							
				//Check whether column name is alphanumeric
				int alp;
				for (alp = 0; alp< strlen(tokenC); alp++)
				{
					int alpv = (int)(tokenC[alp]);
					if(((alpv >= 48) && (alpv <= 57)) || ((alpv >= 65) && (alpv <= 90)) || ((alpv >= 97) && (alpv <= 122)))
						;
					else
					{
						printf("\nError: Invalid parameters in configuration file.\n");
							return -1;
					}
				}		
							
				//If there're no errors, store column name
				strncpy(params->tablepara[i].table_columns[columncount], tokenC, MAX_COLNAME_LEN);					
				//printf("%s6\n", params->tablepara[i].table_columns[columncount]);
				
				//Continue on to next part of the string which is column type of the first pair
				while (tokenC != NULL)
				{
					tokenC = strtok_r(NULL, ":\n", &tokenC_end); //This should be the column type
						if(tokenC == NULL)
							break;
								
						//Check whether it's an int or char type or neither							
						if (strcmp(tokenC, "int") == 0) //type is an int
						{
							params->tablepara[i].column_types[columncount] = -1;
							columncount++;
							//printf("Type stored is %d.\n", params->tablepara[i].column_types[columncount]);
						}
						
						else if (strncmp(tokenC,"char[",5) == 0) //type is a char[n] 
						{
							int charvalA;
							char* tokencopy;
							tokencopy = (char *)malloc (strlen(tokenC)+1);
							strcpy (tokencopy, tokenC);
							
							//separating integer from char[n]
							char tempnumberstr[10];
														
							int a1;
							int jk = 0;
							for(a1 = 0; (tokencopy[a1]!= ']'); a1++ )
							{
								if(tokencopy[a1] == '[')
								{
									a1++;
									while (tokencopy[a1] == ' ')
										a1++;
									break;
								}					
							}
							
							int k1;
							for (k1 = a1; (tokencopy[k1]!= ']'); k1++ )
							{	
								int tv = (int)(tokencopy[k1]);
								if (((tv >= 48) && (tv <= 57)) || (tv == 43) || (tv == 45)) //is a number or plus minus sign
								{
									tempnumberstr[jk]= tokencopy[k1];
									jk++;
								}	
								else
								{
									printf("\nError: Invalid parameters in configuration file.\n");
										return -1;
								}
							}
							jk++;
							tempnumberstr[jk]= '\0';
							charvalA = atoi(tempnumberstr);
							
							if (charvalA<=0) //if the size of the array is negative or zero size array
							{
								printf("\nError: Invalid parameters in configuration file.\n");
									return -1;	
							}
							//Storing column type
							params->tablepara[i].column_types[columncount]= charvalA;
							//printf("Type stored is %d.\n", params->tablepara[i].column_types[columncount]);
							columncount++;
							free(tokencopy);
						}					
						else //Wrong column type or no column type
						{
							printf("\nError: Invaid parameters in the configuration file.\n");
									return -1;
						}
						tokenCcount++;
						}
					if (tokenCcount != 2) //Accounting for invalid commas, dividing up name and type
					{
						printf("\nError: Invaid format in the configuration file.\n");
							return -1;
					} 
			}
						
			if (tokenB != NULL) //If spaces exist between colons or in names and types
			{
				printf("\nError: Invaid format in the configuration file.\n");
				return -1;
			}			
		//End of parsing first pair of column name and type
		
		//Moving onto next pairs of column names and types
		//tokenA_end is where the rest of the string exists
		///////////////////////////////////////////////////////////////////
		while (tokenA != NULL)
		{
			char *token2, *token2_end;
			tokenA = strtok_r(NULL, ",\n", &tokenA_end); //tokenA = pairs of column names
			
			if (tokenA == NULL)
				break;
			tokencount++; //tokencount is incremented everytime there's a new pair
			//printf("%s7\n", tokenA);
			
			//Cutting out white spaces in name and type column pairs
			token2 = strtok_r(tokenA, " \n", &token2_end);
			char*token2copy = (char *)malloc (strlen(token)+1);
			strcpy(token2copy, token2);				
							
			token2 = strtok_r(NULL, " \n", &token2_end);
			
			if(token2 == NULL) //No weird white spaces in column pair
			{
				//Cutting away between column name and type. Also check for extra or missing commas
				char *token3, *token3_end;
				token3 = strtok_r(token2copy, ":\n", &token3_end); //This should be the column name
				//printf("%s8\n", token3);
				int token3count = 1; //Gonna make sure there're only two tokens in one pair
																
				//Check for duplicate column names
				int k;
				for (k = 0; k< MAX_COLUMNS_PER_TABLE; k++)
				{
					if (params->tablepara[i].table_columns[k][0] == '\0')
								break;
					if (strcmp(token3, params->tablepara[i].table_columns[k]) == 0) ///if duplicate column name already exists
					{
						printf("\nError: Duplicate table column names in configuration file.\n");
									return -1;
					}	
				}
											
				//Check whether column name is alphanumeric
				int al;
				for (al = 0; al< strlen(token3); al++)
				{
					int alv = (int)(token3[al]);
					if(((alv >= 48) && (alv <= 57)) || ((alv >= 65) && (alv <= 90)) || ((alv >= 97) && (alv <= 122)))
						;
					else
					{
						printf("\nError: Invalid parameters in configuration file.\n");
								return -1;
					}
				}		
											
				//If there're no errors, store column name
				strncpy(params->tablepara[i].table_columns[columncount], token3, MAX_COLNAME_LEN);					
				//printf("%s9\n", params->tablepara[i].table_columns[columncount]);
								
				//Continue on to next part of the string which is the type
				while (token3 != NULL)
				{
					token3 = strtok_r(NULL, ":\n", &token3_end); //This should be the column type

					if(token3 == NULL)
						break;
												
					//Check whether it's an int or char type or neither							
					if (strcmp(token3, "int") == 0) //type is an int
					{
						params->tablepara[i].column_types[columncount] = -1;
						columncount++;
						//printf("Type stored is %d.\n", params->tablepara[i].column_types[columncount]);						
					}
										
					else if (strncmp(token3,"char[",5) == 0) //type is a char[n] 
					{
						int charval;
						char* token3copy;
						token3copy = (char *)malloc (strlen(token3)+1);
						strcpy (token3copy, token3);
											
						//separating integer from char[n]
						char tempnumberstrA[10];
																		
						int a;
						int j = 0;
						for(a = 0; (token3copy[a]!= ']'); a++ )
						{
							if(token3copy[a] == '[')
							{
								a++;
								while (token3copy[a] == ' ')
									a++;
								break;
							}					
						}
											
						int kk;
						for (kk = a; (token3copy[kk]!= ']'); kk++ )
						{	
							int tn = (int)(token3copy[kk]);
							if (((tn >= 48) && (tn <= 57)) || (tn == 43) || (tn == 45)) //is a number or plus minus sign
							{
								tempnumberstrA[j]= token3copy[kk];
									j++;
							}	
							else
							{
								printf("\nError: Invalid parameters in configuration file.\n");
									return -1;
							}
						}
							j++;
						tempnumberstrA[j]= '\0';
						charval = atoi(tempnumberstrA);
											
						if (charval <= 0) //if the size of the array is negative or zero size array
						{
							printf("\nError: Invalid parameters in configuration file.\n");
									return -1;	
						}
						//Storing column type
						params->tablepara[i].column_types[columncount]= charval;
						//printf("Type stored is %d.\n", params->tablepara[i].column_types[columncount]);
							columncount++;
							free(token3copy);
					}			
					
					else //Wrong column type or no column type
					{
						printf("\nError: Invaid parameters in the configuration file.\n");
							return -1;
					}
					token3count++;						
					
				}
				if (token3count != 2) //Accounting for invalid commas, dividing up name and type
				{
					printf("\nError: Invaid format in the configuration file.\n");
						return -1;
				} 
			}
			
			else if (token2 != NULL) //If spaces exist in between
			{
				printf("\nError: Invaid format in the configuration file.\n");
					return -1;
			}
			free(token2copy);
		}
		/////////////////////////////////////////////////////////////////////
		
		//accounting for extra commas
		//printf("Token count is %d and comma count is %d.\n", tokencount, commacount);
		if (tokencount - commacount != 3)
		{
			printf("\nError: Invalid format in the configuration file.\n");
				return -1;
		}
		
		//freeing malloc memories
		free(token_endcopy);
		free(tokenBcopy);
	}
	
	/// else if (strcmp(name, "data_directory") == 0) 
	///{
	///	strncpy(params->data_directory, value, sizeof params->data_directory);
	///} 
	
	else  
	{
		return 0;/// Ignore unknown config parameters.
	}

	return 0;
}

/**
 * @brief Read and load configuration parameters.
 *
 * @param config_file The name of the configuration file.
 * @param params The structure where config parameters are loaded.
 * @return Return 0 on success, -1 otherwise.
 */
int read_config(const char *config_file, struct config_params *params)
{
	
	int error_occurred = 0;
	params->duplicateparam = false;
	params->duplicatetable = false;
	/// Open file for reading.
	FILE* input;
	input = fopen(config_file, "r");
	
	if (input == NULL)
	{
		
	printf("\nError: Unable to read configuration file.\n");
		error_occurred = 1;
		errno = ERR_UNKNOWN;
		return -1;		
	}

	else
	{
	/// Process the config file.
	while (!error_occurred && !feof(input)) 
		{
			/// Read a line from the file.
			char line[MAX_CONFIG_LINE_LEN];
			char *l = fgets(line, sizeof line, input);			
			
			if (l == line) ///right size, proceed to process line
			{
				int process = process_config_line(line, params);
				
				if (params->duplicateparam == true)
				{
					printf("\nError: Duplicate parameters in configuration file.\n");
					error_occurred = 1;
					return -1;
				}
				
				if (params->duplicatetable == true)
				{
					printf("\nError: Duplicate table names in configuration file.\n");
					error_occurred = 1;
					return -1;
				}
				
				if (process == -1)
				{
					//printf("\nError: Invalid parameters in configuration file.\n");
					error_occurred = 1;
					return -1;
				}
					
			}
			
			else if (!feof(input)) ///not the right size
			{
				printf("\nError: Configuration file too large.\n");
				error_occurred = 1;
				return -1;
			}
		}
				
	if((params->server_host[0] == NULL) || (params->concurrency == -1) || (params->server_port == 0) || (params->username[0] == NULL) || (params->pass_[0] == NULL) || (params->table_name[0][0] == '\0'))
		{
			printf("\nError: Missing parameters in configuration file.\n");
			error_occurred = 1;
			return -1;
		}
		
	}
	/*
	//testing: printing out parameters
	printf("%s\n", params->server_host);
	printf("%d\n", params->server_port);
	printf("%s\n", params->username);
	printf("%s\n", params->pass_);
	
	int i;
	for (i = 0; i< MAX_TABLES; i++)
		{	
			if (params->table_name[i][0] == '\0')
				break;
			
			printf("%s\n", params->table_name[i]);
		}
	
	int j, k;	
	for (j = 0; j< MAX_TABLES; j++)
		{
			if (params->tablepara[j].table_name[0] == '\0')
				break;
				
			printf("%s\n", params->tablepara[j].table_name);
			for (k = 0; k< MAX_COLUMNS_PER_TABLE; k++)
			{
				if (params->tablepara[j].table_columns[k][0] == '\0')
					break;
				printf("%s\n", params->tablepara[j].table_columns[k]);
				printf("%d\n", params->tablepara[j].column_types[k]);
			}
			
		}
	*/			
			
	fclose(input);
	return 0;
}

/**
 * @brief Generates a log message.
 * 
 * @param file The output stream
 * @param message Message to log.
 */
void logger(FILE *file, char *message)
{
	if(!file)
	return;
	fprintf(file,"%s",message);
	fflush(file);
}
/**
 * @brief Generates an encrypted password string using salt CRYPT_SALT.
 * 
 * @param passwd Password before encryption.
 * @param salt Salt used to encrypt the password. If NULL default value
 * DEFAULT_CRYPT_SALT is used.
 * @return Returns encrypted password.
 */
char *generate_encrypted_password(const char *passwd, const char *salt)
{
	if(salt != NULL)
		return crypt(passwd, salt);
	else
		return crypt(passwd, DEFAULT_CRYPT_SALT);
}

/**
 * @brief This is a function that parses the query command from client side to be used in server side
 * 
 * @param unparsed_ pointer to the unparsed string
 * @param query_params *qparams pointer to the struct where query parameters are stored
 * @return pointer to the parsed string
 */
char* query_parsing(char* unparsed_, struct query_params *qparams)
{
    char* unparsed = (char *)malloc (strlen(unparsed_)+1); 
    strcpy(unparsed, unparsed_);
    char *token, *token_end;
    int columncount = 0;
    
    //looping through to count number of commas
    int cc;
    int commacount = 0;
        for(cc = 0; (unparsed[cc]!= '\0'); cc++ )
	{
                int ccv = (int)(unparsed[cc]);
		if (ccv == 44)
			commacount++;
	}     
    
    token = strtok_r(unparsed, ",\0", &token_end); //first predicate
    //printf("%s1\n", token);
    int tokencount = 1; //counting predicates
    
    if (token == NULL) //empty string
        return NULL;
    
    //First predicate parsing starts here
    //cutting out first predicate
    char* tokencopy = (char *)malloc (strlen(token)+1); //first predicate full
    strcpy(tokencopy, token);
    
    //column name parsing starts here
    char *tokenA, *tokenA_end;
    tokenA = strtok_r(tokencopy, "<=>", &tokenA_end); //column name
    //printf("%s2\n", tokenA);
    
    //cutting white space for column name
    char *tokenB, *tokenB_end;
        tokenB = strtok_r(tokenA, " \0", &tokenB_end); //Cutting out white spaces
	//printf("%s3\n", tokenB);
		
	char*tokenBcopy = (char *)malloc (strlen(tokenB)+1);
	strcpy (tokenBcopy, tokenB);				
						
	tokenB = strtok_r(NULL, " \0", &tokenB_end); //Checking whether column name has spaces in it
						
	if (tokenB == NULL) //Meaning there's no spaces in column name
        {
           //check whether column name is alphanumeric
            int alp;
            for (alp = 0; alp< strlen(tokenBcopy); alp++)
            {
                    int alpv = (int)(tokenBcopy[alp]);
                    if(((alpv >= 48) && (alpv <= 57)) || ((alpv >= 65) && (alpv <= 90)) || ((alpv >= 97) && (alpv <= 122)))
                            ;
                    else
                        return NULL;
            } 
            //if it's okay, store the column name
            strncpy (qparams->column_names[columncount], tokenBcopy, MAX_COLNAME_LEN); 
            //printf("Column name stored is %s.\n", qparams->column_names[columncount]);
        }
        else if (tokenB != NULL) //there's white spaces in column name
            return NULL;
  
    //column name parsing ends here
        
    //operator parsing starts here
    //looping through to search for operators
    int o;
    int ocount = 0;
    char operator;
        for(o = 0; (token[o]!= '\0'); o++ )
	{
                int ov = (int)(token[o]);
		if ((ov == 60) || (ov == 61) || (ov == 62))
                {
                    operator = token[o];
		    ocount++;
                }
	}    
    if (ocount == 0)
        return NULL;
    if (ocount != 1)
        return NULL;
    //if it's right store the operator
    qparams->operator[columncount] = operator;
    //printf("Operator stored is %c.\n", qparams->operator[columncount]);
    //operator parsing ends here
    
    //value parsing starts here
    //tokenA_end is the string after the operator
    
    //cutting out white spaces beginning and end and storing into a new string array
    char valueA[MAX_VALUE_LEN]; 
    memset(valueA,0,sizeof(valueA));
    int p;
    int alphacount = 0;
    for (p= 0; (tokenA_end[p]!='\0'); p++) //cutting space in the beginning
    {
        int pv = (int)(tokenA_end[p]);
        if (((pv >= 48) && (pv <= 57)) || ((pv >= 65) && (pv <= 90)) || ((pv >= 97) && (pv <= 122)) || (pv == 32) || (pv == 43) || (pv == 45)) //alphanumeric or plus minus
        {
            if ((pv != 32) && (alphacount == 0))//beginning alphanumeric
            {
                valueA[alphacount] = tokenA_end[p];
                alphacount++;
            }
            else if(alphacount > 0)
            {
                valueA[alphacount] = tokenA_end[p];
                alphacount++;
            }
        }
        else //not alphanumeric
                return NULL;
     }
    if (alphacount == 0)
        return NULL;
    
    //printf("%shi\n", valueA);
    char* value = valueA;
    while (value[strlen(value)-1] == ' ')
        value[strlen(value)-1] = '\0';
    
    //printf("lala%slala\n", value);
    
    //after parsing store the value
    strncpy (qparams->value[columncount], value, MAX_VALUE_LEN); 
    //printf("Value stored is %s.\n", qparams->value[columncount]);
    //value parsing ends here
    columncount++;
    ///First predicate parsing ends here    
    
    ////////////////////////////////////////////////////////////////////
    ///Next predicates parsing start here
    //moving onto next predicates
    while (token != NULL)
    {
        token = strtok_r(NULL, ",\0", &token_end); //full predicate
        if (token == NULL)
            break;
        tokencount++;
        //printf("%s4\n", token);
        
        char*tokenstar = (char *)malloc (strlen(token)+1); //full predicate that's not cut
	strcpy (tokenstar, token);
        
        //column name parsing starts here
        char *token1, *token1_end;
        token1 = strtok_r(token, "<=>", &token1_end); //column name
        //printf("%s5\n", token1);

        //cutting white space for column name
        char *token2, *token2_end;
        token2 = strtok_r(token1, " \0", &token2_end); //Cutting out white spaces
	//printf("%s6\n", token2);
		
	char*token2copy = (char *)malloc (strlen(token2)+1);
	strcpy (token2copy, token2);				
						
	token2 = strtok_r(NULL, " \0", &token2_end); //Checking whether column name has spaces in it
						
	if (token2 == NULL) //Meaning there's no spaces in column name
        {
           //check whether column name is alphanumeric
            int al;
            for (al = 0; al< strlen(token2copy); al++)
            {
                    int alv = (int)(token2copy[al]);
                    if(((alv >= 48) && (alv <= 57)) || ((alv >= 65) && (alv <= 90)) || ((alv >= 97) && (alv <= 122)))
                            ;
                    else
                        return NULL;
            } 
            //if it's okay, store the column name
            strncpy (qparams->column_names[columncount], token2copy, MAX_COLNAME_LEN); 
            //printf("Column name stored is %s.\n", qparams->column_names[columncount]);
        }
        else if (token2 != NULL) //there's white spaces in column name
            return NULL;
   
    //column name parsing ends here
        
    //operator parsing starts here
    //looping through to search for operators
       
    int op;
    int ocountt = 0;
    char operatorr;
        for(op = 0; (tokenstar[op]!= '\0'); op++ )
	{
                int ovv = (int)(tokenstar[op]);
		if ((ovv == 60) || (ovv == 61) || (ovv == 62))
                {
                    operatorr = tokenstar[op];
		    ocountt++;
                }
	}    
    if (ocountt == 0)
        return NULL;
    if (ocountt != 1)
        return NULL;
    //if it's right store the operator
    qparams->operator[columncount] = operatorr;
    //printf("Operator stored is %c.\n", qparams->operator[columncount]);
    //operator parsing ends here
    
    //value parsing starts here
    //token1_end is the string after the operator
    
    //cutting out white spaces beginning and end and storing into a new string array
    char valueB[MAX_VALUE_LEN];    
    int pk;
    int alphacount2 = 0;
    for (pk= 0; (token1_end[pk]!='\0'); pk++) //cutting space in the beginning
    {
        int pkv = (int)(token1_end[pk]);
        if (((pkv >= 48) && (pkv <= 57)) || ((pkv >= 65) && (pkv <= 90)) || ((pkv >= 97) && (pkv <= 122)) || (pkv == 32) || (pkv == 43) || (pkv == 45)) //alphanumeric or plus minus
        {
            if ((pkv != 32) && (alphacount2 == 0))//beginning alphanumeric
            {
                valueB[alphacount2] = token1_end[pk];
                alphacount2++;
            }
            else if(alphacount2 > 0)
            {
                valueB[alphacount2] = token1_end[pk];
                alphacount2++;
            }
        }
        else //not alphanumeric
                return NULL;
     }
    if (alphacount2 == 0)
        return NULL;
    
    //printf("%shi\n", valueB);
    char* value2 = valueB;
    while (value2[strlen(value2)-1] == ' ')
        value2[strlen(value2)-1] = '\0';
    
    //printf("lala%slala\n", value2);
    
    //after parsing store the value
    strncpy (qparams->value[columncount], value2, MAX_VALUE_LEN); 
    //printf("Value stored is %s.\n", qparams->value[columncount]);
    //value parsing ends here
    columncount++;
    ///predicate parsing ends here
  
    free(tokenstar);
    free(token2copy);   
    }
    ///////////////////////////////////////////////////////////////////
    ///Next predicates parsing end here
    
    //if the commas don't match up the number of predicates
    if ((tokencount - commacount) != 1)
        return NULL;
    //free malloc memories
    free(tokencopy);
    free(tokenBcopy);
    free(unparsed);
    
    /////////////////////////////////////////////////////////////////
    ///Putting it together in a string
    char big[3000];
    char* bigs = big;
    char* space = " ";
    char* comma = ",";
    
    int b;
    for (b = 0; b < MAX_COLUMNS_PER_TABLE; b++)
    {
        if (qparams->column_names[b][0] == '\0')
            break;
        
        char* column_name = qparams->column_names[b];
                strcat(bigs, column_name);
                strcat(bigs, space);
                
                strncat (bigs, &(qparams->operator[b]), 1);
                strcat(bigs, space);
                
        char* value = qparams->value[b];
                strcat(bigs, value);
                strcat(bigs, comma);       
    }
    bigs[strlen(bigs)-1] = '\0';    
    
    //printf("%shi\n", bigs);
    
    char* parsed = bigs;
    return parsed;
}

/**
 * @brief This is a function that parses the set command from client side to be used in server side
 * 
 * @param unparsed_ pointer to the unparsed string
 * @param set_params *sparams pointer to the struct where set parameters are stored
 * @return pointer to the parsed string
 */
char* set_parsing(char* unparsed_, struct set_params *sparams)
{
    char* unparsed = (char *)malloc (strlen(unparsed_)+1); 
    strcpy(unparsed, unparsed_);
    char *token, *token_end;
    int columncount = 0;
    
    //looping through to count number of commas
    int cc;
    int commacount = 0;
        for(cc = 0; (unparsed[cc]!= '\0'); cc++ )
	{
                int ccv = (int)(unparsed[cc]);
		if (ccv == 44)
			commacount++;
	}     
    
    token = strtok_r(unparsed, ",\0", &token_end); //first predicate
    //printf("%s1\n", token);
    int tokencount = 1; //counting predicates
    
    if (token == NULL) //empty string
        return NULL;
    
    //First predicate parsing starts here
    //cutting out first predicate
    char* tokencopy = (char *)malloc (strlen(token)+1); //first predicate full
    strcpy(tokencopy, token);
    
    //column name parsing starts here
    char *tokenA, *tokenA_end;
    tokenA = strtok_r(tokencopy, " \0", &tokenA_end); //column name
    //printf("%s2\n", tokenA);
    
    //cutting white space for column name
    char *tokenB, *tokenB_end;
        tokenB = strtok_r(tokenA, " \0", &tokenB_end); //Cutting out white spaces
	//printf("%s3\n", tokenB);
		
	char*tokenBcopy = (char *)malloc (strlen(tokenB)+1);
	strcpy (tokenBcopy, tokenB);				
						
	tokenB = strtok_r(NULL, " \0", &tokenB_end); //Checking whether column name has spaces in it
						
	if (tokenB == NULL) //Meaning there's no spaces in column name
        {
           //check whether column name is alphanumeric
            int alp;
            for (alp = 0; alp< strlen(tokenBcopy); alp++)
            {
                    int alpv = (int)(tokenBcopy[alp]);
                    if(((alpv >= 48) && (alpv <= 57)) || ((alpv >= 65) && (alpv <= 90)) || ((alpv >= 97) && (alpv <= 122)))
                            ;
                    else
                        return NULL;
            } 
            //if it's okay, store the column name
            strncpy (sparams->column_names[columncount], tokenBcopy, MAX_COLNAME_LEN); 
            //printf("Column name stored is %s.\n", qparams->column_names[columncount]);
        }
        else if (tokenB != NULL) //there's white spaces in column name
            return NULL;
  
    //column name parsing ends here
    
    //value parsing starts here
    //tokenA_end is the string after the operator
    
    //cutting out white spaces beginning and end and storing into a new string array
    char valueA[MAX_VALUE_LEN]; 
    memset(valueA,0,sizeof(valueA));
    int p;
    int alphacount = 0;
    for (p= 0; (tokenA_end[p]!='\0'); p++) //cutting space in the beginning
    {
        int pv = (int)(tokenA_end[p]);
        if (((pv >= 48) && (pv <= 57)) || ((pv >= 65) && (pv <= 90)) || ((pv >= 97) && (pv <= 122)) || (pv == 32) || (pv == 43) || (pv == 45)) //alphanumeric or plus minus
        {
            if ((pv != 32) && (alphacount == 0))//beginning alphanumeric
            {
                valueA[alphacount] = tokenA_end[p];
                alphacount++;
            }
            else if(alphacount > 0)
            {
                valueA[alphacount] = tokenA_end[p];
                alphacount++;
            }
        }
        else //not alphanumeric
                return NULL;
     }
    if (alphacount == 0)
        return NULL;
    
    //printf("%shi\n", valueA);
    char* value = valueA;
    while (value[strlen(value)-1] == ' ')
        value[strlen(value)-1] = '\0';
    
    //printf("lala%slala\n", value);
    
    //after parsing store the value
    strncpy (sparams->value[columncount], value, MAX_VALUE_LEN); 
    //printf("Value stored is %s.\n", qparams->value[columncount]);
    //value parsing ends here
    columncount++;
    ///First predicate parsing ends here    
    
    ////////////////////////////////////////////////////////////////////
    ///Next predicates parsing start here
    //moving onto next predicates
    
    while (token != NULL)
    {
        token = strtok_r(NULL, ",\0", &token_end); //full predicate
        if (token == NULL)
            break;
        tokencount++;
        //printf("%s4\n", token);
        
        char*tokenstar = (char *)malloc (strlen(token)+1); //full predicate that's not cut
	strcpy (tokenstar, token);
        
        //column name parsing starts here
        char *token1, *token1_end;
        token1 = strtok_r(token, " \0", &token1_end); //column name
        //printf("%s5\n", token1);

        //cutting white space for column name
        char *token2, *token2_end;
        token2 = strtok_r(token1, " \0", &token2_end); //Cutting out white spaces
	//printf("%s6\n", token2);
		
	char*token2copy = (char *)malloc (strlen(token2)+1);
	strcpy (token2copy, token2);				
						
	token2 = strtok_r(NULL, " \0", &token2_end); //Checking whether column name has spaces in it
						
	if (token2 == NULL) //Meaning there's no spaces in column name
        {
           //check whether column name is alphanumeric
            int al;
            for (al = 0; al< strlen(token2copy); al++)
            {
                    int alv = (int)(token2copy[al]);
                    if(((alv >= 48) && (alv <= 57)) || ((alv >= 65) && (alv <= 90)) || ((alv >= 97) && (alv <= 122)))
                            ;
                    else
                        return NULL;
            } 
            //if it's okay, store the column name
            strncpy (sparams->column_names[columncount], token2copy, MAX_COLNAME_LEN); 
            //printf("Column name stored is %s.\n", qparams->column_names[columncount]);
        }
        else if (token2 != NULL) //there's white spaces in column name
            return NULL;
    
    //value parsing starts here
    //token1_end is the string after the operator
    
    //cutting out white spaces beginning and end and storing into a new string array
    char valueB[MAX_VALUE_LEN]; 
    memset(valueB,0,sizeof(valueB));
    int pk;
    int alphacount2 = 0;
    for (pk= 0; (token1_end[pk]!='\0'); pk++) //cutting space in the beginning
    {
        int pkv = (int)(token1_end[pk]);
        if (((pkv >= 48) && (pkv <= 57)) || ((pkv >= 65) && (pkv <= 90)) || ((pkv >= 97) && (pkv <= 122)) || (pkv == 32) || (pkv == 43) || (pkv == 45)) //alphanumeric or plus minus
        {
            if ((pkv != 32) && (alphacount2 == 0))//beginning alphanumeric
            {
                valueB[alphacount2] = token1_end[pk];
                alphacount2++;
            }
            else if(alphacount2 > 0)
            {
                valueB[alphacount2] = token1_end[pk];
                alphacount2++;
            }
        }
        else //not alphanumeric
                return NULL;
     }
    if (alphacount2 == 0)
        return NULL;
    
    //printf("%shi\n", valueB);
    char* value2 = valueB;
    while (value2[strlen(value2)-1] == ' ')
        value2[strlen(value2)-1] = '\0';
    
    //printf("lala%slala\n", value2);
    
    //after parsing store the value
    strncpy (sparams->value[columncount], value2, MAX_VALUE_LEN); 
    //printf("Value stored is %s.\n", qparams->value[columncount]);
    //value parsing ends here
    columncount++;
    ///predicate parsing ends here
  
    free(tokenstar);
    free(token2copy);   
    }
    ///////////////////////////////////////////////////////////////////
    ///Next predicates parsing end here
    
    //if the commas don't match up the number of predicates
    if ((tokencount - commacount) != 1)
        return NULL;
    //free malloc memories
    free(tokencopy);
    free(tokenBcopy);
    free(unparsed);
    
    /////////////////////////////////////////////////////////////////
    ///Putting it together in a string
    char big[3000];
    memset(big,0,sizeof(big));
    char* bigs = big;
    char* space = " ";
    char* comma = ",";
    
    int b;
    for (b = 0; b < MAX_COLUMNS_PER_TABLE; b++)
    {
        if (sparams->column_names[b][0] == '\0')
            break;
        
        char* column_name = sparams->column_names[b];
                strcat(bigs, column_name);
                strcat(bigs, space);
                
        char* value = sparams->value[b];
                strcat(bigs, value);
                strcat(bigs, comma);       
    }
    bigs[strlen(bigs)-1] = '\0';  
    
    //printf("%shi\n", bigs);
    
    char* parsed = bigs;
    return parsed;
}




