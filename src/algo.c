#include "algo.h"

#define _STR(__arg__) # __arg__
#define STR(__arg__) _SSTR(__arg__)

#define DEBUG \
    return err_str("Debug line: " STR(__LINE__))

#define t_set_f(__arr__, __index__, __val__) \
    PyTuple_SetItem(__arr__, __index__, Py_BuildValue("f", __val__))

inline static PyObject *err_str(const char *msg) {
    raise_exception(msg);
    return NULL;
}

PyObject *band_filter(PyObject *self, PyObject *args) {
    PyObject *data, *result;
    Py_ssize_t size;
    double item;
    int i;

    if (!PyArg_ParseTuple(args, "O!", &PyTuple_Type, &data))
        return err_str("Incorrect input");

    size = PyTuple_Size(data);
    if (!size)
        return err_str("Incorrect tuple length");

    result = PyTuple_New(size);
    if (!result)
        return err_str("Error allocating memory");

    for (i = 0; i < size; i++) {
        if (!PyArg_Parse(PyTuple_GetItem(data, i), "d", &item))
            return err_str("Error parsing array");
        t_set_f(result, i, item * 2);
    }

    return result;
}
