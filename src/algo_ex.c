#include "algo_ex.h"

static PyObject *exception;

// Модуль помогает кинуть исключение в питон в случае неверного ввода

int init_exception(PyObject *parent) {
    exception = PyErr_NewException(LIB_NAME ".Exception", NULL, NULL);
    Py_INCREF(exception);
    PyModule_AddObject(parent, "exception", exception);
    return exception != NULL;
}

void del_exception() {
    // Освободить выделенную исключениями память
    Py_CLEAR(exception);
}

void raise_exception_n(const char *msg, const char *file, int line) {
    // Послать исключение в питон
    char str[1024];
    snprintf(str, sizeof(str), "Exception came from %s:%d: %s\n", file, line, msg);
    PyErr_SetString(exception, str);
}
