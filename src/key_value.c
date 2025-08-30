/** !
 * Database
 * 
 * @file src/db.c
 * 
 * @author Jacob Smith
 */

// header
#include <key_value/key_value.h>

// structure definitions
struct key_value_db_s
{
    bool running;

    cache       *p_cache;
    binary_tree *p_binary_tree;

    struct 
    {
        thread_pool     *p_thread_pool;
        socket_tcp       _socket;
        parallel_thread *p_listener_thread;
    } network;
};

struct key_value_property_s
{
    char _name[63+1];
    char _value[255+1];
    json_value *p_value;
};

int key_value_db_process
( 
    key_value_db *p_key_value_db, 
    char *p_request, size_t request_len,
    char *p_response, size_t *p_response_len
);

void *key_value_property_key_accessor ( key_value_property *p_property )
{

    // argument check
    if ( NULL == p_property ) return NULL;

    // done
    return p_property->_name;
}

int key_value_property_comparator ( key_value_property *p_a, key_value_property *p_b )
{

    // argument check
    if ( NULL == p_a ) return 1;
    if ( NULL == p_b ) return -1;

    // done
    return strcmp(p_a->_name, p_b->_name);
}

int key_value_property_equality ( key_value_property *p_a, key_value_property *p_b )
{

    // done
    return ( 0 == key_value_property_comparator(p_a, p_b) );
}

int key_value_db_server_accept ( socket_tcp _socket_tcp, socket_ip_address ip_address, socket_port port_number, key_value_db *p_key_value_db )
{

    // initialized data
    size_t      len      = 0;
    char       *p_buffer = 0;
    json_value *p_value  = 0;
    char _response_buf[4096] = {0}; 
    size_t response_len = 0;

    // log the connection
    log_info("[key value db] Accepted incoming connection from %hhu.%hhu.%hhu.%hhu:%hu\n", 
            (ip_address >> 24) & 0xFF, 
            (ip_address >> 16) & 0xFF, 
            (ip_address >>  8) & 0xFF, 
            (ip_address >>  0) & 0xFF, 
            
            port_number
    );

    while (1)
    {

        // parse the request
        {

            // get the length of the request
            if ( 0 == socket_tcp_receive(_socket_tcp, &len, sizeof(size_t)) ) goto disconnected;

            // error check
            if ( 4096 < len ) goto too_long;

            // allocate a buffer for the rest of the message
            p_buffer = default_allocator(0, len);
            if ( NULL == p_buffer ) goto no_mem;

            memset(p_buffer, 0, len);

            // receive the rest of the message
            if ( 0 == socket_tcp_receive(_socket_tcp, p_buffer, len) ) goto disconnected;
        }

        // process the request
        {
            
            // exit?
            if ( 0 == strcmp(p_buffer, "exit") ) goto clean_disconnect;

            // process
            key_value_db_process(
                p_key_value_db, 
                p_buffer, len, 
                _response_buf + sizeof(size_t), &response_len
            );
        }

        // serialize the response
        {

            // set the length
            *(size_t *)_response_buf = response_len;
            
            // send the result
            socket_tcp_send(_socket_tcp, _response_buf, sizeof(size_t) + response_len);
        }

        memset(_response_buf, 0, sizeof(_response_buf));
        p_buffer = default_allocator(p_buffer, 0);
    }

    clean_disconnect:

    // close the socket
    {
        // initialized data
        char _buf[4096] = {0}; 

        // set the length
        *(size_t *)_buf = 4;
        strncpy(_buf + sizeof(size_t), "exit", 4);
        len = 4;

        // send the result
        socket_tcp_send(_socket_tcp, _buf, sizeof(size_t) + len);

        // close the socket
        socket_tcp_destroy(&_socket_tcp);
    }

    disconnected:
    // log the disconnect
    log_info("[key value db] Connection closed from %hhu.%hhu.%hhu.%hhu:%hu\n", 
            (ip_address >> 24) & 0xFF, 
            (ip_address >> 16) & 0xFF, 
            (ip_address >>  8) & 0xFF, 
            (ip_address >>  0) & 0xFF, 
            
            port_number
    );

    // success
    return 1;

    // error handling
    {

        // socket errors
        {
            too_long:
            no_mem:
            parse_error:
                return 0;
        }
    }
}

int key_value_db_listener ( key_value_db *p_key_value_db )
{

    // log a message
    log_info("[key value db] Listening for incoming connections...\n");

    // listen for incoming connections
    while ( p_key_value_db->running )
        socket_tcp_listen(p_key_value_db->network._socket, (fn_socket_tcp_accept *)key_value_db_server_accept, p_key_value_db);

    // success
    return 1;
}

int key_value_db_construct ( key_value_db **pp_key_value_db )
{

    // argument check
    if ( NULL == pp_key_value_db ) goto no_key_value_db;

    // initialized data
    key_value_db *p_key_value_db = default_allocator(0, sizeof(key_value_db));

    // error check
    if ( NULL == p_key_value_db ) goto no_mem;

    // construct networking stuff
    {
        
        // construct a thread pool
        thread_pool_construct(&p_key_value_db->network.p_thread_pool, 4);

        // construct a socket
        socket_tcp_create(&p_key_value_db->network._socket, socket_address_family_ipv4, 6708);

        // set the running flag
        p_key_value_db->running = true;

        // construct a listener thread
        parallel_thread_start(&p_key_value_db->network.p_listener_thread, (fn_parallel_task *)key_value_db_listener, p_key_value_db);
    }

    // construct database stuff
    {

        // construct an LRU cache
        cache_construct
        (
            &p_key_value_db->p_cache, 
            1024,
            (fn_equality *) key_value_property_equality,
            (fn_key_accessor *) key_value_property_key_accessor
        );

        // construct a binary tree
        binary_tree_construct
        (
            &p_key_value_db->p_binary_tree, 
            (fn_comparator *) key_value_property_comparator, 
            (fn_key_accessor *) key_value_property_key_accessor,
            512
        );

        {
            key_value_property *p_property = default_allocator(0, sizeof(key_value_property));
            if ( NULL == p_property ) goto no_mem;

            strcpy(p_property->_name, "fit");
            p_property->p_value = NULL;

            binary_tree_insert(p_key_value_db->p_binary_tree, p_property);
        }
    }

    // return a pointer to the caller
    *pp_key_value_db = p_key_value_db;

    // success
    return 1;

    // error handling
    {

        // argument errors
        {
            no_key_value_db:
                #ifndef NDEBUG
                    log_error("[key value db] Null pointer provided for parameter \"pp_key_value_db\" in call to function \"%s\"", __FUNCTION__);
                #endif

                // error
                return 0;
        }

        // key value db errors
        {
            failed_to_construct_key_sets:
                #ifndef NDEBUG
                    log_error("[key value db] Failed to construct key sets in call to function \"%s\"", __FUNCTION__);
                #endif

                // error
                return 0;
        }

        // binary tree errors
        {
            failed_to_construct_binary_tree:
                #ifndef NDEBUG
                    log_error("[key value db] Failed to construct binary tree in call to function \"%s\"", __FUNCTION__);
                #endif

                // error
                return 0;
        }

        // standard library errors
        {
            no_mem:
                #ifndef NDEBUG
                    log_error("[standard library] Failed to allocate memory in call to function \"%s\"", __FUNCTION__);
                #endif

                // error
                return 0;
        }
    }
}

int key_value_db_process_get
( 
    key_value_db *p_key_value_db, 
    char         *p_key, 
    
    char *p_response, size_t *p_response_len
)
{

    // argument check
    if ( NULL == p_key_value_db ) goto no_key_value_db;
    if ( NULL ==          p_key ) goto no_key;
    if ( NULL ==     p_response ) goto no_response;
    if ( NULL == p_response_len ) goto no_response_len;

    // initialized data
    key_value_property *p_value = NULL;

    // logs
    log_info("[key value db] get(\"%s\")\n", p_key);

    // search the cache
    if ( 0 == cache_find(p_key_value_db->p_cache, p_key, (void **)&p_value) ) goto not_in_cache;
    
    // logs
    log_info("[key value db] Found key \"%s\" in cache \n", p_key);

    found:

    // serialize the response
    memcpy(p_response, "{\"okay\":\"true\",\"value\":", 23);
    *p_response_len = 23 + json_value_serialize(p_value->p_value, p_response + 23);
    memcpy(p_response + *p_response_len, "}", 1);
    (*p_response_len)++;

    // success
    return 1;

    // this branch handles cache misses
    not_in_cache:
    {

        // search the binary tree
        if ( 0 == binary_tree_search(p_key_value_db->p_binary_tree, p_key, (void **)&p_value) ) goto not_a_key;
        
        // logs
        log_info("[key value db] Found key \"%s\" in tree\n", p_key);
        
        // logs
        log_info("[key value db] Adding key \"%s\" to cache\n", p_key);
    
        // add the value to the cache
        cache_insert(p_key_value_db->p_cache, p_value->_name, p_value);

        // done
        goto found;
    }

    // error handling
    {

        // argument errors
        {
            no_key_value_db:
                #ifndef NDEBUG
                    log_error("[key value db] Null pointer provided for parameter \"p_key_value_db\" in call to function \"%s\"", __FUNCTION__);
                #endif

                // error
                return 0;

            no_key:
                #ifndef NDEBUG
                    log_error("[key value db] Null pointer provided for parameter \"p_key\" in call to function \"%s\"", __FUNCTION__);
                #endif

                // error
                return 0;

            no_response:
               #ifndef NDEBUG
                    log_error("[key value db] Null pointer provided for parameter \"p_response\" in call to function \"%s\"", __FUNCTION__);
                #endif

                // error
                return 0;

            no_response_len:
               #ifndef NDEBUG
                    log_error("[key value db] Null pointer provided for parameter \"p_response_len\" in call to function \"%s\"", __FUNCTION__);
                #endif

                // error
                return 0;
        }

        // key value db errors
        {
            not_a_key:
                #ifndef NDEBUG
                    log_error("[key value db] Key \"%s\" not found in call to function \"%s\"\n", p_key, __FUNCTION__);
                #endif

                // copy the error message to the response buffer
                memcpy(p_response, "{\"okay\":\"false\"}", 16);
                *p_response_len = 16;

                // error
                return 0;
        }
    }
}

int key_value_db_process_set
(
    key_value_db *p_key_value_db,
    char         *p_key,
    json_value   *p_value,

    char *p_response, size_t *p_response_len
)
{

    // argument check
    if ( NULL == p_key_value_db ) goto no_key_value_db;
    if ( NULL ==          p_key ) goto no_key;
    if ( NULL ==        p_value ) goto no_value;
    if ( NULL ==     p_response ) goto no_response;
    if ( NULL == p_response_len ) goto no_response_len;

    // initialized data
    key_value_property *p_property = NULL;

    if ( binary_tree_search(p_key_value_db->p_binary_tree, p_key, (void **)&p_property) ) 
        binary_tree_remove(p_key_value_db->p_binary_tree, p_property, NULL);

    p_property = default_allocator(0, sizeof(key_value_property));
    if (NULL == p_property) goto no_mem;
    
    // copy the key
    strncpy(p_property->_name, p_key, sizeof(p_property->_name) - 1);
    p_property->_name[sizeof(p_property->_name) - 1] = '\0';

    // serialize the value
    json_value_serialize(p_value, p_property->_value);

    // parse the value
    json_value_parse(p_property->_value, NULL, &p_property->p_value);

    // insert the value 
    binary_tree_insert(p_key_value_db->p_binary_tree, p_property);

    // remove the property from the cache
    cache_remove(p_key_value_db->p_cache, p_property->_name, NULL);

    // serialize the response
    memcpy(p_response, "{\"okay\":\"true\",\"value\":", 23);
    *p_response_len = 23 + json_value_serialize(p_property->p_value, p_response + 23);
    memcpy(p_response + *p_response_len, "}", 1);
    (*p_response_len)++;

    // success
    return 1;

    // error handling
    {

        // argument errors
        {
            no_key_value_db:
                #ifndef NDEBUG
                    log_error("[key value db] Null pointer provided for parameter \"p_key_value_db\" in call to function \"%s\"", __FUNCTION__);
                #endif

                // error
                return 0;

            no_key:
                #ifndef NDEBUG
                    log_error("[key value db] Null pointer provided for parameter \"p_key\" in call to function \"%s\"", __FUNCTION__);
                #endif

                // error
                return 0;

            no_value:
               #ifndef NDEBUG
                    log_error("[key value db] Null pointer provided for parameter \"p_value\" in call to function \"%s\"", __FUNCTION__);
                #endif

                // error
                return 0;

            no_response:
               #ifndef NDEBUG
                    log_error("[key value db] Null pointer provided for parameter \"p_response\" in call to function \"%s\"", __FUNCTION__);
                #endif

                // error
                return 0;

            no_response_len:
               #ifndef NDEBUG
                    log_error("[key value db] Null pointer provided for parameter \"p_response_len\" in call to function \"%s\"", __FUNCTION__);
                #endif

                // error
                return 0;
        }

        // key value db errors
        {
            not_a_key:
                #ifndef NDEBUG
                    log_error("[key value db] Key \"%s\" not found in call to function \"%s\"\n", p_key, __FUNCTION__);
                #endif

                // copy the error message to the response buffer
                memcpy(p_response, "{\"okay\":\"false\"}", 16);
                *p_response_len = 16;

                // error
                return 0;
        }

        // standard library errors
        {
            no_mem:
                #ifndef NDEBUG
                    log_error("[standard library] Failed to allocate memory in call to function \"%s\"", __FUNCTION__);
                #endif

                // error
                return 0;
        }
    }
}


int key_value_db_process
( 
    key_value_db *p_key_value_db, 
    char *p_request, size_t request_len,
    char *p_response, size_t *p_response_len
)
{

    // argument check
    if ( NULL == p_key_value_db ) goto no_key_value_db;
    if ( NULL ==      p_request ) goto no_request;
    if ( NULL ==     p_response ) goto no_response;
    if ( NULL == p_response_len ) goto no_response_len;

    // initialized data
    size_t  cur = 0;
    char   *command  = NULL,
           *op1      = NULL,
           *op2      = NULL,
           *op3      = NULL;

    size_t command_start = 0, command_end   = 0,
           op1_start     = 0, op1_end       = 0,
           op2_start     = 0, op2_end       = 0,
           op3_start     = 0, op3_end       = 0;

    // parse the command
    {

        // skip leading blanks
        while 
        ( 
            isblank(p_request[cur]) && 
            cur < request_len
        ) cur++;
        
        // bounds check
        if ( cur >= request_len ) goto bad_request;

        // store the command start offset
        command_start = cur;

        // skip the command itself 
        while 
        ( 
            !isblank(p_request[cur]) && 
            p_request[cur] != '\0' &&
            cur < request_len
        ) cur++;

        // bounds check
        if ( cur > request_len ) goto bad_request;

        // store the command end offset
        command_end = cur;

        // store the command
        command = &p_request[command_start],
        p_request[command_end] = '\0',
        cur++;

        // print the command
        // log_info("[key value db] Command: \"%s\"\n", command);
    }

    // process get
    if ( 0 == strcmp(command, "get") )
    {

        // error check
        if ( p_request[cur] == '\0' ) goto failed_to_parse_get_key;

        // parse the key
        {

            // skip leading blanks
            while 
            ( 
                isblank(p_request[cur]) && 
                cur < request_len
            ) cur++;

            // bounds check
            if ( cur >= request_len ) goto bad_request;

            // store operand start offset
            op1_start = cur;

            // skip the operand itself 
            while 
            ( 
                !isblank(p_request[cur]) && 
                p_request[cur] != '\0' &&
                cur < request_len
            ) cur++;

            // bounds check
            if ( cur > request_len ) goto bad_request;

            // store the operand end offset
            op1_end = cur;

            // store the operand
            op1 = &p_request[op1_start];
            p_request[op1_end] = '\0';
            cur++;
            
            // print the operand
            // log_info("[key value db] Operand 1: \"%s\"\n", op1);
        }

        // error check
        if ( NULL == op1 ) goto failed_to_parse_get_key;

        // process the get command
        key_value_db_process_get(p_key_value_db, op1, p_response, p_response_len);
    }
    
    // process set
    else if ( 0 == strcmp(command, "set") )
    {

        // initialized data
        json_value *p_value = NULL;

        // error check
        if ( p_request[cur] == '\0' ) goto failed_to_parse_set_key;

        // parse the key
        {

            // skip leading blanks
            while 
            ( 
                isblank(p_request[cur]) && 
                cur < request_len
            ) cur++;

            // bounds check
            if ( cur >= request_len ) goto bad_request;

            // store operand start offset
            op1_start = cur;

            // skip the operand itself 
            while 
            ( 
                !isblank(p_request[cur]) && 
                p_request[cur] != '\0' &&
                cur < request_len
            ) cur++;

            // bounds check
            if ( cur > request_len ) goto bad_request;

            // store the operand end offset
            op1_end = cur;

            // store the operand
            op1 = &p_request[op1_start];
            p_request[op1_end] = '\0';
            cur++;
            
            // print the operand
            // log_info("[key value db] Operand 1: \"%s\"\n", op1);
        }

        // parse the value
        {

            // skip leading blanks
            while
            (
                isblank(p_request[cur]) &&
                cur < request_len
            ) cur++;

            // bounds check
            if (cur >= request_len) goto failed_to_parse_set_value;

            // store operand start offset
            op2_start = cur;

            // skip the operand itself
            while
            (
                !isblank(p_request[cur]) &&
                p_request[cur] != '\0' &&
                cur < request_len
            ) cur++;

            // bounds check
            if (cur > request_len) goto failed_to_parse_set_value;

            // store the operand end offset
            op2_end = cur;

            // store the operand
            op2 = &p_request[op2_start];
            p_request[op2_end] = '\0';
            cur++;

            json_value_parse(op2, NULL, &p_value);
        }

        // error check
        if ( NULL == op1 ) goto failed_to_parse_set_key;

        // process the get command
        key_value_db_process_set(p_key_value_db, op1, p_value, p_response, p_response_len);
    }
    

    // success
    return 1;

    // error handling
    {

        // argument errors
        {
            no_key_value_db:
                #ifndef NDEBUG
                    log_error("[key value db] Null pointer provided for parameter \"p_key_value_db\" in call to function \"%s\"", __FUNCTION__);
                #endif

                // error
                return 0;

            no_request:
                #ifndef NDEBUG
                    log_error("[key value db] Null pointer provided for parameter \"p_request\" in call to function \"%s\"", __FUNCTION__);
                #endif

                // error
                return 0;

            no_response:
                #ifndef NDEBUG
                    log_error("[key value db] Null pointer provided for parameter \"p_response\" in call to function \"%s\"", __FUNCTION__);
                #endif

                // error
                return 0;

            no_response_len:
                #ifndef NDEBUG
                    log_error("[key value db] Null pointer provided for parameter \"p_response_len\" in call to function \"%s\"", __FUNCTION__);
                #endif

                // error
                return 0;
        }

        // key value db errors
        {
            failed_to_parse_get_key:
                #ifndef NDEBUG
                    log_error("[key value db] Failed to parse get request key in call to function \"%s\"\n", __FUNCTION__);
                #endif

                // error
                return 0;

            failed_to_parse_set_key:
                #ifndef NDEBUG
                    log_error("[key value db] Failed to parse set request key in call to function \"%s\"\n", __FUNCTION__);
                #endif

                // error
                return 0;

            failed_to_parse_set_value:
                #ifndef NDEBUG
                    log_error("[key value db] Failed to parse set request value in call to function \"%s\"\n", __FUNCTION__);
                #endif

                // error
                return 0;

            bad_request:
                #ifndef NDEBUG
                    log_error("[key value db] Bad request in call to function \"%s\"\n", __FUNCTION__);
                #endif

                // error
                return 0;
        }
    }
}

int key_value_db_print ( key_value_db *p_key_value_db )
{

    // argument check
    if ( NULL == p_key_value_db ) goto no_key_value_db;

    // initialized data
    //

    // success
    return 1;

    // error handling
    {

        // argument errors
        {
            no_key_value_db:
                #ifndef NDEBUG
                    log_error("[key value db] Null pointer provided for parameter \"p_key_value_db\" in call to function \"%s\"", __FUNCTION__);
                #endif

                // error
                return 0;
        }
    }
}
