# eval-logger
this is a tool to deobfuscate php script who use eval in the technique, it return original string from the eval, so no matter how much you use eval, it will useless
# Install
```sh
sudo dnf install php-devel php-pear gcc make autoconf
phpize
./configure --enable-eval_logger
make
```
# Test
```sh
php -d extension=/home/user/eval-logger/modules/eval_logger.so ~/Downloads/webshell.php
cat eval_log.txt
```
