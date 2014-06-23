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

short band_filters_test(PyObject *y_data, PyObject *y_dest, Py_ssize_t size) {
    float x[12];
    float y[3];
    bzero(y, sizeof(y));
    bzero(x, sizeof(x));

    Py_ssize_t i = 0;
    int j;
    float ry;
    float yi;

    for (; i < size; i++) {
        PyArg_Parse(PyTuple_GET_ITEM(y_data, i), "f", &yi);
        if (i < st_arr_l(x))
            ry = 0;
        else {
            ry = 2 * y[1] - y[0] + (ry - 2 * x[5] + x[11]) / 32.0f;

            for (j = 0; j < st_arr_l(x) - 1; j++)
                x[j] = x[j + 1];
            x[11] = yi;

            for (j = 0; j < st_arr_l(y) - 1; j++)
                y[j] = y[j + 1];
            y[2] = ry;
        }
        PyTuple_SET_ITEM(y_dest, i, Py_BuildValue("f", ry));
    }

    return 1;
}

short band_filters_impl_hi(PyObject *y_data, PyObject *y_dest, Py_ssize_t size) {
/*
 * PyArg_Parse(PyTuple_GetItem(x_data, i), "d", &x);
 * PyArg_Parse(PyTuple_GetItem(y_data, i), "d", &y)
 */
    float x[] = { 0.0f, 0.0f, 0.0f };
    float y[] = { 0.0f, 0.0f, 0.0f };

    float coefs[5][5] = {
        { -6.057e-5, 1.973e-8, 4.909e-8, 8.132e-9, 1.227e-9 },
        { -6.057e-5, 3.946e-8, 9.818e-8, 1.626e-8, 2.455e-9 },
        { 0.0f,      1.973e-8, 4.909e-8, 8.132e-9, 1.227e-9 },
        { -1.0f,    -2.0f,    -2.0f,    -2.0f,    -2.0f     },
        { 0.0f,      1.0f,     1.0f,     1.0f,     1.0f     }
    };

    //int exp = 10;
    //int iters = exp / 2;
    int iters = 2;

    Py_ssize_t i;
    int j;
    float yi;

    double *xvec = (double *)PyMem_RawMalloc(sizeof(double) * size);
    if (!xvec)
        return 0;

    for (j = 0; j < iters; j++) {
        for (i = 0; i < size; i++) {
            double ry = 0;

            if (!j)
                PyArg_Parse(PyTuple_GET_ITEM(y_data, i), "f", &yi);
            else
                yi = xvec[i];

            if (i > 2)
                ry = coefs[0][j] * yi + coefs[1][j] * x[1] + coefs[2][j] * x[2] - coefs[3][j] * y[1] - coefs[4][j] * y[2];

            if (j == iters - 1)
                PyTuple_SET_ITEM(y_dest, i, Py_BuildValue("f", ry));
            else
                xvec[i] = ry;

            x[2] = x[1];
            x[1] = x[0];
            x[0] = yi;
            y[2] = y[1];
            y[1] = y[0];
            y[0] = ry;
        }
    }

    PyMem_RawFree(xvec);

    return 1;
}

short band_filters_impl_lo(PyObject *y_data, PyObject *y_dest, Py_ssize_t size) {
/*
 * PyArg_Parse(PyTuple_GetItem(x_data, i), "d", &x);
 * PyArg_Parse(PyTuple_GetItem(y_data, i), "d", &y)
 */
    float x[] = { 0.0f, 0.0f, 0.0f };
    float y[] = { 0.0f, 0.0f, 0.0f };

    float coefs[5][5] = {
        {  0.301f,  0.241f,  0.207f,  0.187f,  0.178f },
        {  0.601f,  0.483f,  0.413f,  0.374f,  0.356f },
        {  0.301f,  0.241f,  0.207f,  0.187f,  0.178f },
        { -0.538f, -0.432f, -0.370f, -0.335f, -0.319f },
        {  0.741f,  0.397f,  0.196f,  0.083f,  0.031f }
    };

    //int exp = 10;
    //int iters = exp / 2;
    int iters = 1;

    Py_ssize_t i;
    int j;
    float yi;

    double *xvec = (double *)PyMem_RawMalloc(sizeof(double) * size);
    if (!xvec)
        return 0;

    for (j = 0; j < iters; j++) {
        for (i = 0; i < size; i++) {
            double ry = 0;

            if (!j)
                PyArg_Parse(PyTuple_GET_ITEM(y_data, i), "f", &yi);
            else
                yi = xvec[i];

            if (i > 2)
                ry = coefs[0][j] * yi + coefs[1][j] * x[1] + coefs[2][j] * x[2] - coefs[3][j] * y[1] - coefs[4][j] * y[2];

            if (j == iters - 1)
                PyTuple_SET_ITEM(y_dest, i, Py_BuildValue("f", ry));
            else
                xvec[i] = ry;

            x[2] = x[1];
            x[1] = x[0];
            x[0] = yi;
            y[2] = y[1];
            y[1] = y[0];
            y[0] = ry;
        }
    }

    PyMem_RawFree(xvec);

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

    if (band_filters_test(y, ry, size)) {
        result = PyTuple_New(2);
        if (!result)
            return err_str("Error allocating memory");
        t_set_t(result, 0, x);
        t_set_t(result, 1, ry);
    }
    return result;
}
