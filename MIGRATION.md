# Shell Migration Strategy

## Target File Structure

```
src/
├── main.c
├── lexer.c / lexer.h         ← Phase 2
├── ast.c / ast.h             ← Phase 3
├── parser.c / parser.h       ← Phase 3
├── expander.c / expander.h   ← Phase 4
├── executor.c / executor.h   ← Phase 5
├── builtins.c / builtins.h   ← Phase 5
├── variables.c / variables.h ← Unchanged
└── utils.c / utils.h         ← Unchanged
```

---

## Phase 1 — Foundation

> Goal: Set up new files without breaking anything existing

- [x] **1.1** Create `ast.h` with `NodeType` enum and `ASTNode` struct skeleton
- [x] **1.2** Create `ast.c` with stub `ast_node_create()` and `ast_free()`
- [x] **1.3** Rename `tokens.c / tokens.h` → `lexer.c / lexer.h`
- [x] **1.4** Update all `#include "tokens.h"` references to `#include "lexer.h"`
- [x] **1.5** Create empty stub files: `parser.c/h`, `expander.c/h`, `executor.c/h`, `builtins.c/h`

**✅ Checkpoint:** Project compiles. Shell works exactly as before.

---

## Phase 2 — Lexer

> Goal: Replace old tokenizer with a proper state machine lexer

- [x] **2.1** Add `position` field to `Token` struct (for error messages later)
- [ ] **2.2** Change fixed `tokens[50][50]` → dynamic `TokenList` with `create/push/free`
- [ ] **2.3** Implement state machine in `lex()`:
  - [x] Whitespace skipping
  - [x] Single-char operators: `|` `>` `<` `&` `;`
  - [x] Multi-char operators: `||` `&&` `>>` `<<` (check these FIRST)
  - [x] Single quotes `'...'` → `is_literal = true`
  - [x] Double quotes `"..."` → `is_quoted = true`
  - [x] Escape characters `\` → strip backslash, keep next char
  - [x] Regular words → set `needs_expansion` if contains `$` `*` `~`
  - [x] EOF token at end
- [ ] **2.4** Replace old `tokenize()` call in `main.c` with `lex()`

**✅ Checkpoint:** Manually test these inputs print correct token lists:

```
echo $USER | grep john
echo "hello world"
echo 'no $expand'
ls > out.txt
cmd1 && cmd2
```

---

## Phase 3 — AST + Parser

> Goal: Build a tree that represents command structure and relationships

- [ ] **3.1** Flesh out `ast.h`:
  - [ ] `Redirect` struct (filename, type, fd)
  - [ ] All `NodeType` values: `COMMAND` `PIPELINE` `AND` `OR` `SEQUENCE` `SUBSHELL`
  - [ ] Full `ASTNode` struct (args, redirects, left, right, body, background)
- [ ] **3.2** Implement `ast.c`:
  - [ ] `ast_node_create()`
  - [ ] `ast_free()` — must free all children recursively, no leaks
  - [ ] `ast_print()` — for debugging, prints tree with indentation

**✅ Checkpoint:** Can manually create and free AST nodes, no crashes.

- [ ] **3.3** Implement parser helpers in `parser.c`:
  - [ ] `peek()` — look at current token
  - [ ] `consume()` — take current token and advance
  - [ ] `check()` — is current token this type?
  - [ ] `match()` — consume if matches, return bool
- [ ] **3.4** Implement `parse_command()` — collects words and redirects
- [ ] **3.5** Implement `parse_pipeline()` — handles `|`
- [ ] **3.6** Implement `parse_command_line()` — handles `&&` `||` `;`
- [ ] **3.7** Implement subshell parsing — handles `(` and `)`
- [ ] **3.8** Add `parse()` + `parser_create()` + `parser_free()`
- [ ] **3.9** Add lex → parse to `main.c`, use `ast_print()` to verify

**✅ Checkpoint:** Test these parse correctly (use ast_print to verify):
```
echo hello                        → COMMAND [echo, hello]
echo hello | grep h               → PIPE(COMMAND[echo,hello], COMMAND[grep,h])
ls && echo done                   → AND(COMMAND[ls], COMMAND[echo,done])
cat file.txt > out.txt            → COMMAND[cat, file.txt] redirect[> out.txt]
(echo hello)                      → SUBSHELL(COMMAND[echo, hello])
echo a | grep a && echo found     → AND(PIPE(...), COMMAND[echo,found])
```

---

## Phase 4 — Expander

> Goal: Resolve all lazy values in the AST before execution

- [ ] **4.1** Implement `expand_string()`:
  - [ ] `$VAR` syntax
  - [ ] `${VAR}` syntax
  - [ ] `$?` — last exit status
  - [ ] `$$` — shell PID
- [ ] **4.2** Implement tilde expansion:
  - [ ] `~` alone → `HOME`
  - [ ] `~/path` → `HOME/path`
- [ ] **4.3** Implement escape sequences: `\n` `\t` `\\` `\"` `\'`
- [ ] **4.4** Implement `expand(ASTNode*)` to walk the tree:
  - [ ] Expand all args in `COMMAND` nodes
  - [ ] Expand redirect filenames
  - [ ] Skip `is_literal` tokens (single quoted)
  - [ ] Skip glob expansion in `is_quoted` tokens (double quoted)
  - [ ] Recurse into `PIPELINE` `AND` `OR` `SEQUENCE` `SUBSHELL`
- [ ] **4.5** Add expand step to `main.c` after parse

**✅ Checkpoint:** Test these expand correctly:
```
"echo $USER"       → args = ["echo", "john"]
"echo ~/docs"      → args = ["echo", "/home/john/docs"]
"echo '$USER'"     → args = ["echo", "$USER"]   ← literal, no expansion
"echo \"$USER\""   → args = ["echo", "john"]    ← quoted, still expands vars
```

---

## Phase 5 — Executor

> Goal: Replace old command runner with AST walker

### 5.1 — Builtins

- [ ] Create `builtins.h` with updated `Builtin` struct
- [ ] Update handler signature: `int handler(ASTNode *node)`
- [ ] Migrate `command_metadata` → `builtin_metadata` in `builtins.c`
- [ ] Update each handler to use `node->args[]` instead of `tokens[][]`:
  - [ ] `echo`
  - [ ] `cd`
  - [ ] `pwd`
  - [ ] `type`
  - [ ] `which`
  - [ ] `clear`
  - [ ] `exit`
- [ ] Update `get_builtin()` lookup function

**✅ Checkpoint:** All builtins compile with new signatures.

### 5.2 — Basic Command Execution

- [ ] Implement `execute_command()`:
  - [ ] Check builtins first via `get_builtin()`
  - [ ] Validate arg count, print usage if wrong
  - [ ] Find + run external executables from PATH
- [ ] Connect `execute()` to `main.c`

**✅ Checkpoint:** `echo`, `cd`, `pwd`, `type`, `which`, `clear` all work through new executor.

### 5.3 — Sequence and Conditionals

- [ ] Implement `NODE_SEQUENCE` (`;`)
- [ ] Implement `NODE_AND` (`&&`)
- [ ] Implement `NODE_OR` (`||`)

**✅ Checkpoint:**
```
echo hello ; echo world       → prints both
true && echo yes              → prints yes
false && echo yes             → prints nothing
false || echo fallback        → prints fallback
true || echo fallback         → prints nothing
```

### 5.4 — Redirects

- [ ] `>` redirect stdout to file (overwrite)
- [ ] `>>` redirect stdout to file (append)
- [ ] `<` redirect stdin from file
- [ ] `2>` redirect stderr to file
- [ ] `2>>` redirect stderr to file (append)

**✅ Checkpoint:**
```
echo hello > out.txt          → file contains "hello"
echo world >> out.txt         → file contains "hello\nworld"
cat < out.txt                 → prints file contents
ls fakefile 2> err.txt        → error written to file
```

### 5.5 — Pipes

- [ ] Implement `execute_pipeline()`:
  - [ ] Create OS pipe (read_end, write_end)
  - [ ] Fork left command → stdout → write_end
  - [ ] Fork right command → stdin → read_end
  - [ ] Parent waits for both children
- [ ] Multi-pipe comes free from recursive AST walk

**✅ Checkpoint:**
```
echo hello | grep hello       → prints hello
ls | grep .c                  → lists .c files
cat file | grep x | sort      → multi-pipe works
```

### 5.6 — Background Jobs

- [ ] Check `node->background` flag in executor
- [ ] Fork but don't wait for child
- [ ] Print PID of background job

**✅ Checkpoint:**
```
sleep 5 &                     → returns immediately, prints PID
```

### 5.7 — Subshell

- [ ] Implement `NODE_SUBSHELL`
- [ ] Fork child, execute body, parent waits

**✅ Checkpoint:**
```
(cd /tmp && pwd)              → prints /tmp
pwd                           → prints original dir (unchanged)
```

### 5.8 — Full Integration

- [ ] Replace all old execution logic in `main.c` with `execute(ast)`
- [ ] Store last exit status in `$?` variable after each command

**✅ Checkpoint:** Full pipeline working end to end:
```
echo $USER | grep $USER && echo "found" > out.txt
```

---

## Phase 6 — Cleanup

> Goal: Remove all old code, clean compile

- [ ] **6.1** Delete old `tokenize()` function from `lexer.c`
- [ ] **6.2** Delete old command execution logic from `main.c`
- [ ] **6.3** Delete `command_info.c/h` (absorbed into `builtins.c/h`)
- [ ] **6.4** Remove old handler signatures from `commands.h`
- [ ] **6.5** Delete `commands.c/h` if fully replaced by `builtins.c` + `executor.c`
- [ ] **6.6** Update Makefile to include all new files
- [ ] **6.7** Compile with `-Wall -Wextra`, fix all warnings

**✅ Final Checkpoint:** Clean compile, zero warnings. All features work:
```
echo $USER                    ← variable expansion
cd ~/Documents                ← tilde expansion
ls | grep .c                  ← pipes
echo hi > out.txt             ← redirects
true && echo yes              ← conditionals
false || echo fallback        ← conditionals
echo a ; echo b               ← sequence
(cd /tmp && pwd)              ← subshell
sleep 3 &                     ← background
```

---

## Golden Rules

1. **One phase at a time** — never start Phase 3 until Phase 2 checkpoint passes
2. **Compile between every step** — small steps = easy to find bugs
3. **Keep old code until new code works** — don't delete until replacement is verified
4. **Test each checkpoint manually** — type the command, verify the output
5. **If stuck** — comment out new code, revert to last checkpoint, understand before continuing

---

## Estimated Effort

| Phase | Difficulty | Time |
|-------|------------|------|
| 1 - Foundation | Easy | ~30 min |
| 2 - Lexer | Medium | ~2-3 hrs |
| 3 - Parser + AST | Hard | ~4-6 hrs |
| 4 - Expander | Medium | ~2-3 hrs |
| 5 - Executor | Hard | ~6-8 hrs |
| 6 - Cleanup | Easy | ~30 min |
