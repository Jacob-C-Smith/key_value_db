/** !
 * Header file for key value database
 * 
 * @file key_value.h
 * 
 * @author Jacob Smith
 */

// Standard library
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

// core
#include <core/log.h>

// data
#include <data/array.h>
#include <data/dict.h>
#include <data/tree.h>
#include <data/binary.h>

// json module
#include <reflection/json.h>

// Preprocessor definitions
#define KEY_VALUE_DB_PROPERTY_KEY_LENGTH_MAX   31   + 1
#define KEY_VALUE_DB_PROPERTY_VALUE_LENGTH_MAX 975 + 1
#define KEY_VALUE_DB_NODE_SIZE_BYTES           (KEY_VALUE_DB_PROPERTY_VALUE_LENGTH_MAX + KEY_VALUE_DB_PROPERTY_KEY_LENGTH_MAX)

// Structure declarations
struct key_value_db_s;
struct key_value_db_property_s;

// Type definitions
typedef struct key_value_db_s          key_value_db;
typedef struct key_value_db_property_s key_value_db_property;

// Function declarations
// Allocators
/** !
 * Allocate a key value database
 * 
 * @param pp_key_value the key value database
 * 
 * @return 1 on success, 0 on error
 */
int key_value_db_create ( key_value_db **pp_key_value );

// Constructors
int key_value_db_construct ( key_value_db **pp_key_value_db, const char *p_database_file );

// Property
int key_value_db_put ( key_value_db *p_key_value_db, char *p_output, char *p_key, json_value *p_value );
int key_value_db_get ( key_value_db *p_key_value_db, char *p_output, char *p_key );

// Write
int key_value_db_write ( key_value_db *p_key_value_db, const char *p_path );

// Parse statement
int key_value_db_parse_statement ( key_value_db *p_key_value, char *p_input, char *p_output );