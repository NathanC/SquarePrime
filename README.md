SquarePrime
===========

Multithreaded program written in ANSI C that generates magic squares using a simple evolutionary algorithm.

It uses the pthreads library, and currently only works on windows. However I plan to port it to Linux as well.

I wrote this for a project in a programming languages class over a year ago, and haven't really modified it since. It was one of the first non-trivial programs I wrote, so the code is a little messy. So far, the largest square I've evolved with it is of size 34x34, but evolving squares upwards of 20 takes trial and error and time. Magic squares of around 10x10 evolve almost instantly with the population set at 50,000.

When a square is evolved, it writes it out to a text file. I've included the largest square I've been able to evolve in the examples directory. The main weakness of this algorithm is that I believe it easily gets trapped in local optimums, but it's still a fun piece of code. 

To compile, simply run:
  gcc main.c -o magic_squares.exe -lpthread
