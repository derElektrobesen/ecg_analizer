#include "algo.h"

#define _STR(__arg__) # __arg__
#define STR(__arg__) _SSTR(__arg__)

#define DEBUG \
    return err_str("Debug line: " STR(__LINE__))

#define t_set_f(__arr__, __index__, __val__) \
    PyTuple_SetItem(__arr__, __index__, Py_BuildValue("f", __val__))
#define t_set_t(__arr__, __index__, __val__) \
    PyTuple_SetItem(__arr__, __index__, Py_BuildValue("O", __val__))

#define st_arr_l(__arr__) \
    sizeof(__arr__) / sizeof(*__arr__)

inline static PyObject *err_str(const char *msg) {
    raise_exception(msg);
    return NULL;
}

static short band_filters_impl(PyObject *y_data, PyObject *y_dest, Py_ssize_t size) {
    double x_cache[13], y_cache[3] = { 0, 0, 0 };
    double x, y, ry;
    int x_cache_offset = 0;
    Py_ssize_t i;

    if (size < st_arr_l(x_cache))
        return 0;

#define scroll(__index__) ({ \
        x_cache[ __index__ + x_cache_offset >= st_arr_l(x_cache) \
                    ? __index__ + x_cache_offset - st_arr_l(x_cache) \
                    : __index__ + x_cache_offset ]; \
    })

    for (i = 0; i < size; i++) {
        PyArg_Parse(PyTuple_GetItem(x_data, i), "d", &x);
        PyArg_Parse(PyTuple_GetItem(y_data, i), "d", &y);

        if (i >= st_arr_l(x_cache) - st_arr_l(y_cache)) {
            y_cache[0] = y_cache[1];
            y_cache[1] = y_cache[2];
            y_cache[2] = y;
        }

        if (i < st_arr_l(x_cache)) {
            x_cache[i] = x;
            ry = y;
        } else {
            /* ry == 2 * y[i - 1] - y[i - 2] + 1/32 * (x[i] - 2 * x[i - 6] + x[i - 12] */
            ry = 2 * y_cache[1] - y_cache[0];
            ry += (x - 2 * scroll(6) + scroll(12)) / 32.0;
            if (++x_cache_offset > st_arr_l(x_cache))
                x_cache_offset -= st_arr_l(x_cache);
        }

        t_set_f(y_dest, i, ry);
    }

#undef scroll

    return 1;
}

PyObject *band_filter(PyObject *self, PyObject *args) {
    PyObject *data, *result = NULL;
    PyObject *x, *y, *ry;
    Py_ssize_t size;

    if (!PyArg_ParseTuple(args, "O!", &PyTuple_Type, &data))
        return err_str("Incorrect input");

    size = PyTuple_Size(data);
    if (!size)
        return err_str("Incorrect tuple given");
    if (size != 2)
        return err_str("Tuple must contain 2 tuples (x & y)");

    PyArg_Parse(PyTuple_GetItem(data, 0), "O!", &PyTuple_Type, &x);
    PyArg_Parse(PyTuple_GetItem(data, 1), "O!", &PyTuple_Type, &y);

    size = PyTuple_Size(x);
    if (!size)
        return err_str("Incorrect coordinates given (length == 0)");
    if (size != PyTuple_Size(y))
        return err_str("Incorrect coordinates given (x_len != y_len)");

    ry = PyTuple_New(size);
    if (!ry)
        return err_str("Error allocating memory");

    if (band_filters_impl(y, ry, size)) {
        result = PyTuple_New(2);
        if (!result)
            return err_str("Error allocating memory");
        t_set_t(result, 0, x);
        t_set_t(result, 1, ry);
    }
    return result;
}
