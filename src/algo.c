#include "algo.h"

#define st_arr_l(__arr__) \
    sizeof(__arr__) / sizeof(*__arr__)

#define EXP 10
#define INTER_STEP 4

//#define USE_ALL_FILTERS
#define USE_LO_FILTER
//#define USE_HI_FILTER

inline static PyObject *err_str(const char *msg) {
    raise_exception(msg);
    return NULL;
}

static void differ(float *signal, Py_ssize_t size, float dx) {
    Py_ssize_t i = 1;
    float last_val = *signal;
    for (; i < size; i++) {
        float tmp = signal[i];
        signal[i] = (signal[i] - last_val) / dx;
        last_val = tmp;
    }
    *signal = signal[1];
}

static void sig_sqr(float *signal, Py_ssize_t size) {
    Py_ssize_t i = 0;
    for (; i < size; i++, signal++)
        *signal *= *signal;
}

static Py_ssize_t integrate(float *signal, float *x_coord, Py_ssize_t size, float dx) {
    Py_ssize_t i;
    const int step = INTER_STEP;
    int st = step;
    dx *= step;
    for (i = 0; i < size; i += step) {
        if (i + step > size)
            st = size - i;
        signal[i / step] = (signal[i] + signal[i + st]) * dx * 0.5f;
        x_coord[i / step] = x_coord[i] + dx * 0.5f;
    }
    return size / step + (i + step > size ? 1 : 0);
}

#if defined USE_ALL_FILTERS || defined USE_HI_FILTER
static void band_filters_impl_hi(const float *src, float *dst, Py_ssize_t size) {
    float x[] = { 0.0f, 0.0f };
    float y[] = { 0.0f, 0.0f };

    float coefs[5][5] = {
        { 9.705e-8f, 7.912e-8f, 5.012e-8f, 5.012e-8f, 3.187e-9f },
        { 1.941e-7f, 1.582e-7f, 1.002e-7f, 1.002e-7f, 6.373e-9f },
        { 9.705e-8f, 7.912e-8f, 5.012e-8f, 5.012e-8f, 3.187e-9f },
        { -2.0f, -2.0f, -2.0f, -2.0f, -2.0f },
        { 1.0f, 1.0f, 1.0f, 1.0f, 1.0f },
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

    float *xvec = (float *)PyMem_RawMalloc(sizeof(float) * size);
    float *yvec = (float *)PyMem_RawMalloc(sizeof(float) * size);
    if (!xvec || !yvec) {
        PyMem_RawFree(xvec ?: yvec);
        return err_str("Error allocating memory");
    }

    Py_ssize_t i = 0;
    for (; i < size; i++)
        PyArg_Parse(PyTuple_GET_ITEM(y, i), "f", xvec + i);

    float *ptr = yvec;

#if defined USE_ALL_FILTERS || defined USE_LO_FILTER
    band_filters_impl_lo(xvec, yvec, size);
#endif
#if defined USE_ALL_FILTERS
    band_filters_impl_hi(yvec, xvec, size);
#elif defined USE_HI_FILTER
    band_filters_impl_hi(xvec, yvec, size);
    ptr = xvec;
#endif

    float *x_ptr = xvec == ptr ? yvec : xvec;
    float dx = xvec[1] - xvec[0];

    for (i = 0; i < size; i++)
        PyArg_Parse(PyTuple_GET_ITEM(x, i), "f", x_ptr + i);

    differ(ptr, size, dx);
    differ(ptr, size, dx);
    sig_sqr(ptr, size);
    size = integrate(ptr, x_ptr, size, dx);

    _PyTuple_Resize(&x, size);
    ry = PyTuple_New(size);
    if (!ry)
        return err_str("Error allocating memory");

    result = PyTuple_New(2);
    if (result) {
        for (i = 0; i < size; i++) {
            PyTuple_SET_ITEM(ry, i, Py_BuildValue("f", ptr[i]));
            PyTuple_SET_ITEM(x, i, Py_BuildValue("f", x_ptr[i]));
        }
        PyTuple_SET_ITEM(result, 0, Py_BuildValue("O", x));
        PyTuple_SET_ITEM(result, 1, Py_BuildValue("O", ry));
    } else
        err_str("Error allocating memory");

    PyMem_RawFree(xvec);
    PyMem_RawFree(yvec);
    return result;
}
