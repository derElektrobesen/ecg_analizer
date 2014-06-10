#ifndef ALGO_EX_H
#define ALGO_EX_H

#include <Python.h>

#define raise_exception(__msg__) \
    raise_exception_n(__msg__, __FILE__, __LINE__)

int init_exception();
void del_exception();
void raise_exception_n(const char *msg, const char *file, int line);

#endif
