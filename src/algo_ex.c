#include "algo_ex.h"

static PyObject *exception;

int init_exception(PyObject *parent) {
    exception = PyErr_NewException(LIB_NAME ".Exception", NULL, NULL);
    Py_INCREF(exception);
    PyModule_AddObject(parent, "exception", exception);
    return exception != NULL;
}

void del_exception() {
    Py_CLEAR(exception);
}

void raise_exception_n(const char *msg, const char *file, int line) {
    char str[1024];
    snprintf(str, sizeof(str), "Exception came from %s:%d: %s\n", file, line, msg);
    PyErr_SetString(exception, str);
}
