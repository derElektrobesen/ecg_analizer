#include "algo.h"

#define st_arr_l(__arr__) \
    sizeof(__arr__) / sizeof(*__arr__)

#define EXP 10
#define INTER_STEP 4

static PyObject *err_str(const char *msg) {
    // Бросает исключение в питон
    raise_exception(msg);
    return NULL;
}

// Дифференциация
static void differ(float *signal, Py_ssize_t size, float dx) {
    Py_ssize_t i = 1;
    float last_val = *signal;
    for (; i < size; i++) {
        float tmp = signal[i];
        signal[i] = (signal[i] - last_val) / dx; // tan(alpha)
        last_val = tmp;
    }
    *signal = signal[1];
}

// Возведение переданных значений в квадрат
static void sig_sqr(float *signal, Py_ssize_t size) {
    Py_ssize_t i = 0;
    for (; i < size; i++, signal++)
        *signal *= *signal;
}

// Алгоритм скользящего окна
void moving_window(const float *signal, float *out, Py_ssize_t size) {
    const int N = 38;
    Py_ssize_t i = N;
    int j = 0;

    for (; i < size; i++) {
        out[i] = 0.0f;
        for (j = 0; j < N; j++) out[i] += signal[i - N + j];
        out[i] /= (float)N;
    }
}

// Применение высокочастотного фильтра
static void band_filters_impl_hi(const float *src, float *dst, Py_ssize_t size) {
    Py_ssize_t i = 1;
    *dst = *src;
    for (; i < size; i++) {
        dst[i] = dst[i - 1] - src[i] / 32;
        if (i > 15)
            dst[i] += src[i - 16];
        if (i > 16)
            dst[i] -= src[i - 17];
        if (i > 31)
            dst[i] += src[i - 32] / 32;
    }
}

// Применение низкочастотного фильтра
static void band_filters_impl_lo(const float *src, float *dst, Py_ssize_t size) {
    Py_ssize_t i = 0;
    for (; i < size; i++) {
        if (i < 2)
            dst[i] = src[i];
        else {
            dst[i] = 2 * dst[i - 1] - dst[i - 2] + src[i] / 32;
            if (i > 5)
                dst[i] -= src[i - 6] / 16;
            if (i > 11)
                dst[i] += src[i - 12] / 32;
        }
    }
}

// Вызывается ф-ия напрямую из питона
PyObject *band_filter(PyObject *self, PyObject *args) {
    PyObject *data, *result = NULL;
    PyObject *x, *y, *ry;
    Py_ssize_t size;

    if (!PyArg_ParseTuple(args, "O!", &PyTuple_Type, &data)) // Проверка ввода
        return err_str("Incorrect input");

    size = PyTuple_Size(data); // Проверка ввода
    if (!size)
        return err_str("Incorrect tuple given");
    if (size != 2)
        return err_str("Tuple must contain 2 tuples (x & y)");

    PyArg_Parse(PyTuple_GetItem(data, 0), "O!", &PyTuple_Type, &x); // Вытаскивание значений по х, у из питона
    PyArg_Parse(PyTuple_GetItem(data, 1), "O!", &PyTuple_Type, &y);

    size = PyTuple_Size(x);
    if (!size) // Проверка размеров массивов данных
        return err_str("Incorrect coordinates given (length == 0)");
    if (size != PyTuple_Size(y))
        return err_str("Incorrect coordinates given (x_len != y_len)");

    float *xvec = (float *)PyMem_RawMalloc(sizeof(float) * size); // Выделение памяти для локальных буфферных переменных
    float *yvec = (float *)PyMem_RawMalloc(sizeof(float) * size);
    if (!xvec || !yvec) {
        PyMem_RawFree(xvec ?: yvec); // Ошибка при выделении памяти
        return err_str("Error allocating memory");
    }

    Py_ssize_t i = 0;
    for (; i < size; i++)
        PyArg_Parse(PyTuple_GET_ITEM(y, i), "f", xvec + i); // Копирование входного сигнала в локальные переменные

    float *ptr = yvec;

    band_filters_impl_lo(xvec, yvec, size); // Применение низкочастотного фильтра
    band_filters_impl_hi(yvec, xvec, size); // Примененеи высокочастотного фильтра

    float *x_ptr = xvec == ptr ? yvec : xvec;
    for (i = 0; i < size; i++) // Копирование временного массива из питона
        PyArg_Parse(PyTuple_GET_ITEM(x, i), "f", x_ptr + i);

    float dx = x_ptr[1] - x_ptr[0];

    differ(ptr, size, dx); // дифференцирование
    sig_sqr(ptr, size); // Возведение в квадрат
    moving_window(ptr, x_ptr, size); // ПРименение алгоритма скользящего окна
    memcpy(ptr, x_ptr, size * sizeof(*ptr)); // Перемещение значений в другую буфферную переменную
    for (i = 0; i < size; i++) // Перемещение значений времени из питона в массив
        PyArg_Parse(PyTuple_GET_ITEM(x, i), "f", x_ptr + i);

    _PyTuple_Resize(&x, size);
    ry = PyTuple_New(size);
    if (!ry)
        return err_str("Error allocating memory");

    // Подготовка переменных для вывода обратно в питон
    result = PyTuple_New(2);
    if (result) {
        for (i = 0; i < size; i++) {
            PyTuple_SET_ITEM(ry, i, Py_BuildValue("f", ptr[i])); // Копируем в питон массив сигнала
            PyTuple_SET_ITEM(x, i, Py_BuildValue("f", x_ptr[i])); // время
        }
        PyTuple_SET_ITEM(result, 0, Py_BuildValue("O", x)); // [ [ x_data ... ], [ y_data ... ] ]
        PyTuple_SET_ITEM(result, 1, Py_BuildValue("O", ry));
    } else
        err_str("Error allocating memory");

    // Освобождение памяти
    PyMem_RawFree(xvec);
    PyMem_RawFree(yvec);
    return result;
}
