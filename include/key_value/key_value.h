/** !
 * Data 
 * 
 * @file data/data.h
 * 
 * @author Jacob Smith
 */

// standard library
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

// gsdk
#include <gsdk.h>

/// core
#include <core/log.h>
#include <core/socket.h>
#include <core/pack.h>

/// data
#include <data/array.h>
#include <data/binary.h>
#include <data/cache.h>

/// performance
#include <performance/thread_pool.h>

// structure declarations
struct key_value_db_s;
struct key_value_property_s;

// type definitions
typedef struct key_value_db_s       key_value_db;
typedef struct key_value_property_s key_value_property;

// forward declarations
/// constructors
int key_value_db_construct ( key_value_db **pp_db );

/// printers
int key_value_db_print ( key_value_db *p_db );
