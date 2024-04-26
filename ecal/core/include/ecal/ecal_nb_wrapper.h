#include <ecal/ecal.h>
#include <stdint.h>
#include <string>
#include <functional>
#include <cstddef>
#include <mutex>
#include <ecal/ecal_types.h>

#include <nanobind/nanobind.h>
#include <nanobind/operators.h>
#include <nanobind/operators.h>
#include <nanobind/stl/string.h>
#include <nanobind/stl/shared_ptr.h>
#include <nanobind/stl/tuple.h>

namespace eCAL
{
    class CNBSubscriber: public CSubscriber
    {
    public:
        CNBSubscriber() : CSubscriber() { }
        CNBSubscriber(const std::string& s) : CSubscriber(s) { }
        CNBSubscriber(const std::string& s, const SDataTypeInformation& datainfo) : CSubscriber(s,datainfo) { }

        std::string Receive(int nb_timeout)
        {
            std::string Rec_Data = "No Reception";
            auto success = eCAL::CSubscriber::ReceiveBuffer(Rec_Data, nullptr, nb_timeout);
            return(Rec_Data);
        }
        bool WrapAddRecCB(nanobind::callable callback_)
        {
            assert(IsCreated());
            RemReceiveCallback();

            {
                std::lock_guard<std::mutex> callback_lock(m_python_callback_mutex);
                m_python_callback = callback_;
            }
            auto callback = std::bind(&CNBSubscriber::ReceiveCallback, this, std::placeholders::_1, std::placeholders::_2);
            return(CSubscriber::AddReceiveCallback(callback));
        }

    private:
        void ReceiveCallback(const char* topic_name_, const struct eCAL::SReceiveCallbackData* data_)
        {
            nanobind::callable fn_callback;
            {
                std::lock_guard<std::mutex> callback_lock(m_python_callback_mutex);
                fn_callback = m_python_callback;
            }

            try {
                nanobind::gil_scoped_acquire g2;
                fn_callback(nanobind::str(topic_name_), nanobind::bytes((const char*)data_->buf, data_->size), nanobind::int_(data_->time));
            }
            catch (const nanobind::python_error& e) {
                std::cout<<e.what();
            }
        }

        nanobind::callable m_python_callback;
        std::mutex m_python_callback_mutex;
    };

    class CNBSrvClient : public CServiceClient
    {
    public:
        CNBSrvClient() : CServiceClient() { }
        CNBSrvClient(const std::string& s) : CServiceClient(s) { }

        bool WrapAddRespCB(nanobind::callable callback_)
        {
            assert(IsConnected());
            {
                std::lock_guard<std::mutex> callback_lock(m_python_callback_mutex);
                m_python_callback = callback_;
            }
            auto callback = std::bind(&CNBSrvClient::ResponseCallback, this, std::placeholders::_1);
            return(CServiceClient::AddResponseCallback(callback));
        }

    private:
        void ResponseCallback(const struct SServiceResponse& data_)
        {
            nanobind::callable fn_callback;
            {
                std::lock_guard<std::mutex> callback_lock(m_python_callback_mutex);
                fn_callback = m_python_callback;
            }

            try {
                nanobind::gil_scoped_acquire g2;
                fn_callback(data_.service_name, data_.response);
            }
            catch (const nanobind::python_error& e) {
                std::cout << e.what();
            }
       }

        nanobind::callable m_python_callback;
        std::mutex m_python_callback_mutex;
    };

    class CNBSrvServer : public CServiceServer
    {
    public:
        CNBSrvServer() : CServiceServer() { }
        CNBSrvServer(const std::string& s) : CServiceServer(s) { }

        bool WrapAddMethodCB(const std::string& nb_method, const std::string& nb_req_type, const std::string& nb_resp_type, nanobind::callable callback_)
        {
            assert(IsConnected());
            {
                std::lock_guard<std::mutex> callback_lock(m_python_callback_mutex);
                m_python_callback = callback_;
            }
            auto Servercallback = std::bind(&CNBSrvServer::MethodCallback, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4, std::placeholders::_5);
            return(CNBSrvServer::AddMethodCallback(nb_method, nb_req_type, nb_resp_type, Servercallback));
        }

    private:
        void MethodCallback(const std::string& method_, const std::string& req_type_, const std::string& resp_type_, const std::string& request, std::string& response)
        {
            nanobind::callable fn_callback;
            {
                std::lock_guard<std::mutex> callback_lock(m_python_callback_mutex);
                fn_callback = m_python_callback;
            }

            try {
                nanobind::gil_scoped_acquire g2;
                fn_callback(method_, req_type_, resp_type_, request, response);
            }
            catch (const nanobind::python_error& e) {
                std::cout << e.what();
            }
        }

        nanobind::callable m_python_callback;
        std::mutex m_python_callback_mutex;
    };
}
