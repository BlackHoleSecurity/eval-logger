#include "php.h"                // Core PHP API
#include "zend_compile.h"      // Zend compilation engine API
#include "php_streams.h"       // PHP stream wrapper for file operations

// Store the original zend_compile_string function pointer for restoration
static zend_op_array* (*original_compile_string)(zend_string *, const char *, zend_compile_position) = NULL;

/**
 * Log the string passed to eval() into a file.
 * This function opens a stream to "eval_log.txt", writes the string, and closes the stream.
 */
static void log_eval_string(zend_string *source_string)
{
    // Open eval_log.txt for writing in binary mode. Overwrites on each call.
    php_stream *stream = php_stream_open_wrapper("/tmp/eval_log", "wb", REPORT_ERRORS, NULL);
    if (stream) {
        // Write the raw eval string contents to the file
        php_stream_write(stream, ZSTR_VAL(source_string), ZSTR_LEN(source_string));
        php_stream_write(stream, "\n", 1);  // Append newline
        php_stream_close(stream);          // Close the stream
    }
}

/**
 * Our wrapper around zend_compile_string.
 * Called whenever eval() is used. Logs the eval content before passing it to the original compiler.
 */
static zend_op_array* eval_logger_compile_string(zend_string *source_string, const char *filename, zend_compile_position compile_pos)
{
    log_eval_string(source_string);  // Log the eval content
    return original_compile_string(source_string, filename, compile_pos);  // Call original compiler
}

/**
 * MINIT function: Called when the extension is loaded.
 * Swaps out zend_compile_string for our custom logging version.
 */
PHP_MINIT_FUNCTION(eval_logger)
{
    original_compile_string = zend_compile_string;              // Backup original function
    zend_compile_string = eval_logger_compile_string;           // Replace with our custom function
    return SUCCESS;
}

/**
 * MSHUTDOWN function: Called when the extension is unloaded.
 * Restores the original zend_compile_string function pointer.
 */
PHP_MSHUTDOWN_FUNCTION(eval_logger)
{
    zend_compile_string = original_compile_string;              // Restore original function
    return SUCCESS;
}

// Module entry definition for the PHP engine
zend_module_entry eval_logger_module_entry = {
    STANDARD_MODULE_HEADER,
    "eval_logger",               // Extension name
    NULL,                        // Functions (none exposed)
    PHP_MINIT(eval_logger),      // Startup hook
    PHP_MSHUTDOWN(eval_logger),  // Shutdown hook
    NULL,                        // RINIT
    NULL,                        // RSHUTDOWN
    NULL,                        // MINFO
    "0.1",                       // Extension version
    STANDARD_MODULE_PROPERTIES
};

// Required macro for PHP to recognize the extension
ZEND_GET_MODULE(eval_logger)

