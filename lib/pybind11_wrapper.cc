// pybind11_wrapper.cpp
#include <pybind11/pybind11.h>
#include <funcs.h>
#include "ofdmtxrx.h"
#include<string>


namespace py = pybind11;
using namespace pybind11::literals;


PYBIND11_PLUGIN(pybind11_wrapper) {
    py::module m("pybind11_wrapper", "pybind11 liquid_usrp wrapper for python");
    m.def("add", &add, "A function which adds two numbers",
          "i"_a=1, "j"_a=2);
    py::class_<Pet>(m, "Pet")
        .def(py::init<const std::string &>())
        .def("setName", &Pet::setName)
        .def("getName", &Pet::getName);

    py::class_<ofdmtxrx>(m, "ofdmtxrx")
        .def(py::init<>())
        .def("set_tx_freq", &ofdmtxrx::set_tx_freq)
        .def("set_tx_rate", &ofdmtxrx::set_tx_rate);


    return m.ptr();
}
