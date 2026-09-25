# Description

A small Unix shell written in C, based on the shell project from "Operating Systems: Three Easy Pieces". It supports interactive and batch modes, built-in commands (`cd`, `path`, `exit`), pipelines, input and output redirection, and parallel commands separated by `&`.

## Build

```bash
make
```

## Usage

Interactive mode:

```bash
./shell
```

Batch mode:

```bash
./shell commands.txt
```

## Files overview
* **`src/common.c`**: Common constants and functions used.
* **`src/main.c`**: Manages the core functionality, two possible modes: batch (file) and interactive (stdin).
* **`src/parser.c`**: Tokenizes the input and checks its correctness.
* **`src/builtin.c`**: Handles builtin commands like cd, path and exit.
* **`src/executor.c`**: Executes the subcommands by forking the children and executing binaries via execv function. Handles the pipes and redirection.