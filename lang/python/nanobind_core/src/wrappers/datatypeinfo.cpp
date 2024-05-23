#include <ecal/ecal.h>
#include <stdint.h>
#include <string>
#include <functional>
#include <cstddef>
#include <mutex>
#include <ecal/ecal_types.h>

#include <nanobind/nanobind.h>
#include <nanobind/operators.h>
#include <nanobind/stl/string.h>
#include <nanobind/stl/shared_ptr.h>
#include <nanobind/stl/tuple.h>
#include <wrappers/datatypeinfo.h>

namespace eCAL
{
    SDataTypeInformation convert(const CNBDataTypeInformation& nb_info)
    {
        SDataTypeInformation info;
        info.name = nb_info.name;
        info.encoding = nb_info.encoding;
        info.descriptor = std::string(nb_info.descriptor.c_str(), nb_info.descriptor.size());
    }
}