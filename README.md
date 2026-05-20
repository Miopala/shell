# Description
Custom Unix shell implemented in C based on assignment from "Operating Systems: Three Easy Pieces".

## Files overview
* **`src/common.c`**: Common constants and functions used.
* **`src/main.c`**: Manages the core functionality, two possible modes: batch (file) and interactive (stdin).
* **`src/parser.c`**: Tokenizes the input and checks its correctness.
* **`src/builtin.c`**: Handles builtin commands like cd, path and exit.
* **`src/executor.c`**: Executes the subcommands by forking the children and doing execv function. Handles the pipes and redirection.