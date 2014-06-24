#include "algo.h"

#define _STR(__arg__) # __arg__
#define STR(__arg__) _SSTR(__arg__)

#define st_arr_l(__arr__) \
    sizeof(__arr__) / sizeof(*__arr__)

#define EXP 10

inline static PyObject *err_str(const char *msg) {
    raise_exception(msg);
    return NULL;
}

static void band_filters_impl_hi(const float *src, float *dst, Py_ssize_t size) {
    float x[] = { 0.0f, 0.0f };
    float y[] = { 0.0f, 0.0f };

    float coefs[5][5] = {
        { -6.057e-5, 1.973e-8, 4.909e-8, 8.132e-9, 1.227e-9 },
        { -6.057e-5, 3.946e-8, 9.818e-8, 1.626e-8, 2.455e-9 },
        { 0.0f,      1.973e-8, 4.909e-8, 8.132e-9, 1.227e-9 },
        { -1.0f,    -2.0f,    -2.0f,    -2.0f,    -2.0f     },
        { 0.0f,      1.0f,     1.0f,     1.0f,     1.0f     }
    };

    int iters = EXP / 2;

    Py_ssize_t i;
    int j;
    float yi;

    for (j = 0; j < iters; j++) {
        for (i = 0; i < size; i++) {
            float ry = 0;
            yi = j ? dst[i] : src[i];
            ry = coefs[0][j] * yi + coefs[1][j] * x[0] + coefs[2][j] * x[1] - coefs[3][j] * y[0] - coefs[4][j] * y[1];
            dst[i] = ry;

            x[1] = x[0];
            x[0] = yi;
            y[1] = y[0];
            y[0] = ry;
        }
    }
}

static void band_filters_impl_lo(const float *src, float *dst, Py_ssize_t size) {
    float x[] = { 0.0f, 0.0f };
    float y[] = { 0.0f, 0.0f };

    float coefs[5][5] = {
        {  0.301f,  0.241f,  0.207f,  0.187f,  0.178f },
        {  0.601f,  0.483f,  0.413f,  0.374f,  0.356f },
        {  0.301f,  0.241f,  0.207f,  0.187f,  0.178f },
        { -0.538f, -0.432f, -0.370f, -0.335f, -0.319f },
        {  0.741f,  0.397f,  0.196f,  0.083f,  0.031f }
    };

    int iters = EXP / 2;

    Py_ssize_t i;
    int j;
    float yi;

    for (j = 0; j < iters; j++) {
        for (i = 0; i < size; i++) {
            float ry = 0;
            yi = j ? dst[i] : src[i];
            ry = coefs[0][j] * yi + coefs[1][j] * x[0] + coefs[2][j] * x[1] - coefs[3][j] * y[0] - coefs[4][j] * y[1];
            dst[i] = ry;

            x[1] = x[0];
            x[0] = yi;
            y[1] = y[0];
            y[0] = ry;
        }
    }
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

    float *xvec = (float *)PyMem_RawMalloc(sizeof(float) * size);
    float *yvec = (float *)PyMem_RawMalloc(sizeof(float) * size);
    if (!xvec)
        return err_str("Error allocating memory");

    Py_ssize_t i = 0;
    for (; i < size; i++)
        PyArg_Parse(PyTuple_GET_ITEM(y, i), "f", xvec + i);

    band_filters_impl_lo(xvec, yvec, size);
    band_filters_impl_hi(yvec, xvec, size);

    result = PyTuple_New(2);
    if (result) {
        for (i = 0; i < size; i++)
            PyTuple_SET_ITEM(ry, i, Py_BuildValue("f", xvec[i]));
        PyTuple_SET_ITEM(result, 0, Py_BuildValue("O", x));
        PyTuple_SET_ITEM(result, 1, Py_BuildValue("O", ry));
    } else
        err_str("Error allocating memory");

    PyMem_RawFree(xvec);
    PyMem_RawFree(yvec);
    return result;
}
