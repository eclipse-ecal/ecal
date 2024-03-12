#include <nanobind/nanobind.h>

NB_MODULE(nanobind_core, m) {
    m.def("hello", []() { return "Hello world!"; });
}