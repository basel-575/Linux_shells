# Incremental Shell Project

This repository contains a step-by-step implementation of a Unix-like shell, progressing from a basic command interpreter to a feature-rich shell supporting variables and I/O redirection.

## Project Structure

The project is divided into four stages of complexity, each building upon the concepts of the previous version.

### 1. Femto Shell (`femto_shell.cpp`)
The most basic iteration of the shell.
- **Features:**
  - Infinite command loop.
  - Built-in `echo` command.
  - Built-in `exit` command to terminate the shell.
- **Goal:** Establishes the basic Read-Eval-Print Loop (REPL).

### 2. Pico Shell (`pico_shell.c`)
Introduces system calls and process management.
- **Features:**
  - Executes external system commands (e.g., `ls`, `date`, `vi`) using `fork()` and `execvp()`.
  - Built-in `pwd` (Print Working Directory).
  - Built-in `cd` (Change Directory).
  - Error handling for invalid commands.
- **Goal:** Handles basic navigation and process execution.

### 3. Nano Shell (`Nano_shell.cpp`)
Adds environment and variable management.
- **Features:**
  - **Local Variables:** Assign variables using `VAR=value`.
  - **Variable Expansion:** Access variables using `$VAR`.
  - **Environment Variables:** Falls back to system environment variables if a local variable is not found.
  - **Built-ins:** `export` (to set environment variables) and `printenv`.
  - **Test Mode:** `test_mode_on` / `test_mode_off` to suppress prompt output for automated testing.
- **Goal:** Manages shell state and environment data.

### 4. Micro Shell (`Micro_shell.cpp`)
The most advanced version, introducing input/output manipulation.
- **Features:**
  - **Input Redirection (`<`):** Read input from a file (e.g., `cat < input.txt`).
  - **Output Redirection (`>`):** Write output to a file (e.g., `ls > list.txt`).
  - **Error Redirection (`2>`):** Redirect error messages (e.g., `gcc main.c 2> errors.log`).
  - Includes all features from Nano Shell (variables, expansion, etc.).
- **Goal:** Implements standard Unix I/O streams functionality.

## Compilation & Usage

**Note:** The source files define specific entry points (e.g., `femtoshell_main`, `picoshell_main`). To compile them as standalone executables, ensure you rename the entry function to `main` or compile them with a wrapper.

### Prerequisites
- GCC / G++ compiler
- Linux/Unix environment

### Build Commands

```bash
# Compile Femto Shell
g++ femto_shell.cpp -o femto_shell

# Compile Pico Shell
gcc pico_shell.c -o pico_shell

# Compile Nano Shell
g++ Nano_shell.cpp -o nano_shell

# Compile Micro Shell
g++ Micro_shell.cpp -o micro_shell
