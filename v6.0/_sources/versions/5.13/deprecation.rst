:orphan:

.. _compatibility_details_5_13_deprecations:

======================
eCAL 5.13 Deprecations
======================

All API functions (C & C++) deprecated that are no longer part of the
future eCAL6 API.

Deprecated header files
=======================

- :file:`cimpl/ecal_event_cimpl.h`
- :file:`cimpl/ecal_proto_dyn_json_subscriber_cimpl.h`
- :file:`cimpl/ecal_qos_cimpl.h`
- :file:`ecal_clang.h`
- :file:`ecal_event.h`
- :file:`ecal_eventhandle.h`
- :file:`ecal_qos.h`
- :file:`ecal_timed_cb.h`

Deprecated API
==============

- :file:`cimpl/ecal_callback_cimpl.h`:

  .. code-block:: cpp
    
     enum eCAL_Subscriber_Event
     {
       sub_event_none              = 0,
       sub_event_connected         = 1,
       sub_event_disconnected      = 2,
       sub_event_dropped           = 3,
       sub_event_timeout           = 4,         //!< deprecated, will be removed in future eCAL versions
       sub_event_corrupted         = 5,
       sub_event_update_connection = 6,
     };
     
     struct SPubEventCallbackDataC
     {
       enum eCAL_Publisher_Event  type;
       long long                  time;
       long long                  clock;
       const char*                tid;
       const char*                tname;
       const char*                tencoding;
       const char*                tdesc;
       
       const char*                ttype;        //!< deprecated, please use new tname + tencoding fields
     };
     
     struct SSubEventCallbackDataC
     {
       enum eCAL_Subscriber_Event type;
       long long                  time;
       long long                  clock;
       const char*                tid;
       const char*                tname;
       const char*                tencoding;
       const char*                tdesc;
       
       const char*                ttype;        //!< deprecated, please use new tname + tencoding fields
     };

- :file:`cimpl/ecal_client_cimpl.h`:

  .. code-block:: cpp
    
     int eCAL_Client_AddResponseCallbackC(ECAL_HANDLE handle_, ResponseCallbackCT callback_, void* par_);

- :file:`cimpl/ecal_init_cimpl.h`:

  .. code-block:: cpp
    
     #define eCAL_Init_Publisher   0x01
     #define eCAL_Init_Subscriber  0x02
     #define eCAL_Init_Service     0x04
     #define eCAL_Init_Monitoring  0x08
     #define eCAL_Init_Logging     0x10
     #define eCAL_Init_TimeSync    0x20
     #define eCAL_Init_RPC         0x40         /*!< deprecated, will be removed in future eCAL versions */
     #define eCAL_Init_ProcessReg  0x80

- :file:`cimpl/ecal_log_cimpl.h`:

  .. code-block:: cpp
    
     void eCAL_Logging_StartCoreTimer();
     void eCAL_Logging_StopCoreTimer();
     void eCAL_Logging_SetCoreTime(double time_);
     double eCAL_Logging_GetCoreTime();

- :file:`cimpl/ecal_monitoring_cimpl.h`:

  .. code-block:: cpp
    
     int eCAL_Monitoring_PubMonitoring(int state_, const char* name_);
     int eCAL_Monitoring_PubLogging(int state_, const char* name_);

- :file:`cimpl/ecal_process_cimpl.h`:

  .. code-block:: cpp
    
     int eCAL_Process_GetHostID();
     float eCAL_Process_GetProcessCpuUsage();
     unsigned long eCAL_Process_GetProcessMemory();
     
     long long eCAL_Process_GetSClock();
     long long eCAL_Process_GetSBytes();
     long long eCAL_Process_GetWClock();
     long long eCAL_Process_GetWBytes();
     long long eCAL_Process_GetRClock();
     long long eCAL_Process_GetRBytes();

- :file:`cimpl/ecal_publisher_cimpl.h`:

  .. code-block:: cpp
    
     int eCAL_Pub_SetQOS(ECAL_HANDLE handle_, struct SWriterQOSC qos_);
     int eCAL_Pub_GetQOS(ECAL_HANDLE handle_, struct SWriterQOSC* qos_);
     
     int eCAL_Pub_SetMaxBandwidthUDP(ECAL_HANDLE handle_, long bandwidth_);

- :file:`cimpl/ecal_server_cimpl.h`:

  .. code-block:: cpp 
    
     int eCAL_Server_AddMethodCallbackC(ECAL_HANDLE handle_, const char* method_, const char* req_type_, const char* resp_type_, MethodCallbackCT callback_, void* par_);
     int eCAL_Server_RemMethodCallbackC(ECAL_HANDLE handle_, const char* method_);

- :file:`cimpl/ecal_subscriber_cimpl.h`:

  .. code-block:: cpp
    
     int eCAL_Sub_SetQOS(ECAL_HANDLE handle_, struct SReaderQOSC qos_);
     int eCAL_Sub_GetQOS(ECAL_HANDLE handle_, struct SReaderQOSC* qos_);
     
     int eCAL_Sub_SetTimeout(ECAL_HANDLE handle_, int timeout_);

- :file:`cimpl/ecal_tlayer_cimpl.h`:

  .. code-block:: cpp
  
     enum eTransportLayerC
     {
       tlayer_none       = 0,
       tlayer_udp_mc     = 1,
       tlayer_shm        = 4,
       tlayer_tcp        = 5,
       tlayer_inproc     = 42,  /*!< deprecated, whole layer will be removed in future eCAL versions */
       tlayer_all        = 255
     };

- :file:`cimpl/ecal_util_cimpl.h`:
  
  .. code-block:: cpp
    
     int eCAL_Util_GetTypeName(const char* topic_name_, void* topic_type_, int topic_type_len_);
     int eCAL_Util_GetDescription(const char* topic_name_, void* topic_desc_, int topic_desc_len_);

- :file:`ecal_callback.h`:

  .. code-block:: cpp
    
     struct SPubEventCallbackData
     {
       eCAL_Publisher_Event type;
       long long            time;
       long long            clock;
       std::string          tid;
       std::string          ttype;       //!< deprecated, please use new tdatatype field
       std::string          tdesc;       //!< deprecated, please use new tdatatype field 
       
       SDataTypeInformation tdatatype;
     };
     
     struct SSubEventCallbackData
     {
       eCAL_Subscriber_Event type;
       long long             time;
       long long             clock;
       std::string           tid;
       std::string           ttype;       //!< deprecated, please use new tdatatype field
       std::string           tdesc;       //!< deprecated, please use new tdatatype field 
       
       SDataTypeInformation  tdatatype;
     };

- :file:`ecal_log.h`:

  .. code-block:: cpp
    
     void StartCoreTimer();
     void StopCoreTimer();
     void SetCoreTime(double time_);
     double GetCoreTime();

- :file:`ecal_monitoring.h`:

  .. code-block:: cpp
    
     int GetMonitoring(std::string& mon_);
     int PubMonitoring(bool state_, std::string name_ = "ecal.monitoring");
     int PubLogging(bool state_, std::string name_ = "ecal.logging");

- :file:`ecal_process.h`:

  .. code-block:: cpp
    
     int GetHostID();
     float GetProcessCpuUsage();
     unsigned long GetProcessMemory();
     
     long long GetWClock();
     long long GetWBytes();
     long long GetRClock();
     long long GetRBytes();

- :file:`ecal_publisher.h`:

  .. code-block:: cpp
    
     CPublisher(const std::string& topic_name_, const std::string& topic_type_, const std::string& topic_desc_ = "");
     bool Create(const std::string& topic_name_, const std::string& topic_type_, const std::string& topic_desc_ = "");
     
     bool SetTypeName(const std::string& topic_type_name_);
     bool SetDescription(const std::string& topic_desc_);
     
     bool SetQOS(const QOS::SWriterQOS& qos_);
     QOS::SWriterQOS GetQOS();
     
     bool SetMaxBandwidthUDP(long bandwidth_);
     
     std::string GetTypeName() const;
     std::string GetDescription() const;

- :file:`ecal_subscriber.h`:

  .. code-block:: cpp
    
     CSubscriber(const std::string& topic_name_, const std::string& topic_type_, const std::string& topic_desc_ = "");
     bool Create(const std::string& topic_name_, const std::string& topic_type_, const std::string& topic_desc_ = "");
     
     bool SetQOS(const QOS::SReaderQOS& qos_);
     QOS::SReaderQOS GetQOS();
     
     std::string GetTypeName() const;
     std::string GetDescription() const;
     
     bool SetTimeout(int timeout_);

- :file:`ecal_tlayer.h`:

  .. code-block:: cpp
  
     enum eTransportLayer
     {
       tlayer_none       = 0,
       tlayer_udp_mc     = 1,
       tlayer_shm        = 4,
       tlayer_tcp        = 5,
       tlayer_inproc     = 42,  //!< deprecated, whole layer will be removed in future eCAL versions
       tlayer_all        = 255
     };
     
     struct ECAL_API STLayer
     {
       eSendMode sm_udp_mc;
       eSendMode sm_shm;
       eSendMode sm_inproc;  //!< deprecated, whole layer will be removed in future eCAL versions */
       eSendMode sm_tcp;
     };

- :file:`ecal_util.h`:

  .. code-block:: cpp
    
     bool GetTopicTypeName(const std::string& topic_name_, std::string& topic_type_);
     std::string GetTopicTypeName(const std::string& topic_name_);
     
     bool GetTopicDescription(const std::string& topic_name_, std::string& topic_desc_);
     std::string GetTopicDescription(const std::string& topic_name_);
     std::string GetDescription(const std::string& topic_name_);

- :file:`msg/capnproto/dynamic.h`:

  .. code-block:: cpp
    
     std::string GetTypeName() const;

- :file:`msg/capnproto/subscriber.h`:

  .. code-block:: cpp
    
     std::string GetTypeName() const;

- :file:`msg/protobuf/publisher.h`:

  .. code-block:: cpp
    
     std::string GetTypeName() const;
     std::string GetDescription() const;

- :file:`msg/string/publisher.h`:

  .. code-block:: cpp
    
     CPublisher(const std::string& topic_name_, const std::string& topic_type_, const std::string& topic_desc_);

- :file:`msg/publisher.h`:

  .. code-block:: cpp
    
     CMsgPublisher(const std::string& topic_name_, const std::string& topic_type_, const std::string& topic_desc_ = "");
     bool Create(const std::string& topic_name_, const std::string& topic_type_ = "", const std::string& topic_desc_ = "");
     
     virtual std::string GetTypeName() const;
     virtual std::string GetDescription() const;

- :file:`msg/subscriber.h`:

  .. code-block:: cpp
  
     CMsgSubscriber(const std::string& topic_name_, const std::string& topic_type_ = "", const std::string& topic_desc_ = "");
     bool Create(const std::string& topic_name_, const std::string& topic_type_ = "", const std::string& topic_desc_ = "")
     
     virtual std::string GetTypeName() const;
     virtual std::string GetDescription() const;

Added API
=========

- :file:`ecal_subscriber_cimpl.h`:

  .. code-block:: cpp
    
     ECALC_API int eCAL_Sub_GetTypeName(ECAL_HANDLE handle_, void* buf_, int buf_len_);
     ECALC_API int eCAL_Sub_GetEncoding(ECAL_HANDLE handle_, void* buf_, int buf_len_);

- :file:`ecal_util_cimpl.h`:

  .. code-block:: cpp
    
     ECALC_API int eCAL_Util_GetTopicEncoding(const char* topic_name_, void* topic_encoding_, int topic_encoding_len_);
     ECALC_API int eCAL_Util_GetTopicDescription(const char* topic_name_, void* topic_desc_, int topic_desc_len_);
