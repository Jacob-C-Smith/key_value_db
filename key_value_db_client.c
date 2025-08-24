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

/// performance
#include <performance/connection.h>

// db
#include <key_value/key_value.h>

// forward declarations
/** !
 * Print a usage message to standard out
 * 
 * @param argv0 the name of the program
 * 
 * @return void
 */
void print_usage ( const char *argv0 );

/** !
 * Parse command line arguments
 * 
 * @param argc            the argc parameter of the entry point
 * @param argv            the argv parameter of the entry point
 * 
 * @return void on success, program abort on failure
 */
void parse_command_line_arguments ( int argc, const char *argv[] );

// data
unsigned short  port       = 6708;
const char     *p_hostname = "localhost";

// entry point
int main ( int argc, const char *argv[] )
{

    // initialized data
    connection *p_connection = NULL;
    char        _net_buffer[4096] = { 0 };
    char        _stdin_buffer[4096] = { 0 };
    char        _res_buffer[4096] = { 0 };
    size_t      size         = 0;

    // parse command line arguments
    parse_command_line_arguments(argc, argv);

    // log connection 
    log_info("Connecting to db server at %s:%hu\n", p_hostname, port);

    // connect to the server
    if ( 0 == connection_construct(&p_connection, p_hostname, port) ) goto no_connection;

    // repl
    while ( 0 == feof(stdin) )
    {

        // initialized data
        size_t len = 0;
        size_t input_len = 0;

        // read a line from stdin
        fgets(_stdin_buffer, sizeof(_stdin_buffer), stdin);

        // done?
        if ( feof(stdin) ) break;

        // compute the input length
        input_len = strlen(_stdin_buffer) - 1;

        // store a null terminator at the end of the input
        _stdin_buffer[input_len] = '\0';

        // prepend the length of the message
        *(size_t *)_net_buffer = input_len;

        // copy the message into the buffer 
        strncpy(_net_buffer + sizeof(size_t), _stdin_buffer, input_len);
        len = input_len;

        // send
        connection_write(p_connection, _net_buffer, sizeof(size_t) + len);

        // receive
        connection_read(p_connection, _res_buffer, &size);

        // logs
        log_info("Received: %s\n", _res_buffer);

        // done?
        if ( 0 == strcmp(_stdin_buffer, "exit") ) break;
    }

    // success
    return EXIT_SUCCESS;

    // error handling
    {
        no_connection:
            #ifndef NDEBUG
                log_error("Error: Failed to connect to %s:%hu\n", p_hostname, port);
            #endif

            // error
            return EXIT_FAILURE;
    }
}

void print_usage ( const char *argv0 )
{

    // argument check
    if ( argv0 == (void *) 0 ) exit(EXIT_FAILURE);

    // Print a usage message to standard out
    printf("Usage: %s [-p | --port <port>] [-h | --host <hostname>] \n", argv0);

    // done
    return;
}

void parse_command_line_arguments ( int argc, const char *argv[] )
{
    
    // iterate through each command line argument
    for (size_t i = 1; i < (size_t) argc; i++)
    {
        
        // port?
        if
        ( 
            0 == strcmp(argv[i], "-p")     ||
            0 == strcmp(argv[i], "--port")
        )
        {

            // set the port number
            if ( 1 != sscanf(argv[++i], "%hu", &port) ) goto invalid_arguments;
        }

        // host name?
        else if
        ( 
            0 == strcmp(argv[i], "-h")     ||
            0 == strcmp(argv[i], "--host")
        )
            
            // set the host name
            p_hostname = argv[++i];
    }
    
    // success
    return;

    // error handling
    {

        // argument errors
        {
            invalid_arguments:
                
                // Print a usage message to standard out
                print_usage(argv[0]);

                // Abort
                exit(EXIT_FAILURE);
        }
    }
}
