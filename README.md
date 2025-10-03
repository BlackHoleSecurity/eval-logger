Eval Logger
===========

**Eval Logger** is a lightweight PHP extension designed to **deobfuscate obfuscated PHP scripts that rely on `eval()`**. It transparently intercepts and logs every string passed to `eval()`, revealing the original code — no matter how many layers of obfuscation or `eval()` calls are used.

This tool is especially useful for security researchers, malware analysts, and developers trying to reverse-engineer or audit PHP files like webshells, obfuscated malware, or compressed loaders.

* * *

🚀 Features
-----------

*   Captures and logs all content passed to `eval()`.
*   Requires no modifications to the original script.
*   Simple and fast — written in native C as a Zend extension.
*   Supports PHP 8.x.

For PHP 7.x, use the legacy tool: [evalhook (PHP 7)](https://github.com/Cvar1984/evalhook)

* * *

📦 Installation
---------------

1.  **Install dependencies** (Fedora/RHEL-based distros):
    
        sudo dnf install php-devel php-pear gcc make autoconf
    
2.  **Build the extension**:
    
        phpize
        ./configure --enable-eval_logger
        make
        sudo make install

3.  **Check installed modules**:
    
        php -m
        [PHP Modules]
        eval_logger

**Note:** After building, the shared object (`eval_logger.so`) will be available inside the `modules/` and after installing it will also available inside ` /usr/lib64/php/modules/` directory.

* * *

✅ Usage
-------

To analyze a PHP file (e.g., `webshell.php`):
inline hot test (no installing)

    php -d extension=/full/path/to/eval_logger.so /full/path/to/webshell.php
    
installed to ` /usr/lib64/php/modules/`

    php /path/to/webshell.php
    
After execution, the evaluated strings will be logged to `error_log = ` set in `php.ini`

**Example:**

    php -d extension=/home/user/eval-logger/modules/eval_logger.so ~/Downloads/webshell.php
    cat ./error_log
    

* * *

📎 Notes
--------

*   The logger **appends** logs file set in `error_log = ` on every run (can be customized in source).
*   Only logs runtime-evaluated strings (e.g., `eval(base64_decode(...))`).
*   For maximum visibility, ensure the script being analyzed actually executes all evals (some may be conditionally triggered).

* * *

🧠 Why It Matters
-----------------

Obfuscated PHP code often uses nested or layered `eval()` calls to hide real logic. This extension makes such techniques **transparent** and **ineffective**, empowering developers and analysts to regain visibility into the actual code being executed.

* * *

🔗 Related Tools
----------------

*   [evalhook (PHP 7 version)](https://github.com/Cvar1984/evalhook)

* * *

📄 License
----------

GNU General Public License v3.0

* * *

Made with 🔍 by [Cvar1984](https://github.com/Cvar1984)
