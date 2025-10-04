--TEST--
eval_logger should block eval() when no interaction is given on interactive mode
--INI--
eval_logger.enable=1
eval_logger.interactive=1
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
Warning: eval\(\): Eval execution denied by eval_logger \(interactive\) in .*004-mode-interactive\.php on line [0-9]+
after

