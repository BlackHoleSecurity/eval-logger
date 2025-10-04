--TEST--
eval_logger should allow eval() in non interactive mode
--INI--
eval_logger.enable=1
eval_logger.interactive=0
--FILE--
<?php
echo "before\n";
eval("echo 'eval executed\n';");
echo "after\n";
?>
--EXPECT--
before
eval executed
after

