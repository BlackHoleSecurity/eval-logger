Eval Logger
===========

**Eval Logger** is a lightweight PHP extension designed to **deobfuscate obfuscated PHP scripts that rely on `eval()`**.  
It transparently intercepts and logs every string passed to `eval()`, revealing the original code — no matter how many layers of obfuscation or `eval()` calls are used.

This tool is especially useful for security researchers, malware analysts, and developers trying to reverse-engineer or audit PHP files like webshells, obfuscated malware, or compressed loaders.

---

🚀 Features
-----------

* Captures and logs all content passed to `eval()`.
* Requires no modifications to the original script.
* Simple and fast — written in native C as a Zend extension.
* Supports PHP 8.x.

For PHP 7.x, use the legacy tool: [evalhook (PHP 7)](https://github.com/Cvar1984/evalhook)

---

📦 Installation
---------------

### Linux (Fedora/RHEL/Debian)

1. **Install dependencies**:

       sudo dnf install php-devel php-pear gcc make autoconf
       # or Debian/Ubuntu:
       sudo apt install php-dev php-pear build-essential autoconf

2. **Build the extension**:

       phpize
       ./configure --enable-eval_logger
       make
       sudo make install

3. **Check installed modules**:

       php -m
       [PHP Modules]
       eval_logger

**Note:** After building, the shared object (`eval_logger.so`) will be inside `modules/`, and after installing it will also be available inside `/usr/lib64/php/modules/` (or distro equivalent).

---

### Windows

1. **Install PHP development tools**  
   - Download and install [PHP for Windows](https://windows.php.net/download/).  
   - Ensure you install the correct **Thread Safe (TS)** or **Non-Thread Safe (NTS)** build depending on your setup.  
   - Install **php-devel package** for Windows (contains headers for extension building).

2. **Install build tools**  
   - Install [Visual Studio Build Tools](https://visualstudio.microsoft.com/visual-cpp-build-tools/).  
   - Make sure to include *Desktop development with C++*.  
   - Add `cl.exe` and `nmake.exe` to your PATH (usually inside `C:\Program Files (x86)\Microsoft Visual Studio\...`).

3. **Build the extension**:

       phpize
       configure --enable-eval_logger
       nmake

   This will produce `php_eval_logger.dll` in the `Release_TS` or `Release_NTS` directory.

4. **Enable the extension in `php.ini`**:

       extension=php_eval_logger.dll

---

✅ Usage
-------

To analyze a PHP file (e.g., `webshell.php`):

### Inline hot test (no installing)

Linux:

    php -d extension=/full/path/to/eval_logger.so /full/path/to/webshell.php

Windows:

    php -d extension=C:\path\to\php_eval_logger.dll C:\path\to\webshell.php

### Installed globally

Linux:

    php /path/to/webshell.php

Windows:

    php C:\path\to\webshell.php

After execution, the evaluated strings will be logged to the file defined in your `php.ini`:

Linux:

    error_log = /var/log/php_errors.log

Windows:

    error_log = C:\php\logs\php_error.log

**Example (Linux):**

    php -d extension=/home/user/eval-logger/modules/eval_logger.so ~/Downloads/webshell.php
    cat /var/log/php_errors.log

**Example (Windows):**

    php -d extension=C:\eval-logger\php_eval_logger.dll C:\Users\User\Downloads\webshell.php
    type C:\php\logs\php_error.log

---

📎 Notes
--------

* The logger **appends** logs file set in `error_log =` on every run (can be customized in source).  
* Only logs runtime-evaluated strings (e.g., `eval(base64_decode(...))`).  
* For maximum visibility, ensure the script being analyzed actually executes all evals (some may be conditionally triggered).  
* Works cross-platform (Linux, Windows) with platform-specific build instructions.  

---

🧠 Why It Matters
-----------------

Obfuscated PHP code often uses nested or layered `eval()` calls to hide real logic.  
This extension makes such techniques **transparent** and **ineffective**, empowering developers and analysts to regain visibility into the actual code being executed.

---

🔗 Related Tools
----------------

* [evalhook (PHP 7 version)](https://github.com/Cvar1984/evalhook)

---

📄 License
----------

GNU General Public License v3.0

---

Made with 🔍 by [Cvar1984](https://github.com/Cvar1984)

