#include "php.h"            // Core PHP API
#include "zend_compile.h"   // Zend compilation engine API
#include "php_streams.h"    // PHP stream wrapper for file operations
#include "zend_smart_str.h" // Smart string handling
#include "SAPI.h"           // Server API for PHP
#include "php_ini.h"        // INI handling
#include "ext/standard/info.h" // For PHP_MINFO
#include <stdio.h>          // Standard I/O for user prompts
#include <string.h>         // String manipulation functions

extern sapi_module_struct sapi_module;


static zend_op_array *eval_logger_compile_string(zend_string *source_string, const char *filename, zend_compile_position compile_pos);
static PHP_INI_MH(OnUpdateEvalLoggerEnable);

ZEND_BEGIN_MODULE_GLOBALS(eval_logger)
    zend_bool enable;
    zend_bool interactive;
    zend_long mode;
    char *logfile;
ZEND_END_MODULE_GLOBALS(eval_logger)

ZEND_DECLARE_MODULE_GLOBALS(eval_logger)

/* Access macro for globals (works in ZTS and non-ZTS) */
#ifndef EVAL_LOGGER_G
# ifdef ZTS
/* eval_logger_globals_id is provided by ZEND_DECLARE_MODULE_GLOBALS in ZTS builds */
#  define EVAL_LOGGER_G(v) TSRMG(eval_logger_globals_id, zend_eval_logger_globals *, v)
# else
#  define EVAL_LOGGER_G(v) (eval_logger_globals.v)
# endif
#endif

// Store the original zend_compile_string function pointer for restoration
static zend_op_array *(*original_compile_string)(zend_string *, const char *, zend_compile_position) = NULL;

/* INI entries
 * - eval_logger.enable (boolean) : enable/disable hook
 * - eval_logger.mode   (int)     : 0 = log only (default), 1 = block execution
 * - eval_logger.interactive (bool): only prompt on CLI when enabled (default off)
 * - eval_logger.logfile (string)  : optional path (empty = use php_log_err)
 */
PHP_INI_BEGIN()
    STD_PHP_INI_BOOLEAN("eval_logger.enable", "1", PHP_INI_ALL, OnUpdateEvalLoggerEnable, enable, zend_eval_logger_globals, eval_logger_globals)
    STD_PHP_INI_ENTRY("eval_logger.mode", "0", PHP_INI_ALL, OnUpdateLong, mode, zend_eval_logger_globals, eval_logger_globals)
    STD_PHP_INI_BOOLEAN("eval_logger.interactive", "1", PHP_INI_ALL, OnUpdateBool, interactive, zend_eval_logger_globals, eval_logger_globals)
    STD_PHP_INI_ENTRY("eval_logger.logfile", "", PHP_INI_ALL, OnUpdateString, logfile, zend_eval_logger_globals, eval_logger_globals)
PHP_INI_END()

/* Initialize defaults for globals */
static void php_eval_logger_init_globals(zend_eval_logger_globals *eg)
{
    eg->enable = 1;
    eg->interactive = 0;
    eg->mode = 0;
    eg->logfile = NULL;
}


/* INI handler to enable/disable runtime hooking */
static PHP_INI_MH(OnUpdateEvalLoggerEnable)
{
    int enabled = zend_ini_parse_bool(new_value);
    EVAL_LOGGER_G(enable) = enabled ? 1 : 0;

    if (enabled) {
        /* install hook if not already installed */
        if (!original_compile_string) {
            original_compile_string = zend_compile_string;
            zend_compile_string = eval_logger_compile_string;
        }
    } else {
        /* uninstall hook if installed */
        if (original_compile_string) {
            zend_compile_string = original_compile_string;
            /* keep original_compile_string for possible re-enable later */
        }
    }
    return SUCCESS;
}


/**
 * Prompt the user for permission to allow eval execution.
 * Only used in CLI mode when interactive mode is enabled.
 */
static int ask_user_permission(zend_string *source_string)
{
    char answer[8];

    // Print the eval source
    fprintf(stderr, "Eval attempt detected:\n");
    fwrite(ZSTR_VAL(source_string), 1, ZSTR_LEN(source_string), stderr);
    fprintf(stderr, "\nAllow execution? [y/N]: ");
    fflush(stderr);

    if (fgets(answer, sizeof(answer), stdin))
    {
        if (answer[0] == 'y' || answer[0] == 'Y')
            return 1;
    }
    return 0;
}

/**
 * Log the string passed to eval() into a file.
 */
static void log_eval_string(zend_string *source_string, int allowed)
{
    smart_str message = {0};
    smart_str_appends(&message, "[EVAL_LOGGER_BEGIN]\n");
    smart_str_append(&message, source_string);
    smart_str_appends(&message, "\n[EVAL_LOGGER_END]");
    smart_str_0(&message); // null terminate

    if (EVAL_LOGGER_G(logfile) && strlen(EVAL_LOGGER_G(logfile)) > 0) {
        /* write to dedicated logfile */
        FILE *f = fopen(EVAL_LOGGER_G(logfile), "a");
        if (f) {
            fwrite(ZSTR_VAL(message.s), 1, ZSTR_LEN(message.s), f);
            fclose(f);
        } else {
            /* fallback to php error log if unable to open custom logfile */
            php_log_err(ZSTR_VAL(message.s));
        }
    } else {
        /* default: write to php error log (php.ini error_log or webserver log) */
        php_log_err(ZSTR_VAL(message.s));
    }

    smart_str_free(&message);
}

/**
 * Our wrapper around zend_compile_string.
 * Called whenever eval() is used. Logs the eval content before passing it to the original compiler.
 */
static zend_op_array *eval_logger_compile_string(zend_string *source_string, const char *filename, zend_compile_position compile_pos)
{
    // if (!EVAL_LOGGER_G(enable)) {
    //     return original_compile_string(source_string, filename, compile_pos);
    // }
    if (!EVAL_LOGGER_G(enable)) {
    php_error_docref(NULL, E_WARNING, "Eval blocked (eval_logger disabled in ini)");
    return NULL;
}

    int allowed = 1;

    if (sapi_module.name && strcmp(sapi_module.name, "cli") == 0 && EVAL_LOGGER_G(interactive)) {
        if (!ask_user_permission(source_string)) {
            php_error_docref(NULL, E_WARNING, "Eval execution denied by eval_logger (interactive)");
            allowed = 0;
        }
    }

    log_eval_string(source_string, allowed);

    if (!allowed || EVAL_LOGGER_G(mode) == 1) {
        return NULL; // block eval
    }

    return original_compile_string(source_string, filename, compile_pos);
}
/**
 * MINFO function: Displays information about the extension in phpinfo().
 */
PHP_MINFO_FUNCTION(eval_logger)
{
    php_info_print_table_start();
    php_info_print_table_header(2, "eval_logger support", "enabled");
    DISPLAY_INI_ENTRIES();
    php_info_print_table_end();
}

/**
 * MINIT function: Called when the extension is loaded.
 * Swaps out zend_compile_string for our custom logging version.
 */
PHP_MINIT_FUNCTION(eval_logger)
{
    ZEND_INIT_MODULE_GLOBALS(eval_logger, php_eval_logger_init_globals, NULL);
    REGISTER_INI_ENTRIES();

    if (EVAL_LOGGER_G(enable)) {
        if (!original_compile_string) {
            original_compile_string = zend_compile_string;
            zend_compile_string = eval_logger_compile_string;
        }
    }

    return SUCCESS;
}

/**
 * MSHUTDOWN function: Called when the extension is unloaded.
 * Restores the original zend_compile_string function pointer.
 */
PHP_MSHUTDOWN_FUNCTION(eval_logger)
{
    UNREGISTER_INI_ENTRIES();

    if (original_compile_string) {
        zend_compile_string = original_compile_string;
        /* keep original_compile_string as-is for possible re-init next load */
    }

    /* free allocated logfile string if any (non-ZTS case) */
#ifndef ZTS
    if (EG(active) && EVAL_LOGGER_G(logfile)) {
        efree(EVAL_LOGGER_G(logfile));
        EVAL_LOGGER_G(logfile) = NULL;
    }
#endif

    return SUCCESS;
}

// Module entry definition for the PHP engine
zend_module_entry eval_logger_module_entry = {
    STANDARD_MODULE_HEADER,
    "eval_logger",              // Extension name
    NULL,                       // Functions (none exposed)
    PHP_MINIT(eval_logger),     // Startup hook
    PHP_MSHUTDOWN(eval_logger), // Shutdown hook
    NULL,                       // RINIT
    NULL,                       // RSHUTDOWN
    PHP_MINFO(eval_logger),     // MINFO
    "1.1.1",                    // Extension version
    STANDARD_MODULE_PROPERTIES};

// Required macro for PHP to recognize the extension
ZEND_GET_MODULE(eval_logger)
