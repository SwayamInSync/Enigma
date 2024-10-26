// python/src/bindings.cpp
#include <pybind11/pybind11.h>
#include <pybind11/complex.h>
#include <pybind11/stl.h>
#include "Scalar.h"
#include "DEBUG.h"

namespace py = pybind11;
using namespace enigma;

// Helper function to convert Python numeric types to Scalar
Scalar py_to_scalar(const py::object &obj)
{
    if (py::isinstance<py::bool_>(obj))
    {
        return Scalar(obj.cast<bool>());
    }
    if (py::isinstance<py::int_>(obj))
    {
        return Scalar(obj.cast<int64_t>());
    }
    if (py::isinstance<py::float_>(obj))
    {
        return Scalar(obj.cast<double>());
    }
    if (PyComplex_Check(obj.ptr()))
    {
        std::complex<double> c(
            PyComplex_RealAsDouble(obj.ptr()),
            PyComplex_ImagAsDouble(obj.ptr()));
        return Scalar(c);
    }

    // Try to extract Scalar from Python object
    try
    {
        return obj.cast<Scalar>();
    }
    catch (const py::cast_error &)
    {
        throw py::type_error("Cannot convert Python object to Scalar");
    }
}

// Convert Scalar to appropriate Python type
py::object scalar_to_py(const Scalar &scalar)
{
    if (scalar.isBoolean())
    {
        return py::bool_(scalar.to<bool>());
    }
    if (scalar.isIntegral())
    {
        return py::int_(scalar.to<int64_t>());
    }
    if (scalar.isFloatingPoint())
    {
        return py::float_(scalar.to<double>());
    }
    if (scalar.isComplex())
    {
        auto c = scalar.to<std::complex<double>>();
        return py::reinterpret_steal<py::object>(
            PyComplex_FromDoubles(c.real(), c.imag()));
    }
    throw py::type_error("Unknown Scalar type");
}

PYBIND11_MODULE(_enigma, m)
{
    // Create the module
    m.doc() = "Python bindings for the Enigma tensor framework";

    // Register exception translations
    py::register_exception<ScalarError>(m, "ScalarError");
    py::register_exception<ScalarTypeError>(m, "ScalarTypeError");

    // Register ScalarType enum
    py::enum_<ScalarType>(m, "ScalarType")
        .value("int8", ScalarType::Int8)
        .value("int16", ScalarType::Int16)
        .value("int32", ScalarType::Int32)
        .value("int64", ScalarType::Int64)
        .value("uint8", ScalarType::UInt8)
        .value("uint16", ScalarType::UInt16)
        .value("uint32", ScalarType::UInt32)
        .value("uint64", ScalarType::UInt64)
        .value("float32", ScalarType::Float32)
        .value("float64", ScalarType::Float64)
        .value("complex64", ScalarType::Complex64)
        .value("complex128", ScalarType::Complex128)
        .value("bool", ScalarType::Bool)
        .export_values();

    // Register Scalar class
    py::class_<Scalar>(m, "Scalar")
        // Constructors
        .def(py::init<>())
        .def(py::init([](const py::object &value, py::object dtype)
                      {
            if (!dtype.is_none()) {
                auto scalar_type = dtype.cast<ScalarType>();
                // TODO: Implement explicit dtype conversion
                throw py::type_error("Explicit dtype not yet implemented");
            }
            return py_to_scalar(value); }),
             py::arg("value"), py::arg("dtype") = py::none())

        // Type checking methods
        .def_property_readonly("dtype", &Scalar::type)
        .def("is_floating_point", &Scalar::isFloatingPoint)
        .def("is_integral", &Scalar::isIntegral)
        .def("is_complex", &Scalar::isComplex)
        .def("is_bool", &Scalar::isBoolean)

        // Conversion methods
        .def("to_float", [](const Scalar &self)
             { return self.to<double>(); })
        .def("to_int", [](const Scalar &self)
             { return self.to<int64_t>(); })
        .def("to_bool", [](const Scalar &self)
             { return self.to<bool>(); })
        .def("to_complex", [](const Scalar &self)
             { return self.to<std::complex<double>>(); })

        // String representation
        .def("__str__", &Scalar::toString)
        .def("__repr__", [](const Scalar &self)
             { return "enigma.Scalar(" + self.toString() + ")"; })

        // Arithmetic operators
        .def("__add__", [](const Scalar &self, const py::object &other)
             {
            if (py::isinstance<Scalar>(other)) {
                return self + other.cast<Scalar>();
            }
            return self + py_to_scalar(other); })
        .def("__sub__", [](const Scalar &self, const py::object &other)
             {
            if (py::isinstance<Scalar>(other)) {
                return self - other.cast<Scalar>();
            }
            return self - py_to_scalar(other); })
        .def("__mul__", [](const Scalar &self, const py::object &other)
             {
            if (py::isinstance<Scalar>(other)) {
                return self * other.cast<Scalar>();
            }
            return self * py_to_scalar(other); })
        .def("__truediv__", [](const Scalar &self, const py::object &other)
             {
            if (py::isinstance<Scalar>(other)) {
                return self / other.cast<Scalar>();
            }
            return self / py_to_scalar(other); })
        .def("__neg__", [](const Scalar &self)
             { return -self; })

        // Reverse operators
        .def("__radd__", [](const Scalar &self, const py::object &other)
             { return py_to_scalar(other) + self; })
        .def("__rsub__", [](const Scalar &self, const py::object &other)
             { return py_to_scalar(other) - self; })
        .def("__rmul__", [](const Scalar &self, const py::object &other)
             { return py_to_scalar(other) * self; })
        .def("__rtruediv__", [](const Scalar &self, const py::object &other)
             { return py_to_scalar(other) / self; })

        // Comparison operators
        .def("__eq__", [](const Scalar &self, const py::object &other)
             {
            if (py::isinstance<Scalar>(other)) {
                return self == other.cast<Scalar>();
            }
            return self == py_to_scalar(other); })
        .def("__ne__", [](const Scalar &self, const py::object &other)
             {
            if (py::isinstance<Scalar>(other)) {
                return self != other.cast<Scalar>();
            }
            return self != py_to_scalar(other); });

    // Module-level functions
    m.def("get_dtype", [](const Scalar &scalar)
          { return scalar.type(); });
    m.def("promote_types", &Scalar::promoteTypes);
    m.def("can_cast", &Scalar::canCast);
}