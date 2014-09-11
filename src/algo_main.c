#include <Python.h>
#include "algo.h"
#include "algo_ex.h"

// Модуль инициализации С библиотеки при старте программы

static PyMethodDef module_methods[] = {
    { "band_filter", (PyCFunction)band_filter, METH_VARARGS }, // Определение методов, доступных из питона
    { NULL, NULL }
};

static int module_traverse(PyObject *m, visitproc visit, void *arg) {
    /* Memory allocations */
    return 0;
}

static int module_clear(PyObject *m) {
    /* Memory clear */
    del_exception();
    return 0;
}

static struct PyModuleDef moduledef = { // Структура характеризует модуль С для питона (****)
    PyModuleDef_HEAD_INIT,
    LIB_NAME, // Имя модуля
    NULL,
    0,
    module_methods, // Методы модуля
    NULL,
    module_traverse, // Ф-ия инициализации (ничего не делает)
    module_clear, // Функция деинициализации
    NULL
};

PyObject *PyInit_algo() {
    // Ф-ия вызывается при старте программы

    PyObject *module = PyModule_Create(&moduledef); // Инициализация параметров модуля см (****)

    // Обработка ошибок инициализации
    if (module == NULL)
        return NULL;

    if (!init_exception(module)) {
        Py_DECREF(module);
        return NULL;
    }
    return module;
}
