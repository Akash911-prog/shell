# Shell

## Command line grammar

``` code
# Top-level command line: sequences of pipelines separated by semicolons
command_line  → pipeline ( TOKEN_SEMICOLON pipeline )*

# Pipeline: one or more commands connected with pipes
pipeline      → command ( TOKEN_PIPE command )*

# Command: a simple command with optional redirects and background execution
command       → (TOKEN_WORD+ | sub_command) redirect* TOKEN_BACKGROUND?

# Simple command: either a sequence of words (command + args) or a parenthesized sub-command
sub_commands → TOKEN_WORD+ 
               | TOKEN_LPAREN command_line TOKEN_RPAREN

# Redirects: all possible input/output/error redirections
redirect      → TOKEN_REDIRECT_IN TOKEN_WORD
               | TOKEN_REDIRECT_OUT TOKEN_WORD
               | TOKEN_REDIRECT_APPEND TOKEN_WORD
               | TOKEN_REDIRECT_ERR TOKEN_WORD
               | TOKEN_REDIRECT_ERR_APPEND TOKEN_WORD

# Logical expressions using AND/OR operators
logical_expr  → command_line ( TOKEN_AND command_line | TOKEN_OR command_line )*
```
