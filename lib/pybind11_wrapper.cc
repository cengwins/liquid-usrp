// pybind11_wrapper.cpp
#include <pybind11/pybind11.h>
#include <funcs.h>
namespace py = pybind11;
using namespace pybind11::literals;


PYBIND11_PLUGIN(pybind11_wrapper) {
    py::module m("pybind11_wrapper", "pybind11 example plugin");
    m.def("add", &add, "A function which adds two numbers",
          "i"_a=1, "j"_a=2);
    return m.ptr();
}
