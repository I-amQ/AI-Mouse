#pragma once
#include <python.h>

class Pointer {

private:
    PyObject* pFunc;
    PyObject* pArgs;

public:

	Pointer() {
        PyObject* pModule;
        Py_Initialize();
        PyObject* sys = PyImport_ImportModule("sys");
        PyObject* path = PyObject_GetAttrString(sys, "path");
        PyList_Append(path, PyUnicode_FromString("."));
        pModule = PyImport_ImportModule("module");
        this->pFunc = PyObject_GetAttrString(pModule, "move");
        this->pArgs = PyTuple_New(2);
	}

    void move(long dx, long dy);

};