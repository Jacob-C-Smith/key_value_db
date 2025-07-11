/** !
 * Header file for key value database
 * 
 * @file key_value.c
 * 
 * @author Jacob Smith
 */

// header
#include "key_value.h"

// function declarations
// allocators
/** !
 * Allocate a key value database
 * 
 * @param pp_key_value the key value database
 * 
 * @return 1 on success, 0 on error
 */
int key_value_db_create ( key_value_db **pp_key_value_db )
{

    // Argument errors
    if ( pp_key_value_db == (void *) 0 ) goto no_key_value_db;

    // Initialized data
    key_value_db *p_key_value_db = realloc(0, sizeof(key_value_db));

    // Error handling
    if ( p_key_value_db == (void *) 0 ) goto no_mem;

    // Initialize memory
    memset(p_key_value_db, 0, sizeof(key_value_db));

    // Return a pointer to the caller
    *pp_key_value_db = p_key_value_db;

    // Success
    return 1;

    // Error handling
    {

        // Argument errors
        {
            no_key_value_db:
                #ifndef NDEBUG
                    log_error("[data] [key value] Null pointer provided for parameter \"pp_key_value_db\" in call to function \"%s\"\n", __FUNCTION__);
                #endif

                // Error
                return 0;
        }
        
        // Standard library
        {

            no_mem:
                #ifndef NDEBUG
                    log_error("[Standard library] Failed to allocate memory in call to function \"%s\"\n");
                #endif

                // Error
                return 0;
        }
    }
}

// parsers
/** !
 * Parse a key value database node
 * 
 * @param p_file             the file
 * @param p_binary_tree_node pointer to the binary tree node
 * 
 * @return 1 on success, 0 on error
 */
int key_value_db_property_parse ( FILE *p_file, binary_tree_node *p_binary_tree_node );

// key accessor
const void *key_value_db_property_key_accessor (  const void *p_value );

int key_value_db_construct ( key_value_db **pp_key_value_db, const char *p_database_file, unsigned short port )
{

    // argument errors
    if ( pp_key_value_db == (void *) 0 ) goto no_key_value_db;

    // initialized data
    key_value_db *p_key_value_db = (void *) 0;
    FILE *p_file = (void *) 0;

    // allocate a key value database
    if ( key_value_db_create(&p_key_value_db) == 0 ) goto failed_to_allocate_key_value_db;
    
    // store the database file path
    strncpy(p_key_value_db->_database_file, p_database_file, sizeof(p_key_value_db->_database_file) - 1);

    // open the file 
    p_file = fopen(p_key_value_db->_database_file, "r");

    // if the file exists ...
    if ( p_file )
    {

        // ... close the file ...
        fclose(p_file);

        // ... and parse the binary tree ...
        binary_tree_parse(
            &p_key_value_db->p_binary_tree,
            p_key_value_db->_database_file,
            (fn_tree_equal *)strcmp,
            key_value_db_property_key_accessor,
            key_value_db_property_parse
        );
    }

    // ... if the file doesn't exist ...
    else
    {

        // ... construct a binary tree 
        binary_tree_construct(
            &p_key_value_db->p_binary_tree, 
            (fn_tree_equal *)strcmp, 
            key_value_db_property_key_accessor,
            KEY_VALUE_DB_NODE_SIZE_BYTES
        );
    }

    // construct a socket
    if ( 0 == socket_tcp_create(&p_key_value_db->_socket, socket_address_family_ipv4, port) ) goto failed_to_construct_socket;

    // construct a thread pool
    if ( 0 == thread_pool_construct(&p_key_value_db->p_thread_pool, 4) ) goto failed_to_construct_socket;

    // Return a pointer to the caller
    *pp_key_value_db = p_key_value_db;

    // Success
    return 1;

    // error handling
    {

        // argument errors
        {
            no_key_value_db:
                #ifndef NDEBUG
                    log_error("[data] [key value] Null pointer provided for parameter \"pp_key_value_db\" in call to function \"%s\"\n", __FUNCTION__);
                #endif

                // Error
                return 0;
        }

        // core errors
        {
            failed_to_construct_socket:
                #ifndef NDEBUG
                    log_error("[data] [key value] Failed to allocate memory for key value database in call to function \"%s\"\n", __FUNCTION__);
                #endif

                // Error
                return 0;

        }

        // data errors
        {
            failed_to_allocate_key_value_db:
                #ifndef NDEBUG
                    log_error("[data] [key value] Failed to allocate memory for key value database in call to function \"%s\"\n", __FUNCTION__);
                #endif

                // Error
                return 0;

            failed_to_allocate_dictionary:
                #ifndef NDEBUG
                    log_error("[data] [key value] Failed to allocate memory for dictionary in call to function \"%s\"\n", __FUNCTION__);
                #endif

                // Error
                return 0;
        }
    }
}

int key_value_db_put ( key_value_db *p_key_value_db, char *p_output, char *p_key, json_value *p_value )
{

    // TODO: Argument check
    //

    // Initialized data
    size_t l = strlen(p_key);
    key_value_db_property *p_property = 0;

    // Error check
    if ( l > KEY_VALUE_DB_PROPERTY_KEY_LENGTH_MAX ) return 0;

    // Search for the key
    binary_tree_search(p_key_value_db->p_binary_tree, p_key, (void **) &p_property);

    // Key not found
    if ( NULL == p_property )
    {

        // Allocate memory for a property
        p_property = realloc(0, sizeof(key_value_db_property));

        // Error check
        if ( p_property == (void *) 0 ) goto no_mem;
        
        // Initialize property
        memset(p_property, 0, sizeof(key_value_db_property));

        // Copy the key
        memcpy(p_property->_key, p_key, l);

        // Store the value
        p_property->p_value = p_value;

        // Serialize the value to plaintext
        json_value_serialize(p_value, p_property->_value);

        // Insert the property into the tree
        binary_tree_insert(
            p_key_value_db->p_binary_tree,
            p_property
        );

    }
    else
    {

        // Initialize property
        memset(p_property, 0, sizeof(key_value_db_property));

        // Copy the key
        memcpy(p_property->_key, p_key, l);

        // Store the value
        p_property->p_value = p_value;

        // Serialize the value to plaintext
        json_value_serialize(p_value, p_property->_value);
    }

    // Store the result
    memcpy(p_output, p_property->_value, sizeof(p_property->_value));

    // compute the length of the json text
    l = strlen(p_output);

    // store a line feed and a null terminator
    p_output[l + 0] = '\n',
    p_output[l + 1] = '\0';

    // Success
    return 1;

    // TODO: Error handling
    {

        // Argument errors
        {
            no_mem:
                #ifndef NDEBUG
                    // TODO
                #endif

                // Error
                return 0;
        }
    }
}

int key_value_db_get ( key_value_db *p_key_value_db, char *p_output, char *p_key )
{

    // TODO: Argument check
    //

    // Initialized data
    key_value_db_property *p_property = (void *) 0;
    size_t len = 0;

    // Search the binary tree
    if ( 0 == binary_tree_search(p_key_value_db->p_binary_tree, p_key, (void **)&p_property) ) return 0;

    // Print the value to the file
    json_value_serialize(p_property->p_value, p_output);

    // compute the length of the json text
    len = strlen(p_output);

    // store a line feed and a null terminator
    p_output[len + 0] = '\n',
    p_output[len + 1] = '\0';

    // Success
    return 1;

    // TODO: Error handling
    { }
}

int key_value_db_property_print ( void *p_value )
{

    // Initialized data
    key_value_db_property *p_property = p_value;

    printf("%-32s : ", p_property->_key);

    json_value_print(p_property->p_value);

    putchar('\n');

    // Success
    return 1;
}

int key_value_db_property_serialize ( FILE *p_f, binary_tree_node *p_binary_tree_node )
{
    key_value_db_property *p_property = p_binary_tree_node->p_value;

    fwrite(p_property->_key, 1, 32, p_f);

    json_value_serialize(p_property->p_value, p_property->_value);

    fwrite(p_property->_value, 1, 975, p_f),
    fputc('\0', p_f);

    // Success
    return 1;
}

int key_value_db_property_parse ( FILE *p_file, binary_tree_node *p_binary_tree_node )
{

    // TODO: Argument check
    //

    // Initialized data
    key_value_db_property *p_property = realloc(0, sizeof(key_value_db_property));

    // Initialize data
    memset(p_property, 0, sizeof(key_value_db_property));

    // Error check
    //

    // Read a key from the input
    fread(&p_property->_key, KEY_VALUE_DB_PROPERTY_KEY_LENGTH_MAX, 1, p_file);

    // Read a value from the input
    fread(&p_property->_value, KEY_VALUE_DB_PROPERTY_VALUE_LENGTH_MAX, 1, p_file);

    // Parse the json text into a json value
    if ( json_value_parse(p_property->_value, 0, &p_property->p_value) == 0 ) goto failed_to_parse_json_value;

    p_binary_tree_node->p_value = p_property;

    // Success
    return 1;

    // Error handling
    {

        // Argument errors
        {
            
        }
        
        // json errors
        {
            failed_to_parse_json_value:

                // Error
                return 0;
        }
    }

}

const void *key_value_db_property_key_accessor ( const void *p_value ) 
{

    // Initialized data
    key_value_db_property *p_property = (key_value_db_property *) p_value;

    // Success
    return p_property->_key;
}

int key_value_db_write ( key_value_db *p_key_value_db, const char *p_path )
{

    // Serialize the database
    binary_tree_serialize(
        p_key_value_db->p_binary_tree, 
        p_key_value_db->_database_file, 
        key_value_db_property_serialize
    );

    // Success
    return 1;
}

int key_value_db_parse_statement ( key_value_db *p_key_value_db, char *p_input, char *p_output )
{

    // Argument errors
    if ( p_key_value_db == (void *) 0 ) goto no_key_value_db;

    // Initialized data
    int ret = 0;

    // Fast exit
    if ( strlen(p_input) == 0 ) return 1;

    // Set
    if ( strncmp(p_input, "set", 3) == 0 ) goto set_instruction;

    // Get
    else if ( strncmp(p_input, "get", 3) == 0 ) goto get_instruction;

    // List instruction
    else if ( strncmp(p_input, "list", 4) == 0 ) goto list_instruction;

    // Write
    else if ( strncmp(p_input, "write", 5) == 0 ) goto write_instruction;
    
    // Dones
    done:

    // Done
    return ret;

    // Set instruction
    set_instruction:
    {
       
        // Initialized data
        char *p_put_key = 0;
        char *p_put_value = 0;
        json_value *p_value = 0;

        strtok(p_input, " \n");

        p_put_key = strtok(0, " \n");
        p_put_value = strtok(0, "\n");

        // Parse the value
        if ( json_value_parse(p_put_value, 0, &p_value ) == 0 ) goto failed_to_parse_json_value;

        // Update the value
        ret = key_value_db_put(p_key_value_db, p_output, p_put_key, p_value);

        // Done
        goto done;
    }

    // Get instruction
    get_instruction:
    {
       
        // Initialized data
        char *p_get_key = 0;
        json_value *p_value = 0;

        strtok(p_input, " \n");

        p_get_key = strtok(0, " \n");
        p_get_key[strlen(p_get_key)]='\0';

        if ( p_get_key )
            key_value_db_get(p_key_value_db, p_output, p_get_key);

        // Success
        return 1;
    }

    // List instruction
    list_instruction:
    {

        // list the contents of the database
        binary_tree_traverse_inorder(
            p_key_value_db->p_binary_tree, 
            key_value_db_property_print
        );

        // success
        return 1;
    }

    // Write instruction
    write_instruction:
    {

        // Serialize the database
        key_value_db_write(
            p_key_value_db,
            p_key_value_db->_database_file
        );

        // Success
        return 1;
    }

    // Error handling
    {

        // Argument errors
        {
            no_key_value_db:
                #ifndef NDEBUG
                    log_error("[data] [key value] Null pointer provided for parameter \"p_key_value_db\" in call to function \"%s\"\n", __FUNCTION__);
                #endif

                // Error
                return 0;
        }

        // json errors
        {
            failed_to_parse_json_value:
                #ifndef NDEBUG
                    log_error("[data] [key value] Failed to parse json value in call to function \"%s\"\n", __FUNCTION__);
                #endif

                // Error
                return 0;
        }
    }
}
