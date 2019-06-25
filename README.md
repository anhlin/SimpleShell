Originally written for the FreeBSD Operating System
- Modified Makefile to work on MacOS/Linux 

Note: Need to run as root to use some commands such as cat

Files Included:
- lex.l: generates lexical analyzer with flex 
- myshell.c: main code for the shell
- Makefile: compiles files with flags, generates lex.yy.c, .o file, and executable 
	    when "make" is entered. Must "make clean" before making again.
- DESIGN.txt: Description of the design of the program. 

Implemented:
1. exit command which terminates the shell
2. All commands with no arguments.
3. Command with arguments
4. Command with/without arguments, whose output is redirected to a file
5. Command with/without arguments, whose output is appended to a file
6. Command with/without arguments, whose input is redirected from a file 
7. Command with/without arguments, whose output is piped to the input of another command,
   whose output is further piped to the input of another command. 

Also implemented: 
- Multiple commands separated by semicolons
- Piping with two commands
- Error checking for bad/failed commands 

Not implemented:
- Multiple redirection, e.g. wc -l < file >> file2
- More than three commands with piping

Notes:
- The prompt (">") for the shell has some weird behavior after using piping commands.
- Makefile does not run the flags on the lex.yy.c file
