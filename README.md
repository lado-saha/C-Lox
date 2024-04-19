## Crafting Interpreters in C - Lox 

This project is my journey through the fantastic book

**["Crafting Interpreters" by Robert Nystrom](https://craftinginterpreters.com/)**.The book dives deep into how programming languages work by guiding readers to build their own interpreters and then their own compiler. It does this through the creation of a dynamically typed language called Lox, an Object oriented + functional dynamically typed PL. This is the C implemtation, from the second part of the book

The book is truly written in FUNNY but CONCISE language. I never knew cracking jokes could go hand in hand with the dark art of creating compilers

# Project Description:

We are currently building a single-pass compiled language called lox, which compiles source code into bytecode and execute it using a VM.. So far, I've implemented features like:

* A Parser (Using Pratt's Parsing technique) which produces tokens
* A bytecode representation
* A compiler which compiles the tokens to bytecode and pushes it to the chunk stack 
* A virtual machine to execute the bytecode
* Number types, String types, hash tables implemented
* And many more ...

## Future Work

* The main aim of learning this is to be able to learn reverse engineering of malware executables in my aim to becoming a cybersecurity expert

## How to test

* Clone the Project
* Navigate into the project, then... 
```bash
# To compile 
make 

# To run the REPL
make run

# You can write code in ./examples/test.lox and execute it using 
make example

# Else, if run any file as follows
make run path=<FILE PATH>   

```
**Debug Mode**

By default, the code runs in debug mode, which shows the stack at each phase. If you wish to disable debug mode, follow these steps:

1. Open the [`common.h`](./common.h) file in your project.
2. Locate the line `#define DEBUG_PRINT_CODE`.
3. Comment out this line by adding a `//` at the beginning, like so:
   ```c
   ...
   // #define DEBUG_PRINT_CODE
   ...
   ```
4. Save the file.

After commenting out the `DEBUG_PRINT_CODE` line in `common.h`, debug mode will be disabled, and the stack will not be printed at each phase.
   
## Conclusion:
>> NB: PENDING FINAL FILE ARBORESCENCE

Thank you for checking out my project! Feel free to explore the code and see how the language is taking shape.

