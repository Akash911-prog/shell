# 🐚 Shell

A lightweight Windows shell built in C, featuring a recursive descent parser and AST-based execution engine.

## How It Works

Input is processed in four stages:

1. **Lexer** — tokenizes raw input into a flat token list
2. **Parser** — consumes tokens via recursive descent and builds an Abstract Syntax Tree (AST)
3. **Expander** — walks the AST and expands variables, `~`, and arithmetic expressions
4. **Executor** — traverses the AST and executes each node

## Features

### Variable Support
Assign and use variables just like a standard shell:
```sh
name=rohan
echo $name        # rohan
echo ${name}      # rohan
greeting=hello
echo $greeting $name  # hello rohan
```

### Pipes
Chain commands together with `|`:
```sh
ls | sort
echo hello world | find "world"
```

### Redirects
Redirect input and output to files:
```sh
echo hello > out.txt       # write stdout to file
echo hello >> out.txt      # append stdout to file
sort < input.txt           # read stdin from file
echo err 2> err.txt        # redirect stderr
echo err 2>> err.txt       # append stderr
```

### External Executables
Run any `.exe` available on your PATH:
```sh
git status
python script.py
notepad file.txt
```

### Built-in Commands

| Command | Description |
|---------|-------------|
| `cd`    | Change directory |
| `ls`    | List directory contents |
| `echo`  | Print text to stdout |

### Tilde Expansion
`~` expands to the current user's home directory:
```sh
cd ~
ls ~/Documents
```

### Logical Operators
Chain commands with `&&` and `||`:
```sh
cd mydir && echo "success"
cd missing || echo "not found"
```

### Background Execution
Run commands in the background with `&`:
```sh
someprocess.exe &
```

### Subshells
Group commands in a subshell with `()`:
```sh
(cd subdir && ls)
```

### Arithmetic Expressions
Evaluate arithmetic with `$(( ))`:
```sh
echo $((2 + 2))     # 4
echo $((10 * 3))    # 30
```

## Building

```sh
mkdir build
cd build
cmake ..
cmake --build .
```

## Platform

> ⚠️ Currently **Windows only**.