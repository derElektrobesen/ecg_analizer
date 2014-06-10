#include <Python.h>
#include "algo.h"
#include "algo_ex.h"

static PyMethodDef module_methods[] = {
    { "band_filter", (PyCFunction)band_filter, METH_VARARGS },
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

static struct PyModuleDef moduledef = {
    PyModuleDef_HEAD_INIT,
    LIB_NAME,
    NULL,
    0,
    module_methods,
    NULL,
    module_traverse,
    module_clear,
    NULL
};

PyObject *PyInit_algo() {
    PyObject *module = PyModule_Create(&moduledef);

    if (module == NULL)
        return NULL;

    if (!init_exception(module)) {
        Py_DECREF(module);
        return NULL;
    }
    return module;
}
