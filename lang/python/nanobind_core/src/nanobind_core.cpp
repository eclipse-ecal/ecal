#include <nanobind/nanobind.h>
#include <nanobind/operators.h>
#include <ecal/ecal.h>
#include <nanobind/operators.h>
#include <nanobind/stl/string.h>
#include <nanobind/stl/shared_ptr.h>
#include <nanobind/stl/tuple.h>
#include <cstdint>
#include <chrono>
#include <memory>
#include <string>
#include <cstddef>
#include <ecal/ecal_types.h>

#include <modules/module_client.h>
#include <modules/module_core.h>
#include <modules/module_publisher.h>
#include <modules/module_server.h>
#include <modules/module_subscriber.h>
#include <modules/module_util.h>
#include <wrappers/client.h>
#include <wrappers/datatypeinfo.h>
#include <wrappers/publisher.h>
#include <wrappers/server.h>
#include <wrappers/subscriber.h>


//void AddSubscriberClassToModule(const nanobind::module_& module) {};

NB_MODULE(nanobind_core, m) {
    // Struct eCAL::SDataTypeInformation
    nanobind::class_<eCAL::CNBDataTypeInformation>(m, "DataTypeInformation")
        .def(nanobind::init<>())
        .def_rw("name", &eCAL::CNBDataTypeInformation::name)
        .def_rw("encoding", &eCAL::CNBDataTypeInformation::encoding)
        .def_rw("descriptor", &eCAL::CNBDataTypeInformation::descriptor);

    // Struct eCAL::SServiceResponse
    nanobind::class_<eCAL::SServiceResponse>(m, "ServiceResponse")
        .def(nanobind::init<>())
        .def_rw("host_name", &eCAL::SServiceResponse::host_name)
        .def_rw("service_name", &eCAL::SServiceResponse::service_name)
        .def_rw("service_id", &eCAL::SServiceResponse::service_id)
        .def_rw("method_name", &eCAL::SServiceResponse::method_name)
        .def_rw("error_msg", &eCAL::SServiceResponse::error_msg)
        .def_rw("ret_state", &eCAL::SServiceResponse::ret_state)
        .def_rw("call_state", &eCAL::SServiceResponse::call_state)
        .def_rw("response", &eCAL::SServiceResponse::response);

    AddSubscriberClassToModule(m);
    AddPublisherClassToModule(m);
    AddClientClassToModule(m);
    AddServerClassToModule(m);

    AddCoreFuncToModule(m);
    AddUtilFuncToModule(m);

}
