PHP_ARG_ENABLE(eval_logger, whether to enable eval_logger,
[  --enable-eval_logger   Enable eval_logger support])

if test "$PHP_EVAL_LOGGER" = "yes"; then
  PHP_NEW_EXTENSION(eval_logger, eval_logger.c, $ext_shared)
fi

