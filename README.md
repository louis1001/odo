# Odo
[![Generic badge](https://img.shields.io/badge/version-0.4_beta-blue.svg)](https://shields.io/)

My personal programming language. Written in C++.

---
<sup>The README is a work in progress.</sup>

<img src="./docs/assets/imgs/logo.png?raw=true" height="110" alt="logo"></img>

This is my first attempt at making a usable, personalized interpreted programming language.
*First* as in, the specs. This is the 2nd time I port this language, the first edition was written
in `Swift`. At this point, the C++ version has become the main branch of the project.

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

Lately I've also renamed the `print` function to `write`, since the meaning of printing has changed considerably since
the time that function was originally named.

Another change done to odo's syntax was removing the double operators `&&` and `||`, and instead using the actual words
they describe.

## Example Code

Some example code written in odo:

### \> fibonacci.odo
```
#{ fibonacci.odo }#
int n = 45

int x = 0
int y = 0
int z = 1

forange i: n{
    writeln(i, ":", z)
    x = y
    y = z
    z = z + x
}
```

---
### \> string_reversed.odo
```
writeln("Testing reversing a string")

string first = read()

string result = ""
foreach char ~: first {
    result += char
}

writeln(result)
```

## Some current problems

Maybe using shared_ptr everywhere isn't the best. I'll work on this, 
maybe getting weak pointers here and there.

I want to make a standard library with different modules.
For the moment, the native module section of odo is a little barebones.