# Odo
[![Generic badge](https://img.shields.io/badge/version-0.3_beta-blue.svg)](https://shields.io/)

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

Maybe using shared_ptr everywhere isn't the best. I'll work on this, 
maybe getting weak pointers here and there.

I have to work on separating the interpreter from the native functions. I also want to make a
standard library with different modules. Now that I have each kind of value by itself, I can
make one kind of value that runs C++ code.