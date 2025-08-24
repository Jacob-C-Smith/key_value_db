/** !
 * key value database server
 *
 * @file key_value_db_server.c
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
#include <core/pack.h>

// db
#include <key_value/key_value.h>

// entry point
int main ( int argc, const char *argv[] )
{

    // initialized data
    key_value_db *p_key_value_db = NULL;

    // construct an db server
    key_value_db_construct(&p_key_value_db);

    // print the db server
    key_value_db_print(p_key_value_db);

    // keep network up
    getchar();

    // success
    return EXIT_SUCCESS;
}
