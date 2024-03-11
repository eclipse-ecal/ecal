:orphan:

.. _compatibility_details_5_13_deprecations:

======================
eCAL 5.13 Deprecations
======================

All API functions (C & C++) deprecated that are no longer part of the
future eCAL6 API.

Deprecated header files
=======================

- :file:`/ecal/cimpl/ecal_event_cimpl.h`
- :file:`include/ecal/cimpl/ecal_proto_dyn_json_subscriber_cimpl.h`
- :file:`include/ecal/cimpl/ecal_qos_cimpl.h`
- :file:`include/ecal/ecal_clang.h`
- :file:`include/ecal/ecal_event.h`
- :file:`include/ecal/ecal_eventhandle.h`
- :file:`include/ecal/ecal_qos.h`
- :file:`include/ecal/ecal_timed_cb.h`

Deprecated API
==============

- :file:`ecal_callback_cimpl.h`:
  
  .. code-block:: cpp
  
     enum eCAL_Subscriber_Event
     {
       sub_event_none              = 0,
       sub_event_connected         = 1,
       sub_event_disconnected      = 2,
       sub_event_dropped           = 3,
       sub_event_timeout           = 4,  //!< deprecated, will be removed in future eCAL versions
       sub_event_corrupted         = 5,
       sub_event_update_connection = 6,
     };
  
     struct SPubEventCallbackDataC
     {
       enum eCAL_Publisher_Event  type;         //!< event type
       long long                  time;         //!< event time stamp
       long long                  clock;        //!< event clock
       const char*                tid;          //!< topic id of the connected subscriber                          (for pub_event_update_connection only)
       const char*                tname;        //!< topic type encoding of the connected subscriber               (for pub_event_update_connection only)
       const char*                tencoding;    //!< topic type name of the connected subscriber                   (for pub_event_update_connection only)
       const char*                tdesc;        //!< topic type descriptor information of the connected subscriber (for pub_event_update_connection only)
  
       const char*                ttype;        //!< deprecated, please use new tname + tencoding fields
     };
  
     struct SSubEventCallbackDataC
     {
       enum eCAL_Subscriber_Event type;         //!< event type
       long long                  time;         //!< event time stamp
       long long                  clock;        //!< event clock
       const char*                tid;          //!< topic id of the connected publisher                          (for sub_event_update_connection only)
       const char*                tname;        //!< topic type encoding of the connected publisher               (for sub_event_update_connection only)
       const char*                tencoding;    //!< topic type name of the connected publisher                   (for sub_event_update_connection only)
       const char*                tdesc;        //!< topic type descriptor information of the connected publisher (for sub_event_update_connection only)
  
       const char*                ttype;        //!< deprecated, please use new tname + tencoding fields
     };
  
- :file:`ecal_client_cimpl.h`:
  
  .. code-block:: cpp
  
     int eCAL_Client_AddResponseCallbackC(ECAL_HANDLE handle_, ResponseCallbackCT callback_, void* par_);
  
- :file:`ecal_init_cimpl.h`:
  
  .. code-block:: cpp
  
     #define eCAL_Init_Publisher   0x01                          /*!< Initialize Publisher API            */
     #define eCAL_Init_Subscriber  0x02                          /*!< Initialize Subscriber API           */
     #define eCAL_Init_Service     0x04                          /*!< Initialize Service API              */
     #define eCAL_Init_Monitoring  0x08                          /*!< Initialize Monitoring API           */
     #define eCAL_Init_Logging     0x10                          /*!< Initialize Logging API              */
     #define eCAL_Init_TimeSync    0x20                          /*!< Initialize Time API                 */
     #define eCAL_Init_RPC         0x40                          /*!< deprecated, will be removed in future eCAL versions */
     #define eCAL_Init_ProcessReg  0x80                          /*!< Initialize Process Registration API */
  
- :file:`ecal_log_cimpl.h`:
  
  .. code-block:: cpp
  
     void eCAL_Logging_StartCoreTimer();
     void eCAL_Logging_StopCoreTimer();
     void eCAL_Logging_SetCoreTime(double time_);
     double eCAL_Logging_GetCoreTime();
  
- :file:`ecal_process_cimpl.h`:
  
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
  
- :file:`ecal_publisher_cimpl.h`:
  
  .. code-block:: cpp
  
     int eCAL_Pub_SetQOS(ECAL_HANDLE handle_, struct SWriterQOSC qos_);
     int eCAL_Pub_GetQOS(ECAL_HANDLE handle_, struct SWriterQOSC* qos_);
     int eCAL_Pub_SetMaxBandwidthUDP(ECAL_HANDLE handle_, long bandwidth_);
  
- :file:`ecal_server_cimpl.h`:
  
  .. code-block:: cpp 
  
     int eCAL_Server_AddMethodCallbackC(ECAL_HANDLE handle_, const char* method_, const char* req_type_, const char* resp_type_, MethodCallbackCT callback_, void* par_);
     int eCAL_Server_RemMethodCallbackC(ECAL_HANDLE handle_, const char* method_);
  
- :file:`ecal_subscriber_cimpl.h`:
  
  .. code-block:: cpp
  
     int eCAL_Sub_SetQOS(ECAL_HANDLE handle_, struct SReaderQOSC qos_);
     int eCAL_Sub_GetQOS(ECAL_HANDLE handle_, struct SReaderQOSC* qos_);
     int eCAL_Sub_SetTimeout(ECAL_HANDLE handle_, int timeout_);
  
- :file:`ecal_tlayer_cimpl.h`:
  
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
  
- :file:`ecal_log.h`:
  
  .. code-block:: cpp
  
     void StartCoreTimer();
     void StopCoreTimer();
     void SetCoreTime(double time_);
     double GetCoreTime();
  
- :file:`ecal_monitoring.h`:
  
  .. code-block:: cpp
  
     int GetMonitoring(std::string& mon_);
  
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
  
     bool SetQOS(const QOS::SWriterQOS& qos_);
     QOS::SWriterQOS GetQOS();
     bool SetMaxBandwidthUDP(long bandwidth_);
  
- :file:`ecal_subscriber.h`:
  
  .. code-block:: cpp
  
     bool SetQOS(const QOS::SReaderQOS& qos_);
     QOS::SReaderQOS GetQOS();
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
       STLayer()
       {
         sm_udp_mc  = smode_none;
         sm_shm     = smode_none;
         sm_inproc  = smode_none;
         sm_tcp     = smode_none;
       }
       eSendMode sm_udp_mc;  //!< udp multicast
       eSendMode sm_shm;     //!< shared memory
       eSendMode sm_inproc;  //!< deprecated, whole layer will be removed in future eCAL versions */
       eSendMode sm_tcp;     //!< tcp
     };

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
