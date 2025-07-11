/** !
 * Key-Value database shell
 * 
 * @file main.c
 * 
 * @author Jacob Smith
 */

// Standard library
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

// core
#include <core/log.h>

// performance
#include <performance/thread.h>

// data module
#include "key_value.h"

// Preprocessor definitions
#define KEY_VALUE_DB_SHELL_INPUT_MAX  1024
#define KEY_VALUE_DB_SHELL_PROMPT_MAX 128

/** !
 * Initialize the key_value database 
 * 
 * @param void
 * 
 * @return 1 on success, 0 on error
 */
int key_value_db_early ( void );

/** !
 * Print the prompt to standard out
 * 
 * @param void
 * 
 * @return 1 on success, 0 on error
 */
int print_prompt ( void );

int key_get ( const char *p_key, json_value *p_value )
{

    if ( p_value == 0 ) return 0;
    if ( p_value->type != JSON_VALUE_STRING ) return 0;
    if ( p_value->string == 0 ) return 0;
    if ( strcmp(p_value->string, p_key) ) return 0;

    // Success
    return 1;
}

// Forward declarations
/** !
 * Print a usage message to standard out
 * 
 * @param argv0 the name of the program
 * 
 * @return void
 */
void print_usage ( const char *argv0 );

/** !
 * Parse command line arguments program
 * 
 * @param argc             the argc parameter of the entry point
 * @param argv             the argv parameter of the entry point
 * 
 * @return 1 on success, 0 on error
 */
int parse_arguments ( int argc, const char *argv[] );

/** !
 * Listen and serve
 * 
 * @param p_key_value_database the key value database
 * 
 * @return 1 on success, 0 on error
 */
void *listen_and_serve ( void *p_key_value_database );

// data
unsigned short   port              = 6713;
const char      *p_database_file   = NULL;
parallel_thread *p_listener_thread = NULL;
char _prompt[KEY_VALUE_DB_SHELL_PROMPT_MAX] = { ' ', '$', '?', ' ', ' ', '$', '$', ' ', '\0' };
int last_result = 1;
key_value_db *p_key_value_db = (void *) 0;

// entry point
int main ( int argc, const char *argv[] )
{

    // unused
    (void) argc;
    (void) argv;

    // initialized data
    char _input[512], _output[512];

    // parse command line arguments
    parse_arguments(argc, argv);

    // construct the database
    key_value_db_construct(&p_key_value_db, p_database_file, port);

    // start a listener thread
    parallel_thread_start(&p_listener_thread, listen_and_serve, NULL);

    // read, evaluate, print loop
    while( !feof(stdin) )
    {
        
        // clear buffers
        memset(_input, 0, sizeof(_input)),
        memset(_output, 0, sizeof(_output));

        // prompt
        print_prompt();

        // read a line from standard in
        fgets(_input, KEY_VALUE_DB_SHELL_INPUT_MAX, stdin);

        // add a null terminator
        _input[strlen(_input) - 1] = '\0';

        // continuation condition
        if ( strncmp(_input, "exit", KEY_VALUE_DB_SHELL_PROMPT_MAX) == 0 ) break;
        
        // evaluate            
        key_value_db_parse_statement(p_key_value_db, _input, _output);

        // print
        printf("%s", _output);
    }

    // formatting
    putchar('\r');

    // success
    return EXIT_SUCCESS;
}

void print_usage ( const char *argv0 )
{

    // argument check
    if ( argv0 == (void *) 0 ) exit(EXIT_FAILURE);

    // print a usage message to standard out
    printf("Usage: %s [-f <database_file>] [-p <port>]\n", argv0);

    // done
    return;
}

int parse_arguments ( int argc, const char *argv[] )
{

    // create a temporary file name
    p_database_file = tmpnam(0);

    // iterate through command line arguments
    for ( int i = 1; i < argc; i++ )
    {

        // parse the port number
        if 
        (
            0 == strcmp(argv[i], "-p") ||
            0 == strcmp(argv[i], "--port")
        )
        {

            // error check
            if ( i == argc - 1 ) print_usage(argv[0]);

            // parse the port number
            port = strtol(argv[++i], 0, 10);
        }

        // parse the file path
        else if 
        (
            0 == strcmp(argv[i], "-f") ||
            0 == strcmp(argv[i], "--file")
        )
        {

            // error check
            if ( i == argc - 1 ) print_usage(argv[0]);

            // parse the port number
            p_database_file = argv[++i];
        }
    }

    // success
    return 1;

    // Error handling
    {

        // Argument errors
        {

            no_database_path:
                #ifndef NDEBUG
                    log_error("[key value] Null pointer provided for parameter \"pp_database_path\" in call to function \"%s\"\n", __FUNCTION__);
                #endif

                // error
                return 0;

            no_port:
                #ifndef NDEBUG
                    log_error("[key value] Null pointer provided for parameter \"p_port\" in call to function \"%s\"\n", __FUNCTION__);
                #endif

                // error
                return 0;

            missing_command_line_arguments:
                
                // Print a usage message to standard out
                print_usage(argv[0]);

                // Abort execution
                exit(EXIT_FAILURE);
        }
    }
}

int key_value_db_early ( void )
{

    // Initialized data
    size_t  prompt_len = 0;
    char   *p_prompt = getenv("KEY_VALUE_DB_PROMPT");

    // Error check
    if ( p_prompt == NULL ) goto no_prompt;
    
    // Compute the length of the prompt
    prompt_len = strlen(p_prompt);

    // Copy the prompt
    strncpy(_prompt, p_prompt, prompt_len);

    // Success
    return 1;

    no_prompt:

        // Set the prompt to the default
        putenv("KEY_VALUE_DB_PROMPT=\"[ $? ] > \"");

        // Done
        return 1;
}

int print_prompt ( void )
{

    // initialized data
    size_t i = 0;

    // print the prompt
    while ( _prompt[i] )
    {
        
        // Ordinary characters ...
        if ( _prompt[i] != '$' )

            // ... get printed to standard out
            putchar(_prompt[i]);

        // Handle escape sequence
        else
        {

            // Strategy
            switch ( _prompt[i + 1] )
            {

                // Return code
                case '?':

                    // Print the return code
                    printf("%s", last_result ? "\033[92m✔\033[0m" : "\033[91m✘\033[0m");
         
                    // Done
                    break;

                // Dollar sign
                case '$':

                    // Print the return code
                    putchar('$');

                    // Done
                    break;

                // Erroneous prompt
                case '\0':
                default:

                    // Abort
                    exit(EXIT_FAILURE);                
            }

            // Skip
            i++;
        }

        // Increment counter
        i++;
    }
    
    // flush
    fflush(stdout);

    // success
    return 1;
}

void *network_shell ( void *_socket )
{
    
    // initialized data
    socket_tcp _socket_tcp = (socket_tcp) _socket;
    char _buf[512] = { 0 }, _out[1024] = { 0 };
    int r = 1;
    size_t len = 0;

    // Reprint the prompt
    print_prompt();
    
    // REPL 
    while ( true )
    {

        // Clear the input buffer
        memset(_buf, 0, sizeof(_buf));
        memset(_out, 0, sizeof(_out));

        int z = socket_tcp_receive(_socket_tcp, _buf, sizeof(_buf));
        
        // receive data from the socket
        if ( z == 0 )
        {

            // print the disconnect
            printf("\r\033[44m\033[[[[[DISCONNECTED]]]\033[0m\n");

            // reprint the prompt
            print_prompt();

            // done
            break;
        }

        // evaluate
        r = key_value_db_parse_statement(p_key_value_db, (char *)_buf, (char *)_out);

        // error check
        if ( r == -1 )
        {
            
            // close the connection
            socket_tcp_send(_socket_tcp, "", 0);
            socket_tcp_send(_socket_tcp, "Bye bye!", 8);
            socket_tcp_destroy(&_socket_tcp);

            // log the disconnect
            printf("\r\033[44m\033[[[[[DISCONNECTED]]]\033[0m\n");

            // reprint the prompt
            print_prompt();

            
            // Done
            break;
        }

        len = strlen(_out);
        _out[len]     = '\n',
        _out[len + 1] = '\0',
        

        // Print
        socket_tcp_send(_socket_tcp, _out, sizeof(_out));
    } 

}

int accept_client ( socket_tcp _socket_tcp, socket_ip_address ip_address, socket_port port_number, void *const p_parameter )
{

    // initialized data
    key_value_db *p_key_value_db = (key_value_db *)p_parameter;
    unsigned char a = (ip_address & 0xff000000) >> 24,
                  b = (ip_address & 0x00ff0000) >> 16,
                  c = (ip_address & 0x0000ff00) >> 8,
                  d = (ip_address & 0x000000ff) >> 0;
    char _buf[1024] = { 0 };
    char _out[4096] = { 0 };
    int r = 1;
    
    // Log the IP
    printf("\r\033[44m\033[[[[[%d.%d.%d.%d:%d CONNECTED]]]\033[0m\n", a, b, c, d, port);

    thread_pool_execute(p_key_value_db->p_thread_pool, network_shell, (void *)_socket_tcp);

    // Success
    return 1;
}

void *listen_and_serve ( void *unused )
{

    // unused
    (void) unused;

    // log
    log_info("\r[key value db] Listening on :%hd\n", port),
    print_prompt();

    // forever...
    while (true)
        socket_tcp_listen(p_key_value_db->_socket, accept_client, p_key_value_db);

    // success
    return (void *)1;

}
