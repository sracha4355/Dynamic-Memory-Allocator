# Dynamic-Memory-Allocator
My own simplified implementation of malloc, realloc, calloc, and free on Linux

## Purpose of this project
I wanted to learn more about the what, how, and why of dynamic memory allocation, so as a fun little project I wrote my own simplifed dynamic memory allocator.

## How to use
Link alloc.c with alloc.h and include the header whenever you want to use it. Sample makefile is provided.
The standard functions have different names
- malloc is m_alloc
- free is m_free
- calloc is c_alloc
- realloc is re_alloc
They take the same arguments and are used the same way as the standard malloc, realloc, calloc, and free.

## What I learned
I learned a lot about system calls, namely sbrk, pointer arithmetic, writing modular code, organizing chunks of memory and reusing allocated memory efficiently.

## How it works?
The approach is very simple: when we don't have usable memory we request more memory from the OS using the sbrk system call. Sbrk can increase or decrease the size of the calling process's data segment. In other words, it can be used as our heap. The data resides on the heap, but in contiguous chunks. A chunk is made up of two things: a contiguous segment called the meta block, and one byte after the meta block, the dynamically allocated memory. The meta blocks are connected, making up a linkedlist, and they contain info on the size and of the dynamically allocated memory and whether it has been freed. We reuse free blocks whenever we can.

## What I will do in future
My implementation uses a singly linkedlist. A very easy and clean approach, but it poses many problems when we want to use this implementation in a settings regarding large quantities of allocation. I would want to explore different ways I could structure my implementation and try implement already existing implementations. 


