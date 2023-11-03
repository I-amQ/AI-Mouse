#include "vm.h"

void Pointer::move(long dx, long dy) {

    PyObject* pValue1 = PyLong_FromLong(dx);
    PyObject* pValue2 = PyLong_FromLong(dy);
    PyTuple_SetItem(pArgs, 0, pValue1);
    PyTuple_SetItem(pArgs, 1, pValue2);
    PyObject_CallObject(pFunc, pArgs);

}
