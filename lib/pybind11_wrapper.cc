// pybind11_wrapper.cpp
#include <pybind11/pybind11.h>
#include <funcs.h>
#include "ofdmtxrx.h"
#include<string>
#include <pybind11/stl.h>
#include <pybind11/functional.h>

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

    py::class_<eonofdmtxrx>(m, "eonofdmtxrx")
        .def(py::init<>())
        .def("set_tx_freq", &eonofdmtxrx::set_tx_freq)
        .def("set_tx_rate", &eonofdmtxrx::set_tx_rate);

    py::class_<ofdmtxrx>(m, "ofdmtxrx")
        .def(py::init<uint,uint,uint,python_callback_t>())
        .def("set_tx_freq", &ofdmtxrx::set_tx_freq)
        .def("set_tx_rate", &ofdmtxrx::set_tx_rate)
        .def("set_tx_gain_soft", &ofdmtxrx::set_tx_gain_soft)
        .def("set_tx_gain_uhd", &ofdmtxrx::set_tx_gain_uhd)
        .def("set_tx_antenna", &ofdmtxrx::set_tx_antenna)
        .def("reset_tx", &ofdmtxrx::reset_tx)
        .def("transmit_packet", &ofdmtxrx::transmit_packet)
        .def("transmit_symbol", &ofdmtxrx::transmit_symbol)
        .def("assemble_frame", &ofdmtxrx::assemble_frame)
        .def("write_symbol", &ofdmtxrx::write_symbol)
        .def("end_transmit_frame", &ofdmtxrx::end_transmit_frame)
        .def("set_rx_freq", &ofdmtxrx::set_rx_freq)
        .def("set_rx_rate", &ofdmtxrx::set_rx_rate)
        .def("set_rx_gain_uhd", &ofdmtxrx::set_rx_gain_uhd)
        .def("set_rx_antenna", &ofdmtxrx::set_rx_antenna)
        .def("reset_rx", &ofdmtxrx::reset_rx)
        .def("start_rx", &ofdmtxrx::start_rx)
        .def("stop_rx", &ofdmtxrx::stop_rx)
        .def("debug_enable", &ofdmtxrx::debug_enable)
        .def("debug_disable", &ofdmtxrx::debug_disable)
        ;


    return m.ptr();
}
