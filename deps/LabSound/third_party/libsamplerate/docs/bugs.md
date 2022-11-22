---
layout: default
---

# Bug Reporting

If you are a user and have a problem using libsamplerate with another piece of
software, you should contact the author of that other software and get them to
explore their use of this library and possibly submit a bug report. If you are a
coder and think you have found a bug in libsamplerate then read on.

Secret Rabbit Code is an extremely complex piece of code but I do think that it
is relatively bug free. In addition, the source code distribution includes a
comprehensive test suite for regression testing. This means it is extremely
unlikely that new bugs creep in when modifications are made to the code.

SRC is also not the most simple library to use which means that I do get a
number of bug reports which turn out to be bugs in the user's program rather
than bugs in SRC. Up until now, I have investigated each bug report as
thoroughly as possible. Unfortunately, this chews up a lot of my time which
could otherwise be spent improving SRC, working on other Free Software or
spending time with my family.

I have therefore decided, that I cannot investigate any bug report unless the
person reporting the problem can supply me with a short self contained test
program or a modification to one of the existing test programs in the tests/
directory of the source code distribution. The test program should meet the
following criteria:

- Written in C or C++.
- Does not use any libraries or header files other than the ones which are
  standard for the relevant languages. (Of course libsamplerate can be used :-)).
- It is the minimal program which can adequately display the problem.
- It clearly displays the criteria for pass or fail.

Supplying a good test program will maximize the speed with which your bug report
gets dealt with.
