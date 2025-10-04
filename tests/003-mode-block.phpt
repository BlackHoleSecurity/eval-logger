--TEST--
eval_logger should block eval() when mode=1
--INI--
eval_logger.enable=1
eval_logger.mode=1
--FILE--
<?php
echo "before\n";
eval("echo 'eval executed\n';");
echo "after\n";
?>
--EXPECTREGEX--
before
Eval attempt detected:
echo 'eval executed
';
Allow execution\? \[y\/N\]: 
Warning: eval\(\): Eval execution denied by eval_logger \(interactive\) in .*003-mode-block\.php on line [0-9]+
after

