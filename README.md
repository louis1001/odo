# Odo
A port to C++ of my personal programming language.

---
<sup>The README is a work in progress.</sup>

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


**This will be edited in a moment, but in the mean time, just please investigate how to build a CMake project
in your platform.**
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

I'm having trouble with the "Garbage Collector"™
A list contains symbols, and those point to values. When I wanna destroy a list, the values should go too.
That's the problem. Apparently, I'm destroying too much, or too little.
Because of this, the example `hangman.odo` is not working.

Also, the metaprogramming in this project is awful.
There's a comment in `Symbol.h` that explains it a little.

I'm gonna refactor the whole project, specially the `Interpreter` section, and use polymorphism instead
of the way I'm doing things now, putting every kind of member variable a value can have in every single
value, even though they're not being used.

Also, if anyone wants to help compiling the project in Linux and Windows, that'd be amazing.
I haven't tested that yet, but from what I gather, it shouldn't be too difficult.
