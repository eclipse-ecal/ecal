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
    class CNBDataTypeInformation
    {
    public:
        std::string name;          
        std::string encoding;      
        nanobind::bytes descriptor;
    };

    inline SDataTypeInformation convert(const CNBDataTypeInformation& nb_info)
    {
        SDataTypeInformation info;
        info.name = nb_info.name;
        info.encoding = nb_info.encoding;
        info.descriptor = std::string(nb_info.descriptor.c_str(), nb_info.descriptor.size());
    }
    
    class CNBSubscriber: public CSubscriber
    {
    public:
        CNBSubscriber() : CSubscriber() { }
        CNBSubscriber(const std::string& s) : CSubscriber(s) { }
        CNBSubscriber(const std::string& s, const CNBDataTypeInformation& datainfo) : CSubscriber(s, convert(datainfo)) { }

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
                std::lock_guard<std::mutex> callback_lock(m_python_rec_callback_mutex);
                m_python_rec_callback = callback_;
            }
            auto callback = std::bind(&CNBSubscriber::ReceiveCallback, this, std::placeholders::_1, std::placeholders::_2);
            return(CSubscriber::AddReceiveCallback(callback));
        }
        bool WrapAddSubEventCB(eCAL_Subscriber_Event event, nanobind::callable callback_)
        {
            assert(IsCreated());
            RemEventCallback(event);
            {
                std::lock_guard<std::mutex> callback_lock(m_python_event_callback_mutex);
                m_python_event_callback = callback_;
            }
            auto callback = std::bind(&CNBSubscriber::AddSubEventCB, this, std::placeholders::_1, std::placeholders::_2);
            return(CSubscriber::AddEventCallback(event, callback));
        }

    private:
        void ReceiveCallback(const char* topic_name_, const struct eCAL::SReceiveCallbackData* data_)
        {
            nanobind::callable fn_callback;
            {
                std::lock_guard<std::mutex> callback_lock(m_python_rec_callback_mutex);
                fn_callback = m_python_rec_callback;
            }

            try {
                nanobind::gil_scoped_acquire g2;
                fn_callback(nanobind::str(topic_name_), nanobind::bytes((const char*)data_->buf, data_->size), nanobind::int_(data_->time));
            }
            catch (const nanobind::python_error& e) {
                std::cout<<e.what();
            }
        }

        void AddSubEventCB(const char* event_name_, const struct eCAL::SSubEventCallbackData* data_)
        {
            nanobind::callable fn_callback;
            {
                std::lock_guard<std::mutex> callback_lock(m_python_event_callback_mutex);
                fn_callback = m_python_event_callback;
            }

            try {
                nanobind::gil_scoped_acquire g2;
                fn_callback(event_name_, data_->type, nanobind::int_(data_->time), data_->tid);
            }
            catch (const nanobind::python_error& e) {
                std::cout << e.what();
            }
        }
        nanobind::callable m_python_event_callback;
        std::mutex m_python_event_callback_mutex;

        nanobind::callable m_python_rec_callback;
        std::mutex m_python_rec_callback_mutex;
    };

    class CNBPublisher : public CPublisher
    {
    public:
        CNBPublisher() : CPublisher() { }
        CNBPublisher(const std::string& s) : CPublisher(s) { }
        CNBPublisher(const std::string& s, const CNBDataTypeInformation& datainfo) : CPublisher(s, convert(datainfo)) { }

        bool WrapAddPubEventCB(eCAL_Publisher_Event event, nanobind::callable callback_)
        {
            assert(IsCreated());
            RemEventCallback(event);
            {
                std::lock_guard<std::mutex> callback_lock(m_python_event_callback_mutex);
                m_python_event_callback = callback_;
            }
            auto callback = std::bind(&CNBPublisher::AddPubEventCB, this, std::placeholders::_1, std::placeholders::_2);
            return(CPublisher::AddEventCallback(event, callback));
        }

    private:
        void AddPubEventCB(const char* event_name_, const struct eCAL::SPubEventCallbackData* data_)
        {
            nanobind::callable fn_callback;
            {
                std::lock_guard<std::mutex> callback_lock(m_python_event_callback_mutex);
                fn_callback = m_python_event_callback;
            }

            try {
                nanobind::gil_scoped_acquire g2;
                fn_callback(event_name_, data_->type, nanobind::int_(data_->time), data_->tid);
            }
            catch (const nanobind::python_error& e) {
                std::cout << e.what();
            }
        }
        nanobind::callable m_python_event_callback;
        std::mutex m_python_event_callback_mutex;

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
                std::lock_guard<std::mutex> callback_lock(m_python_resp_callback_mutex);
                m_python_resp_callback = callback_;
            }
            auto callback = std::bind(&CNBSrvClient::ResponseCallback, this, std::placeholders::_1);
            return(CServiceClient::AddResponseCallback(callback));
        }
        bool WrapAddCltEventCB(eCAL_Client_Event type, nanobind::callable callback_)
        {
            assert(IsConnected());
            {
                std::lock_guard<std::mutex> callback_lock(m_python_event_callback_mutex);
                m_python_event_callback = callback_;
            }
            auto callback = std::bind(&CNBSrvClient::AddCltEventCB, this, std::placeholders::_1, std::placeholders::_2);
            return(CNBSrvClient::AddEventCallback(type, callback));
        }

    private:
        void ResponseCallback(const struct SServiceResponse& data_)
        {
            nanobind::callable fn_callback;
            {
                std::lock_guard<std::mutex> callback_lock(m_python_resp_callback_mutex);
                fn_callback = m_python_resp_callback;
            }

            try {
                nanobind::gil_scoped_acquire g2;
                fn_callback(data_.service_name, data_.response);
            }
            catch (const nanobind::python_error& e) {
                std::cout << e.what();
            }
        }
        void AddCltEventCB(const char* client_name_, const struct SClientEventCallbackData* data_)
        {
            nanobind::callable fn_callback;
            {
                std::lock_guard<std::mutex> callback_lock(m_python_event_callback_mutex);
                fn_callback = m_python_event_callback;
            }

            try {
                nanobind::gil_scoped_acquire g2;
                fn_callback(client_name_, data_->type, nanobind::int_(data_->time), data_->attr.hname);
            }
            catch (const nanobind::python_error& e) {
                std::cout << e.what();
            }
        }

        nanobind::callable m_python_event_callback;
        std::mutex m_python_event_callback_mutex;

        nanobind::callable m_python_resp_callback;
        std::mutex m_python_resp_callback_mutex;
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
                std::lock_guard<std::mutex> callback_lock(m_python_method_callback_mutex);
                m_python_method_callback = callback_;
            }
            auto Servercallback = std::bind(&CNBSrvServer::MethodCallback, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4, std::placeholders::_5);
            return(CNBSrvServer::AddMethodCallback(nb_method, nb_req_type, nb_resp_type, Servercallback));
        }
        bool WrapAddSrvEventCB(eCAL_Server_Event type, nanobind::callable callback_)
        {
            assert(IsConnected());
            {
                std::lock_guard<std::mutex> callback_lock(m_python_event_callback_mutex);
                m_python_event_callback = callback_;
            }
            auto callback = std::bind(&CNBSrvServer::AddSrvEventCB, this, std::placeholders::_1, std::placeholders::_2);
            return(CNBSrvServer::AddEventCallback(type, callback));
        }

    private:
        int MethodCallback(const std::string& method_, const std::string& req_type_, const std::string& resp_type_, const std::string& request, std::string& response)
        {
            nanobind::callable fn_callback;
            {
                std::lock_guard<std::mutex> callback_lock(m_python_method_callback_mutex);
                fn_callback = m_python_method_callback;
            }

            try {
                nanobind::gil_scoped_acquire g2;
                auto result = fn_callback(method_, req_type_, resp_type_, request);
                // do some check if object holds a tuple if (!result.is_type<nanobind::tuple>)
                nanobind::tuple result_tuple = nanobind::cast<nanobind::tuple>(result);
                response = nanobind::cast<std::string>(result[1]);
                int nb_int = nanobind::cast<int>(result[0]);
                return nb_int;
            }
            catch (const nanobind::python_error& e) {
                std::cout << e.what();
            }
        }
        void AddSrvEventCB(const char* client_name_, const struct SServerEventCallbackData* data_)
        {
            nanobind::callable fn_callback;
            {
                std::lock_guard<std::mutex> callback_lock(m_python_event_callback_mutex);
                fn_callback = m_python_event_callback;
            }

            try {
                nanobind::gil_scoped_acquire g2;
                fn_callback(client_name_, data_->type, nanobind::int_(data_->time));
            }
            catch (const nanobind::python_error& e) {
                std::cout << e.what();
            }
        }

        nanobind::callable m_python_event_callback;
        std::mutex m_python_event_callback_mutex;

        nanobind::callable m_python_method_callback;
        std::mutex m_python_method_callback_mutex;
    };
}
