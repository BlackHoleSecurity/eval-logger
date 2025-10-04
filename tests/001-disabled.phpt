--TEST--
eval_logger should do nothing when disabled
--INI--
eval_logger.enable=0
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

