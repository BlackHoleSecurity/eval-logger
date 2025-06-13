#include "php.h"
#include "zend_compile.h"
#include "php_streams.h"

static zend_op_array* (*original_compile_string)(zend_string *, const char *, zend_compile_position) = NULL;

static void log_eval_string(zend_string *source_string) {
    php_stream *stream = php_stream_open_wrapper("eval_log.txt", "wb", REPORT_ERRORS, NULL);
    if (stream) {
        php_stream_write(stream, ZSTR_VAL(source_string), ZSTR_LEN(source_string));
        php_stream_write(stream, "\n", 1);
        php_stream_close(stream);
    }
}

static zend_op_array* eval_logger_compile_string(zend_string *source_string, const char *filename, zend_compile_position compile_pos) {
    log_eval_string(source_string);
    return original_compile_string(source_string, filename, compile_pos);
}

PHP_MINIT_FUNCTION(eval_logger) {
    original_compile_string = zend_compile_string;
    zend_compile_string = eval_logger_compile_string;
    return SUCCESS;
}

PHP_MSHUTDOWN_FUNCTION(eval_logger) {
    zend_compile_string = original_compile_string;
    return SUCCESS;
}

zend_module_entry eval_logger_module_entry = {
    STANDARD_MODULE_HEADER,
    "eval_logger",
    NULL,
    PHP_MINIT(eval_logger),
    PHP_MSHUTDOWN(eval_logger),
    NULL,
    NULL,
    NULL,
    "0.1",
    STANDARD_MODULE_PROPERTIES
};

ZEND_GET_MODULE(eval_logger)

