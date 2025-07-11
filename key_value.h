/** !
 * Header file for key value database
 * 
 * @file key_value.h
 * 
 * @author Jacob Smith
 */

// standard library
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

// core
#include <core/log.h>
#include <core/socket.h>

// data
#include <data/array.h>
#include <data/dict.h>
#include <data/tree.h>
#include <data/binary.h>

// reflection
#include <reflection/json.h>

// performance
#include <performance/thread_pool.h>

// preprocessor definitions
#define KEY_VALUE_DB_PROPERTY_KEY_LENGTH_MAX   31  + 1
#define KEY_VALUE_DB_PROPERTY_VALUE_LENGTH_MAX 975 + 1
#define KEY_VALUE_DB_NODE_SIZE_BYTES           (KEY_VALUE_DB_PROPERTY_VALUE_LENGTH_MAX + KEY_VALUE_DB_PROPERTY_KEY_LENGTH_MAX)

// structure declarations
struct key_value_db_s
{
    binary_tree *p_binary_tree;
    socket_tcp   _socket;
    thread_pool *p_thread_pool;
    char         _database_file[FILENAME_MAX];
};

struct key_value_db_property_s
{
    char        _key[KEY_VALUE_DB_PROPERTY_KEY_LENGTH_MAX];
    char        _value[KEY_VALUE_DB_PROPERTY_VALUE_LENGTH_MAX];
    json_value *p_value;
};

// type definitions
typedef struct key_value_db_s          key_value_db;
typedef struct key_value_db_property_s key_value_db_property;

// function declarations
// allocators
/** !
 * Allocate a key value database
 * 
 * @param pp_key_value the key value database
 * 
 * @return 1 on success, 0 on error
 */
int key_value_db_create ( key_value_db **pp_key_value );

// constructors
int key_value_db_construct ( key_value_db **pp_key_value_db, const char *p_database_file, unsigned short port );

// property
int key_value_db_put ( key_value_db *p_key_value_db, char *p_output, char *p_key, json_value *p_value );
int key_value_db_get ( key_value_db *p_key_value_db, char *p_output, char *p_key );

// write
int key_value_db_write ( key_value_db *p_key_value_db, const char *p_path );

// parse statement
int key_value_db_parse_statement ( key_value_db *p_key_value, char *p_input, char *p_output );