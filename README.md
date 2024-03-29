
# Cx - A C-to-C Translator and Useful Functions/Classes/Tools

## Deprecation Notice
Useful functions and data structures have moved to the [Fildesh](https://github.com/fildesh/fildesh/) scripting language codebase.
However, this project's handful of C-to-C transformations have been abandoned; they added more confusion that they were worth.

## Description

This is a library that I use to make ANSI C / C++ programming easier.
Many research tools and domain-specific languages build upon ANSI C or a subset of it, so it's not an entirely useless exercise to stick to that standard.
You will need to put the repository of preprocessed files (https://github.com/grencez/cx-pp) in `../cx-pp` in order to build this.
Building is easy though, just typing `make` from the top-level directory will perform the following steps:
```
mkdir -p bld
cd bld
cmake ../src
make
```

From the top-level directory, translate C code by:
```
./bin/cx -x in.c -o out.c
```

## Miscellaneous

Besides the C-to-C translator, there are some miscellaneous things that loosely fit with the idea of a useful library.

* **Standalone** tool that helps embed file contents in C code.
 * [cembed.c](src/cembed.c)
 * Example: `cembed -o shaders.h phong.vert phong.frag diffuse.vert diffuse.frag` and then include `shaders.h` within a function.
* **Standalone** tool that creates a C switch statement for strings.
 * [cswitch.c](src/cswitch.c)
 * Example: `cswitch strvbl -x fnlist.txt -case-pfx 'fn = testfn_' -o switch.c` and then include the `switch.c` within a function to have a switch statement that matches `strvbl` to a line of `fnlist.txt`.
If a match is found, it assigns `fn` to a function whose name starts with `testfn_` and ends with the matched string.
* MPI dissemination scheme using Kautz graphs.
 * [mpidissem.cc](src/mpidissem.cc) and [mpidissem.hh](src/mpidissem.hh)
* LaTeX macros and preamble.
 * [predef.tex](doc/predef.tex) and [preamble.tex](doc/preamble.tex)

## Translations

The primary reason this tool exists is to write for-loops in a reasonable way while retaining ANSI compliance.
It will also strip comments.
The first (common) and second (specialized) loop versions shown below are translated to the third (ANSI-compliant) version.
Great care is taken to preserve line numbers since a debugger will show the translated code.
```
for (unsigned int i = 0; i < n; ++i) {
  // Compute!
}
for (i ; n) {
  // Compute!
}
{unsigned int i; for (i = 0; i < n; ++i) {

}}
```

Braces are also allowed to come before `for`, `while`, `do`, `if`, `else`, and `switch` keywords if the brace is written as `{:`.
For example, the following code is equivalent to the above loops.
```
{:for (i ; n)
  // Compute!
}
```

To avoid warnings for pragmas, `cx` can strip them if run with `-no-pragma`.
For example, the following command removes OpenMP pragmas:
```
cx -no-pragma omp -x in.c -o out.c
```
To strip all quoted includes, run:
```
cx -no-quote-includes -x in.c -o out.c
```

For convenience, C++/other code is supported via the `-c++` or `-shallow` flag.
No fancy source translations occur in this mode, just optional pragma and include.

