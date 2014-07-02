#include "algo.h"

#define st_arr_l(__arr__) \
    sizeof(__arr__) / sizeof(*__arr__)

#define EXP 10
#define INTER_STEP 15
#define PT_INTERVAL 1.0
#define FILTER_C_COUNT EXP / 2

inline static PyObject *err_str(const char *msg) {
    raise_exception(msg);
    return NULL;
}

inline static void differ(float *signal, Py_ssize_t size, float dx) {
    Py_ssize_t i = 1;
    float last_val = *signal;
    for (; i < size; i++) {
        float tmp = signal[i];
        signal[i] = (signal[i] - last_val) / dx;
        last_val = tmp;
    }
    *signal = signal[1];
}

inline static void sig_sqr(float *signal, Py_ssize_t size) {
    Py_ssize_t i = 0;
    for (; i < size; i++, signal++)
        *signal *= *signal;
}

inline static Py_ssize_t integrate(float *signal, float *x_coord, Py_ssize_t size, float dx) {
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

inline static int search_max(const float *start_ptr, const float *end_ptr) {
    float max = *start_ptr;
    int index = 0;
    int i = 0;
    while (++start_ptr < end_ptr) {
        if (max < *start_ptr) {
            max = *start_ptr;
            index = i + 1;
        }
        i++;
    }
    return index;
}

static Py_ssize_t search_r_r(const float *signal, PyObject *dest, Py_ssize_t size, int offset) {
    Py_ssize_t j = 0, i = 0;

    const float *e_ptr = signal + size;
    while (signal < e_ptr) {
        if (signal + offset > e_ptr)
            offset = (int)(e_ptr - signal);
        int l_max = search_max(signal, signal + offset);
        signal += offset;

        PyTuple_SET_ITEM(dest, j++, Py_BuildValue("i", l_max + i));
        i += offset;
    }
    return j;
}

static void band_filters_impl(const float *src, float *dst, Py_ssize_t size, float coefs[5][FILTER_C_COUNT]) {
    Py_ssize_t i;

    float x[] = { 0.0f, 0.0f };
    float y[] = { 0.0f, 0.0f };

    float yi, ry;
    int j;

    for (j = 0; j < FILTER_C_COUNT; j++) {
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

inline static PyObject *alloc_err() {
    return err_str("Error allocate memory");
}

PyObject *band_filter(PyObject *self, PyObject *args) {
    PyObject *data = NULL, *result = NULL;
    PyObject *x = NULL, *y = NULL, *ry = NULL, *maximums = NULL;
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
        return alloc_err();
    }

    Py_ssize_t i = 0;
    for (; i < size; i++)
        PyArg_Parse(PyTuple_GET_ITEM(y, i), "f", xvec + i);

#define HIG_FILTER 0
#define LOW_FILTER 1

    float filters[2][5][FILTER_C_COUNT] = {
        [HIG_FILTER] = {
            /* High frequency filter */
            { 9.869e-4, 9.867e-4, 9.865e-4, 9.864e-4, 9.863e-4 },
            { 1.974e-3, 1.973e-3, 1.973e-3, 1.973e-3, 1.973e-3 },
            { 9.869e-4, 9.867e-4, 9.865e-4, 9.864e-4, 9.863e-4 },
            {-2.0e4,    -2.0e4,    -2.0e4,    -2.0e4,   -2.0e4 },
            { 9.998e3,  9.994e3,  9.991e3,  9.989e3,   9.988e3 }
        }, 
        [LOW_FILTER] = {
            /* Low frequency filter */
            {  0.301f,  0.241f,  0.207f,  0.187f,  0.178f },
            {  0.601f,  0.483f,  0.413f,  0.374f,  0.356f },
            {  0.301f,  0.241f,  0.207f,  0.187f,  0.178f },
            { -0.538f, -0.432f, -0.370f, -0.335f, -0.319f },
            {  0.741f,  0.397f,  0.196f,  0.083f,  0.031f }
        }
    };

    band_filters_impl(xvec, yvec, size, filters[LOW_FILTER]);
    //band_filters_impl(yvec, xvec, size, filters[HIG_FILTER]);

    float *y_ptr = yvec;
    float *x_ptr = xvec;

    for (i = 0; i < size; i++)
        PyArg_Parse(PyTuple_GET_ITEM(x, i), "f", x_ptr + i);

    float dx = x_ptr[1] - x_ptr[0];

    differ(y_ptr, size, dx);
    differ(y_ptr, size, dx);
    sig_sqr(y_ptr, size);
    size = integrate(y_ptr, x_ptr, size, dx);

    maximums = PyTuple_New(size);
    if (!maximums)
        return alloc_err();

    int offset = 0;
    while (x_ptr[offset] - *x_ptr < PT_INTERVAL)
        offset++;

    Py_ssize_t count = search_r_r(y_ptr, maximums, size, offset);
    _PyTuple_Resize(&maximums, count);

    _PyTuple_Resize(&x, size);
    ry = PyTuple_New(size);
    if (!ry)
        return alloc_err();

    result = PyTuple_New(3);
    if (result) {
        for (i = 0; i < size - 1; i++) {
            PyTuple_SET_ITEM(ry, i, Py_BuildValue("f", y_ptr[i]));
            PyTuple_SET_ITEM(x, i, Py_BuildValue("f", x_ptr[i]));
        }
        PyTuple_SET_ITEM(ry, size - 1, Py_BuildValue("f", y_ptr[size - 2]));
        PyTuple_SET_ITEM(x, size - 1, Py_BuildValue("f", x_ptr[size - 2]));

        PyTuple_SET_ITEM(result, 0, Py_BuildValue("O", x));
        PyTuple_SET_ITEM(result, 1, Py_BuildValue("O", ry));
        PyTuple_SET_ITEM(result, 2, Py_BuildValue("O", maximums));
    } else
        err_str("Error allocating memory");

    PyMem_RawFree(xvec);
    PyMem_RawFree(yvec);
    return result;
}
