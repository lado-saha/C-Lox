## Crafting Interpreters in C - Lox 

This project is my journey through the fantastic book "Crafting Interpreters" by **Robert Nystrom** The book dives deep into how programming languages work by guiding readers to build their own interpreters and then their own compiler. It does this through the creation of a dynamically typed language called Lox. This is the C implemtation, from the second part of the book

**Project Description:**
We are currently building an interpreted language called lox, which compiles source code into bytecode and execute it using a VM.. So far, I've implemented features like:

* A Parser (Using Pratt's Parsing technique) which produces tokens
* A bytecode representation
* A compiler which compiles the tokens to bytecode and pushes it to the chunk stack 
* A virtual machine to execute the bytecode
* Number types, String types, hash tables implemented
* And many more ...

**Future Work (Optional):**

As I progress through the book, I plan to implement features like:

* The main aim of learning this is to be able to learn reverse engineering of malware executables in my aim to becoming a cybersecurity expert


**Conclusion:**

Thank you for checking out my project! Feel free to explore the code and see how the language is taking shape.

