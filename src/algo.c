#include "algo.h"

#define st_arr_l(__arr__) \
    sizeof(__arr__) / sizeof(*__arr__)

#define EXP 10

#define USE_ALL_FILTERS
//#define USE_LO_FILTER
//#define USE_HI_FILTER

inline static PyObject *err_str(const char *msg) {
    raise_exception(msg);
    return NULL;
}

#if defined USE_ALL_FILTERS || defined USE_HI_FILTER
static void band_filters_impl_hi(const float *src, float *dst, Py_ssize_t size) {
    float x[] = { 0.0f, 0.0f };
    float y[] = { 0.0f, 0.0f };

    float coefs[5][5] = {
        /*
        { -6.057e-5, 1.973e-8, 4.909e-8, 8.132e-9, 1.227e-9 },
        { -6.057e-5, 3.946e-8, 9.818e-8, 1.626e-8, 2.455e-9 },
        { 0.0f,      1.973e-8, 4.909e-8, 8.132e-9, 1.227e-9 },
        { -1.0f,    -2.0f,    -2.0f,    -2.0f,    -2.0f     },
        { 0.0f,      1.0f,     1.0f,     1.0f,     1.0f     }
        */
        { 9.869e-8, 9.867e-8, 9.865e-8, 9.865e-8, 9.863e-8 },
        { 1.974e-7, 1.973e-7, 1.973e-7, 1.973e-7, 1.973e-7 },
        { 9.869e-8, 9.867e-8, 9.865e-8, 9.864e-8, 9.863e-8 },
        { -2.0f,    -2.0f,    -2.0f,    -2.0f,    -2.0f    },
        { 1.0f,     1.0f,     1.0f,     1.0f,     1.0f     }
    };

    Py_ssize_t i;
    int j;
    float yi, ry;

    for (j = 0; j < EXP / 2; j++) {
        for (i = 0; i < size; i++) {
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
#endif /* defined USE_ALL_FILTERS || defined USE_HI_FILTER */

#if defined USE_ALL_FILTERS || defined USE_LO_FILTER
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

    Py_ssize_t i;
    int j;
    float yi, ry;

    for (j = 0; j < EXP / 2; j++) {
        for (i = 0; i < size; i++) {
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
#endif /* defined USE_ALL_FILTERS || defined USE_LO_FILTER */

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
    if (!xvec || !yvec) {
        PyMem_RawFree(xvec ?: yvec);
        return err_str("Error allocating memory");
    }

    Py_ssize_t i = 0;
    for (; i < size; i++)
        PyArg_Parse(PyTuple_GET_ITEM(y, i), "f", xvec + i);

#if defined USE_ALL_FILTERS || defined USE_LO_FILTER
    band_filters_impl_lo(xvec, yvec, size);
#endif
#if defined USE_ALL_FILTERS
    band_filters_impl_hi(yvec, xvec, size);
#elif defined USE_HI_FILTER
    band_filters_impl_hi(xvec, yvec, size);
#endif

    result = PyTuple_New(2);
    if (result) {
        for (i = 0; i < size; i++) {
#ifndef USE_HI_FILTER
            PyTuple_SET_ITEM(ry, i, Py_BuildValue("f", xvec[i]));
#else
            PyTuple_SET_ITEM(ry, i, Py_BuildValue("f", yvec[i]));
#endif
        }
        PyTuple_SET_ITEM(result, 0, Py_BuildValue("O", x));
        PyTuple_SET_ITEM(result, 1, Py_BuildValue("O", ry));
    } else
        err_str("Error allocating memory");

    PyMem_RawFree(xvec);
    PyMem_RawFree(yvec);
    return result;
}
