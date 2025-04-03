## Object
- data and action associated
- size, type, allignment, storage duration, lifetime, value, name

## Funciton
- collection of statements

## Statement
- ends in ;
- can compile into many machine lang instructions
- Types:
    Declaration statements
    Jump statements
    Expression statements
    Compound statements
    Selection statements (conditionals)
    Iteration statements (loops)
    Try blocks

# Identifiers and Names
- start with a-zA-z_

# Utilities library [https://en.cppreference.com/w/cpp/utility]
## Function Object - functor
- function with state - functions are normally not objects

# I/O library [https://en.cppreference.com/w/cpp/io]
## basic_ios
- facilities to interface with `std::basic_streambuf` objects
## basic_ostream [https://en.cppreference.com/w/cpp/io/basic_ostream]
- cout, cerr, clog
## basic_istream [https://en.cppreference.com/w/cpp/io/basic_istream]
- cin

# Facts
- C: `call by value` -> objects are copied in and out of scopes
- C: `argv[argc] = 0`, `argv[argc+n]` -> environment information
- a pointer with value 0 is `nullptr (NULL)`
  - `*(int *)0 = 0;` is a nullptr dereference -> segfaults
- C has printf < C++ iostream < std::format < std:print < fmt (lib)
- dont use malloc() and free() in C++
- `\r` is carriage return, gets to start of line

# Recursion
- recursive function stack: `ulimit -s` 8mb, `ulimit -s unlimited`
- ever recursion problem can be solved iteractive (for/while)
  - for non-trivial problems recursion can be simpler to write and read
  - fibonacci for example -> `EX` try iteractive and recursive fibonacci 

# Unix commands
- locate, same as `find /path -name <name>` but can prebuild

# Need to learn about
- STL containers
- iterators, containers and algorithms
- templates, generics (function and class templates)
