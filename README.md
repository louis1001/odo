# Odo
[![Generic badge](https://img.shields.io/badge/version-0.2_beta-blue.svg)](https://shields.io/)

A port to C++ of my personal programming language.

---
<sup>The README is a work in progress.</sup>

<img src="./docs/assets/imgs/logo.png?raw=true" height="110" alt="logo"></img>

This is my first attempt at making a usable, personalized interpreted programming language.
*First* as in, the specs. This is the 2nd time I port this language, the first edition was written
in `Swift`.

This project is me practicing C++ in real-world, decently sized applications, and applying some
knowledge about designing programming languages. Most of this techniques I learnt from
[Let's build a simple interpreter](https://ruslanspivak.com/lsbasi-part1/) by Ruslan Spivak.

Considering I have very little experience in C++, this project has a lot of bugs and bad practices.
But from this commit on, I'll be focusing on refactoring my code so it doesn't have as many basic mistakes.

## Build
The requirements for building the project are:
- A C/C++ compiler for your system (I use clang, but gcc/g++ should work too).
- [CMake](https://cmake.org/) installed.


When you have the requirements, building is done as follows:

- Open a terminal in the directory of the project
- `mkdir` a directory called build, and `cd` (or `dir` on windows.) into it.
- Run the command `cmake ..` and then `make`.
- Now you can use `odo` as the executable in the build directory.

## Design

The main objective is having a typed interpreted language that implements features from
all my favorite real languages. Many of the decisions were taking on the premise "does this convention exist
because it's practical, or because of the limitations of the time that language was designed?". For example,
`;` at the end of statements (in Odo they're optional). Or parenthesis surrounding the conditions in `if` or `while`.

## Example Code

Some example code written in odo:

```GO
/* fibonacci.odo */
int n = 45

int x = 0
int y = 0
int z = 1

for(int i = 0; i < n; i++){
    println(i, ":", z)
    x = y
    y = z
    z = z + x
}
```
(In this one it looks really similar to C. It could compile, except for the lack of semicolons.)

---
```GO
println("Testing reversing a string")

string first = read()

string result = ""
for(int i = length(first)-1; i >= 0; i--) {
    result += first[i]
}

println(result)
```

## Some current problems

The metaprogramming in this project is awful.
There's a comment in `Symbol.h` that explains it a little.

I'm gonna refactor the whole project, specially the `Interpreter` section, and use polymorphism instead
of the way I'm doing things now, putting every kind of member variable a value can have in every single
value, even though they're not being used.

**New Logo!**
(And the ascii art if you type about() on the repl)

Optimizations:
- Faster value and symbol lookup
- Visual optimization of double to string values
- Symbol identifier now is a int instead of string (faster comparisons)
- Value now has a pointer to its type instead of taking a copy of its value
- double to_string conversion now checks if the value is just an integer.

Bug fixes:
- Any_type pointer was inconsistent/relying on shaky memory.
- The null value was inconsistent/relying on shaky memory (when assigning its type).
- For loop wasn't resetting the continuing flag.
- Function in Value now checks if the value can or should be copied.
- A comparison with null shouldn't fail because anything can be null.
- The c++ compiler now warns on every little bug (hopefully).
- The *null* value was getting the address 1 by mistake. I should be 0.
- The use of static statements outside of a class is now an error.
- lists now copy their contents (if they're copyable) as well as the list_value.
- Native functions now checks if any of its arguments was already important.
- Removed unused code.

New Features:
- **typeof function**. Returns the type name of the value passed
- **pop function**. Deletes the las element from a list and returns it
- **function callstack**. Errors now have more meaning! Prints a traceback of the error.
- **valueAt function**. This is debug only. Returns a value given an address in the valueTable.
- **foreach and forange statement**. This is an improvement over the c-like for. 
The actual loop should have now native speeds.
- **new "last result" variable**. In the repl you can now type _ to get the last value returned.
