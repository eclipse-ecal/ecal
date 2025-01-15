
.. _program_listing_file_ecal_core_include_ecal_ecal_clang.h:

Program Listing for File ecal_clang.h
=====================================

|exhale_lsh| :ref:`Return to documentation for file <file_ecal_core_include_ecal_ecal_clang.h>` (``ecal/core/include/ecal/ecal_clang.h``)

.. |exhale_lsh| unicode:: U+021B0 .. UPWARDS ARROW WITH TIP LEFTWARDS

.. code-block:: cpp

   const char* rcv_buf     = nullptr;
   int         rcv_buf_len = 0;
   long long   rcv_time    = 0;
   int         timeout     = 0;
   int ret = sub_receive(subscriber_handle, &rcv_buf, &rcv_buf_len, &rcv_time, timeout);
   if(rcv_buf_len > 0)
   {
     ...
     // PROCESS THE BUFFER CONTENT HERE
     ...
     // finally free the allocated memory
     ecal_free_mem((void*)rcv_buf);
   }
   const char* mon_buf_     = NULL;
   int         mon_buf_len_ = 0;
   mon_get_monitoring(subscriber_handle, &mon_buf_, &mon_buf_len_);
   if(mon_buf_len_ > 0)
   {
     ...
     // PROCESS THE BUFFER CONTENT HERE
     ...
     // finally free the allocated memory
     ecal_free_mem(((void*)rcv_buf););
   }
   /* ========================= eCAL LICENSE =================================
    *
    * Copyright (C) 2016 - 2019 Continental Corporation
    *
    * Licensed under the Apache License, Version 2.0 (the "License");
    * you may not use this file except in compliance with the License.
    * You may obtain a copy of the License at
    * 
    *      http://www.apache.org/licenses/LICENSE-2.0
    * 
    * Unless required by applicable law or agreed to in writing, software
    * distributed under the License is distributed on an "AS IS" BASIS,
    * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    * See the License for the specific language governing permissions and
    * limitations under the License.
    *
    * ========================= eCAL LICENSE =================================
   */
   
   #ifndef ECAL_CLANG_H_INCLUDED
   #define ECAL_CLANG_H_INCLUDED
   
   #include <ecal/ecalc.h>
   #include <ecal/ecal_deprecate.h>
   
   /*************************************************************************/
   /*  common                                                               */
   /*************************************************************************/
   ECAL_DEPRECATE_SINCE_5_13("Will be removed in future eCAL versions.")
   ECAL_API const char* ecal_getversion();
   
   ECAL_DEPRECATE_SINCE_5_13("Will be removed in future eCAL versions.")
   ECAL_API int ecal_getversion_components(int* major_, int* minor_, int* patch_);
   
   ECAL_DEPRECATE_SINCE_5_13("Will be removed in future eCAL versions.")
   ECAL_API const char* ecal_getdate();
   
   ECAL_DEPRECATE_SINCE_5_13("Will be removed in future eCAL versions.")
   ECAL_API int ecal_initialize(int argc_, char **argv_, const char* unit_name_);
   
   ECAL_DEPRECATE_SINCE_5_13("Will be removed in future eCAL versions.")
   ECAL_API int ecal_finalize();
   
   ECAL_DEPRECATE_SINCE_5_13("Will be removed in future eCAL versions.")
   ECAL_API int ecal_is_initialized();
   
   ECAL_DEPRECATE_SINCE_5_13("Will be removed in future eCAL versions.")
   ECAL_API int ecal_set_unit_name(const char *unit_name_);
   
   ECAL_DEPRECATE_SINCE_5_13("Will be removed in future eCAL versions.")
   ECAL_API void ecal_set_process_state(const int severity_, const int level_, const char* info_);
   
   ECAL_DEPRECATE_SINCE_5_13("Will be removed in future eCAL versions.")
   ECAL_API bool ecal_ok();
   
   ECAL_DEPRECATE_SINCE_5_13("Will be removed in future eCAL versions.")
   ECAL_API void ecal_free_mem(void* mem_);
   
   ECAL_DEPRECATE_SINCE_5_13("Will be removed in future eCAL versions.")
   ECAL_API void ecal_sleep_ms(const long time_ms_);
   
   ECAL_DEPRECATE_SINCE_5_13("Will be removed in future eCAL versions.")
   ECAL_API void ecal_shutdown_process_uname(const char* unit_name_);
   
   ECAL_DEPRECATE_SINCE_5_13("Will be removed in future eCAL versions.")
   ECAL_API void ecal_shutdown_process_id(const int process_id_);
   
   ECAL_DEPRECATE_SINCE_5_13("Will be removed in future eCAL versions.")
   ECAL_API void ecal_shutdown_processes();
   
   ECAL_DEPRECATE_SINCE_5_13("Will be removed in future eCAL versions.")
   ECAL_API void ecal_shutdown_core();
   
   ECAL_DEPRECATE_SINCE_5_13("Will be removed in future eCAL versions.")
   ECAL_API void ecal_enable_loopback(const int state_);
   
   ECAL_DEPRECATE_SINCE_5_13("Will be removed in future eCAL versions.")
   ECAL_API bool ecal_get_type_name(const char* topic_name_, const char** topic_type_, int* topic_type_len_);
   
   ECAL_DEPRECATE_SINCE_5_13("Will be removed in future eCAL versions.")
   ECAL_API bool ecal_get_description(const char* topic_name_, const char** topic_desc_, int* topic_desc_len_);
   
   /*************************************************************************/
   /*  logging                                                              */
   /*************************************************************************/
   ECAL_DEPRECATE_SINCE_5_13("Will be removed in future eCAL versions.")
   ECAL_API void log_setlevel(const int level_);
   
   ECAL_DEPRECATE_SINCE_5_13("Will be removed in future eCAL versions.")
   ECAL_API void log_setcoretime(const double time_);
   
   ECAL_DEPRECATE_SINCE_5_13("Will be removed in future eCAL versions.")
   ECAL_API void log_message(const char* message_);
   
   /*************************************************************************/
   /*  publisher                                                            */
   /*************************************************************************/
   ECAL_DEPRECATE_SINCE_5_13("Will be removed in future eCAL versions.")
   ECAL_API ECAL_HANDLE pub_create(const char* topic_name_, const char* topic_type_);
   
   ECAL_DEPRECATE_SINCE_5_13("Will be removed in future eCAL versions.")
   ECAL_API bool pub_destroy(ECAL_HANDLE handle_);
   
   ECAL_DEPRECATE_SINCE_5_13("Will be removed in future eCAL versions.")
   ECAL_API bool pub_set_type_name(ECAL_HANDLE handle_, const char* topic_type_name_, const int topic_type_name_length_);
   
   ECAL_DEPRECATE_SINCE_5_13("Will be removed in future eCAL versions.")
   ECAL_API bool pub_set_description(ECAL_HANDLE handle_, const char* topic_desc_, const int topic_desc_length_);
   
   ECAL_DEPRECATE_SINCE_5_13("Will be removed in future eCAL versions.")
   ECAL_API bool pub_set_qos(ECAL_HANDLE handle_, struct SWriterQOSC qos_);
   
   ECAL_DEPRECATE_SINCE_5_13("Will be removed in future eCAL versions.")
   ECAL_API bool pub_get_qos(ECAL_HANDLE handle_, struct SWriterQOSC* qos_);
   
   ECAL_DEPRECATE_SINCE_5_13("Will be removed in future eCAL versions.")
   ECAL_API bool pub_set_layer_mode(ECAL_HANDLE handle_, const int layer_, const int mode_);
   
   ECAL_DEPRECATE_SINCE_5_13("Will be removed in future eCAL versions.")
   ECAL_API bool pub_set_max_bandwidth_udp(ECAL_HANDLE handle_, long bandwidth_);
   
   ECAL_DEPRECATE_SINCE_5_13("Will be removed in future eCAL versions.")
   ECAL_API int pub_send(ECAL_HANDLE handle_, const char* payload_, const int length_, const long long time_);
   
   ECAL_DEPRECATE_SINCE_5_13("Will be removed in future eCAL versions.")
   ECAL_API int pub_send_sync(ECAL_HANDLE handle_, const char* payload_, const int length_, const long long time_, const long long acknowledge_timeout_ms_);
   
   ECAL_DEPRECATE_SINCE_5_13("Will be removed in future eCAL versions.")
   ECAL_API bool pub_add_event_callback(ECAL_HANDLE handle_, enum eCAL_Publisher_Event type_, const PubEventCallbackCT callback_, void* par_);
   
   ECAL_DEPRECATE_SINCE_5_13("Will be removed in future eCAL versions.")
   ECAL_API bool pub_rem_event_callback(ECAL_HANDLE handle_, enum eCAL_Publisher_Event type_);
   
   /*************************************************************************/
   /*  subscriber                                                           */
   /*************************************************************************/
   ECAL_DEPRECATE_SINCE_5_13("Will be removed in future eCAL versions.")
   ECAL_API ECAL_HANDLE sub_create(const char* topic_name_, const char* topic_type_);
   
   ECAL_DEPRECATE_SINCE_5_13("Will be removed in future eCAL versions.")
   ECAL_API bool sub_destroy(ECAL_HANDLE handle_);
   
   ECAL_DEPRECATE_SINCE_5_13("Will be removed in future eCAL versions.")
   ECAL_API bool sub_set_qos(ECAL_HANDLE handle_, struct SReaderQOSC qos_);
   
   ECAL_DEPRECATE_SINCE_5_13("Will be removed in future eCAL versions.")
   ECAL_API bool sub_get_qos(ECAL_HANDLE handle_, struct SReaderQOSC* qos_);
   
   ECAL_DEPRECATE_SINCE_5_13("Will be removed in future eCAL versions.")
   ECAL_API int sub_receive(ECAL_HANDLE handle_, const char** rcv_buf_, int* rcv_buf_len_, long long* rcv_time_, const int timeout_);
   
   ECAL_DEPRECATE_SINCE_5_13("Will be removed in future eCAL versions.")
   ECAL_API bool sub_receive_buffer(ECAL_HANDLE handle_, const char** rcv_buf_, int* rcv_buf_len_, long long* rcv_time_, const int timeout_);
   
   ECAL_DEPRECATE_SINCE_5_13("Will be removed in future eCAL versions.")
   ECAL_API bool sub_add_receive_callback(ECAL_HANDLE handle_, const ReceiveCallbackCT callback_, void* par_);
   
   ECAL_DEPRECATE_SINCE_5_13("Will be removed in future eCAL versions.")
   ECAL_API bool sub_rem_receive_callback(ECAL_HANDLE handle_);
   
   ECAL_DEPRECATE_SINCE_5_13("Will be removed in future eCAL versions.")
   ECAL_API bool sub_add_event_callback(ECAL_HANDLE handle_, enum eCAL_Subscriber_Event type_, const SubEventCallbackCT callback_, void* par_);
   
   ECAL_DEPRECATE_SINCE_5_13("Will be removed in future eCAL versions.")
   ECAL_API bool sub_rem_event_callback(ECAL_HANDLE handle_, enum eCAL_Subscriber_Event type_);
   
   ECAL_DEPRECATE_SINCE_5_13("Will be removed in future eCAL versions.")
   ECAL_API bool sub_set_timeout(ECAL_HANDLE handle_, int timeout_);
   
   /*************************************************************************/
   /*  dyn_json_subscriber                                                  */
   /*************************************************************************/
   ECAL_DEPRECATE_SINCE_5_13("Will be removed in future eCAL versions.")
   ECAL_API ECAL_HANDLE dyn_json_sub_create(const char* topic_name_);
   
   ECAL_DEPRECATE_SINCE_5_13("Will be removed in future eCAL versions.")
   ECAL_API bool dyn_json_sub_destroy(ECAL_HANDLE handle_);
   
   ECAL_DEPRECATE_SINCE_5_13("Will be removed in future eCAL versions.")
   ECAL_API bool dyn_json_sub_add_receive_callback(ECAL_HANDLE handle_, const ReceiveCallbackCT callback_, void* par_);
   
   ECAL_DEPRECATE_SINCE_5_13("Will be removed in future eCAL versions.")
   ECAL_API bool dyn_json_sub_rem_receive_callback(ECAL_HANDLE handle_);
   
   /* TODO: not implemented and not used for now */
   //ECAL_API bool dyn_json_sub_add_event_callback(ECAL_HANDLE handle_, enum eCAL_Subscriber_Event type_, const EventCallbackCT callback_, void* par_);
   //ECAL_API bool dyn_json_sub_rem_event_callback(ECAL_HANDLE handle_, enum eCAL_Subscriber_Event type_);
   //ECAL_API bool dyn_json_sub_set_timeout(ECAL_HANDLE handle_, int timeout_);
   
   /*************************************************************************/
   /*  service                                                              */
   /*************************************************************************/
   ECAL_DEPRECATE_SINCE_5_13("Will be removed in future eCAL versions.")
   ECAL_API ECAL_HANDLE server_create(const char* service_name_);
   
   ECAL_DEPRECATE_SINCE_5_13("Will be removed in future eCAL versions.")
   ECAL_API bool server_destroy(ECAL_HANDLE handle_);
   
   ECAL_DEPRECATE_SINCE_5_13("Will be removed in future eCAL versions.")
   ECAL_API bool server_add_method_callback(ECAL_HANDLE handle_, const char* method_name_, const char* req_type_, const char* resp_type_, const MethodCallbackCT callback_, void* par_);
   
   ECAL_DEPRECATE_SINCE_5_13("Will be removed in future eCAL versions.")
   ECAL_API bool server_rem_method_callback(ECAL_HANDLE handle_, const char* method_name_);
   
   ECAL_DEPRECATE_SINCE_5_13("Will be removed in future eCAL versions.")
   ECAL_API ECAL_HANDLE client_create(const char* service_name_);
   
   ECAL_DEPRECATE_SINCE_5_13("Will be removed in future eCAL versions.")
   ECAL_API bool client_destroy(ECAL_HANDLE handle_);
   
   ECAL_DEPRECATE_SINCE_5_13("Will be removed in future eCAL versions.")
   ECAL_API bool client_set_hostname(ECAL_HANDLE handle_, const char* host_name_);
   
   ECAL_DEPRECATE_SINCE_5_13("Will be removed in future eCAL versions.")
   ECAL_API bool client_call_method(ECAL_HANDLE handle_, const char* method_name_, const char* request_, const int request_len_, const int timeout_);
   
   ECAL_DEPRECATE_SINCE_5_13("Will be removed in future eCAL versions.")
   ECAL_API bool client_call_method_async(ECAL_HANDLE handle_, const char* method_name_, const char* request_, const int request_len_, const int timeout_);
   
   /* TODO: not implemented and not used for now */
   //ECAL_API client_add_response_callback
   //ECAL_API client_rem_response_callback
   
   
   /*************************************************************************/
   /*  monitoring                                                           */
   /*************************************************************************/
   ECAL_DEPRECATE_SINCE_5_13("Will be removed in future eCAL versions.")
   ECAL_API int mon_initialize();
   
   ECAL_DEPRECATE_SINCE_5_13("Will be removed in future eCAL versions.")
   ECAL_API int mon_finalize();
   
   ECAL_DEPRECATE_SINCE_5_13("Will be removed in future eCAL versions.")
   ECAL_API int mon_set_excl_filter(const char* filter_);
   
   ECAL_DEPRECATE_SINCE_5_13("Will be removed in future eCAL versions.")
   ECAL_API int mon_set_incl_filter(const char* filter_);
   
   ECAL_DEPRECATE_SINCE_5_13("Will be removed in future eCAL versions.")
   ECAL_API int mon_set_filter_state(const bool state_);
   
   ECAL_DEPRECATE_SINCE_5_13("Will be removed in future eCAL versions.")
   ECAL_API int mon_get_monitoring(const char** mon_buf_, int* mon_buf_len_);
   
   ECAL_DEPRECATE_SINCE_5_13("Will be removed in future eCAL versions.")
   ECAL_API int mon_get_logging(const char** log_buf_, int* log_buf_len_);
   
   #endif /* ECAL_CLANG_H_INCLUDED */
