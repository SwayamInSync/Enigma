#define PY_SSIZE_T_CLEAN
#include <Python.h>
#include "tensor.h"

static PyObject *Tensor_new(PyTypeObject *type, PyObject *args, PyObject *kwds)
{
    Tensor *self;
    self = (Tensor *)type->tp_alloc(type, 0);
    return (PyObject *)self;
}

static int Tensor_init(PyObject *self, PyObject *args, PyObject *kwds)
{
    PyObject *shape_obj = NULL;
    PyObject *data_obj = NULL;

    if (!PyArg_ParseTuple(args, "O|O", &shape_obj, &data_obj))
        return -1;

    std::vector<long> shape;
    if (PyList_Check(shape_obj))
    {
        Py_ssize_t size = PyList_Size(shape_obj);
        for (Py_ssize_t i = 0; i < size; i++)
        {
            PyObject *item = PyList_GetItem(shape_obj, i);
            shape.push_back(PyLong_AsLong(item));
        }
    }
    else
    {
        PyErr_SetString(PyExc_TypeError, "shape must be a list");
        return -1;
    }

    if (data_obj == NULL)
    {
        new (self) Tensor(shape);
    }
    else if (PyList_Check(data_obj))
    {
        std::vector<float> data;
        Py_ssize_t size = PyList_Size(data_obj);
        for (Py_ssize_t i = 0; i < size; i++)
        {
            PyObject *item = PyList_GetItem(data_obj, i);
            data.push_back((float)PyFloat_AsDouble(item));
        }
        new (self) Tensor(shape, data);
    }
    else
    {
        PyErr_SetString(PyExc_TypeError, "data must be a list");
        return -1;
    }

    return 0;
}

static PyObject *Tensor_add(PyObject *self, PyObject *args)
{
    PyObject *other;
    if (!PyArg_ParseTuple(args, "O", &other))
        return NULL;

    Tensor *result = new Tensor(((Tensor *)self)->add(*(Tensor *)other));
    return (PyObject *)result;
}

static PyObject *Tensor_multiply(PyObject *self, PyObject *args)
{
    PyObject *other;
    if (!PyArg_ParseTuple(args, "O", &other))
        return NULL;

    Tensor *result = new Tensor(((Tensor *)self)->multiply(*(Tensor *)other));
    return (PyObject *)result;
}

static PyMethodDef Tensor_methods[] = {
    {"add", Tensor_add, METH_VARARGS, "Add two tensors"},
    {"multiply", Tensor_multiply, METH_VARARGS, "Multiply two tensors"},
    {NULL, NULL, 0, NULL} /* Sentinel */
};

static PyObject *Tensor_repr(PyObject *self)
{
    // Ensure that self is of type Tensor
    if (!PyObject_IsInstance(self, (PyObject *)&PyTensor_Type))
    {
        PyErr_SetString(PyExc_TypeError, "Expected a Tensor object");
        return NULL;
    }

    Tensor *tensor = (Tensor *)self;

    // Ensure tensor is not NULL
    if (tensor == NULL)
    {
        PyErr_SetString(PyExc_RuntimeError, "Tensor object is NULL");
        return NULL;
    }

    std::string repr = tensor->repr();

    // Check if repr() returned an empty string and handle appropriately
    if (repr.empty())
    {
        PyErr_SetString(PyExc_RuntimeError, "Tensor repr() returned an empty string");
        return NULL;
    }

    return PyUnicode_FromString(repr.c_str());
}

static PyObject *Tensor_str(PyObject *self)
{
    // Ensure that self is of type Tensor
    if (!PyObject_IsInstance(self, (PyObject *)&PyTensor_Type))
    {
        PyErr_SetString(PyExc_TypeError, "Expected a Tensor object");
        return NULL;
    }

    Tensor *tensor = (Tensor *)self;

    // Ensure tensor is not NULL
    if (tensor == NULL)
    {
        PyErr_SetString(PyExc_RuntimeError, "Tensor object is NULL");
        return NULL;
    }

    std::string str = tensor->str();

    // Check if str() returned an empty string and handle appropriately
    if (str.empty())
    {
        PyErr_SetString(PyExc_RuntimeError, "Tensor str() returned an empty string");
        return NULL;
    }

    return PyUnicode_FromString(str.c_str());
}

static PyTypeObject TensorType = {
    PyVarObject_HEAD_INIT(NULL, 0) "enigma.Tensor", /* tp_name */
    sizeof(Tensor),                                 /* tp_basicsize */
    0,                                              /* tp_itemsize */
    0,                                              /* tp_dealloc */
    0,                                              /* tp_vectorcall_offset */
    0,                                              /* tp_getattr */
    0,                                              /* tp_setattr */
    0,                                              /* tp_as_async */
    (reprfunc)Tensor_repr,                          /* tp_repr */
    0,                                              /* tp_as_number */
    0,                                              /* tp_as_sequence */
    0,                                              /* tp_as_mapping */
    0,                                              /* tp_hash  */
    0,                                              /* tp_call */
    (reprfunc)Tensor_str,                           /* tp_str */
    0,                                              /* tp_getattro */
    0,                                              /* tp_setattro */
    0,                                              /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,       /* tp_flags */
    "Tensor object",                                /* tp_doc */
    0,                                              /* tp_traverse */
    0,                                              /* tp_clear */
    0,                                              /* tp_richcompare */
    0,                                              /* tp_weaklistoffset */
    0,                                              /* tp_iter */
    0,                                              /* tp_iternext */
    Tensor_methods,                                 /* tp_methods */
    0,                                              /* tp_members */
    0,                                              /* tp_getset */
    0,                                              /* tp_base */
    0,                                              /* tp_dict */
    0,                                              /* tp_descr_get */
    0,                                              /* tp_descr_set */
    0,                                              /* tp_dictoffset */
    (initproc)Tensor_init,                          /* tp_init */
    0,                                              /* tp_alloc */
    Tensor_new,                                     /* tp_new */
};

static PyModuleDef enigmamodule = {
    PyModuleDef_HEAD_INIT,
    "enigma",                                         /* m_name */
    "Example module that creates an extension type.", /* m_doc */
    -1,                                               /* m_size */
    NULL,                                             /* m_methods */
    NULL,                                             /* m_slots */
    NULL,                                             /* m_traverse */
    NULL,                                             /* m_clear */
    NULL,                                             /* m_free */
};

PyMODINIT_FUNC
PyInit_enigma(void)
{
    PyObject *m;
    if (PyType_Ready(&TensorType) < 0)
        return NULL;

    m = PyModule_Create(&enigmamodule);
    if (m == NULL)
        return NULL;

    Py_INCREF(&TensorType);
    if (PyModule_AddObject(m, "Tensor", (PyObject *)&TensorType) < 0)
    {
        Py_DECREF(&TensorType);
        Py_DECREF(m);
        return NULL;
    }

    return m;
}