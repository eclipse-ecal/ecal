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

/**
 * @file   ecal_clang.h
 * @brief  eCAL C language interface (to wrap ecal into other languages easily)
**/

#ifndef ECAL_CLANG_H_INCLUDED
#define ECAL_CLANG_H_INCLUDED

#include <ecal/ecalc.h>

/*************************************************************************/
/*  common                                                               */
/*************************************************************************/
/**
 * @brief  Get eCAL version string.
 *
 * @return  Full eCAL version string.
**/
ECAL_API const char* ecal_getversion();

/**
 * @brief  Get eCAL version date.
 *
 * @return  Full eCAL version date string.
**/
ECAL_API const char* ecal_getdate();

/**
 * @brief Initialize eCAL API.
 *
 * @param argc_        Number of command line arguments.
 * @param argv_        Array of command line arguments.
 * @param unit_name_   Defines the name of the eCAL unit.
 *
 * @return Zero if succeeded, 1 if already initialized, -1 if failed.
**/
ECAL_API int ecal_initialize(int argc_, char **argv_, const char* unit_name_);

/**
 * @brief Finalize eCAL API.
 *
 * @return Zero if succeeded, 1 if already initialized, -1 if failed.
**/
ECAL_API int ecal_finalize();

/**
 * @brief  Set process state info.
 *
 * @param severity_  Severity.
 * @param level_     Severity level.
 * @param info_      Info message.
 *
**/
ECAL_API void ecal_set_process_state(const int severity_, const int level_, const char* info_);

/**
 * @brief Return the eCAL process state.
 *
 * @return  True if eCAL is in proper state.
**/
ECAL_API bool ecal_ok();

/**
 * @brief  Free an eCAL memory block allocated by functions like
 *         mon_get_monitoring, mon_get_logging,
 *         sub_receive, that let eCAL allocate
 *         the memory internally.
**/
/**
 * @code
 *            // let eCAL allocate memory for the subscriber buffer and return the pointer to 'rcv_buf'
 *            const char* rcv_buf     = nullptr;
 *            int         rcv_buf_len = 0;
 *            long long   rcv_time    = 0;
 *            int         timeout     = 0;
 *            int ret = sub_receive(subscriber_handle, &rcv_buf, &rcv_buf_len, &rcv_time, timeout);
 *            if(rcv_buf_len > 0)
 *            {
 *              ...
 *              // PROCESS THE BUFFER CONTENT HERE
 *              ...
 *              // finally free the allocated memory
 *              ecal_free_mem((void*)rcv_buf);
 *            }
 * @endcode
**/
ECAL_API void ecal_free_mem(void* mem_);

/**
 * @brief  Sleep current thread.
 *
 * @param  time_ms_  Time to sleep in ms.
**/
ECAL_API void ecal_sleep_ms(const long time_ms_);

/**
 * @brief Send shutdown event to specified local user process using it's unit name.
 *
 * @param unit_name_  Process unit name.
**/
ECAL_API void ecal_shutdown_process_uname(const char* unit_name_);

/**
 * @brief Send shutdown event to specified local user process using it's process id.
 *
 * @param process_id_  Process id.
**/
ECAL_API void ecal_shutdown_process_id(const int process_id_);

/**
 * @brief Send shutdown event to all local user processes.
**/
ECAL_API void ecal_shutdown_processes();

/**
 * @brief Send shutdown event to all local core components.
**/
ECAL_API void ecal_shutdown_core();

/**
 * @brief Enable eCAL message loop back,
 *          that means subscriber will receive messages from
 *          publishers of the same process (default == false).
 *
 * @param state_  Switch on message loop back..
**/
ECAL_API void ecal_enable_loopback(const int state_);

/**
 * @brief Gets type name of the specified topic.
 *
 * @param       topic_name_      Topic name.
 * @param [out] topic_type_      Pointer to store the type name information.
 * @param [out] topic_type_len_  Length of allocated buffer,
 *                               eCAL is allocating the buffer for you, use ecal_free_mem to free the buffer finally.
 *
 * @return  True if succeeded.
**/
ECAL_API bool ecal_get_type_name(const char* topic_name_, const char** topic_type_, int* topic_type_len_);

/**
 * @brief Gets type description of the specified topic.
 *
 * @param       topic_name_      Topic name.
 * @param [out] topic_desc_      Pointer to store the type description information.
 * @param [out] topic_desc_len_  Length of allocated buffer,
 *                               eCAL is allocating the buffer for you, use ecal_free_mem to free the buffer finally.
 *
 * @return  True if succeeded.
**/
ECAL_API bool ecal_get_description(const char* topic_name_, const char** topic_desc_, int* topic_desc_len_);

/*************************************************************************/
/*  logging                                                              */
/*************************************************************************/
/**
 * @brief Sets the log level.
 *
 * @param level_  The level.
**/
ECAL_API void log_setlevel(const int level_);

/**
 * @brief Set the current measured core time in s (for user implemented measuring).
 *
 * @param time_  The core time.
**/
ECAL_API void log_setcoretime(const double time_);

/**
 * @brief Log a message (with current log level).
 *
 * @param message_  The log message string.
**/
ECAL_API void log_message(const char* message_);

/*************************************************************************/
/*  publisher                                                            */
/*************************************************************************/
/**
 * @brief Create a publisher.
 *
 * @param topic_name_      Unique topic name.
 * @param topic_type_      Topic type name.
 *
 * @return  Handle of the created publisher or NULL if failed.
**/
ECAL_API ECAL_HANDLE pub_create(const char* topic_name_, const char* topic_type_);

/**
 * @brief Destroy a publisher. 
 *
 * @param handle_  Publisher handle. 
 *
 * @return  True if succeeded.
**/
ECAL_API bool pub_destroy(ECAL_HANDLE handle_);

/**
 * @brief Setup topic type description.
 *
 * @param handle_             Publisher handle.
 * @param topic_desc_         Topic type description.
 * @param topic_desc_length_  Topic type description length.
 *
 * @return  True if succeeded.
**/
ECAL_API bool pub_set_description(ECAL_HANDLE handle_, const char* topic_desc_, const int topic_desc_length_);

/**
 * @brief Set publisher quality of service attributes.
 *
 * @param handle_  Publisher handle.
 * @param qos_     Quality of service policies.
 *
 * @return  True if succeeded.
**/
ECAL_API bool pub_set_qos(ECAL_HANDLE handle_, struct SWriterQOSC qos_);

/**
 * @brief Get publisher quality of service attributes.
 *
 * @param handle_  Publisher handle.
 * @param qos_     Quality of service policies.
 *
 * @return  True if succeeded.
**/
ECAL_API bool pub_get_qos(ECAL_HANDLE handle_, struct SWriterQOSC* qos_);

/**
 * @brief Set publisher send mode for specific transport layer.
 *
 * @param handle_  Publisher handle.
 * @param layer_   Transport layer.
 * @param mode_    Send mode.
 *
 * @return  True if succeeded.
**/
ECAL_API bool pub_set_layer_mode(ECAL_HANDLE handle_, const int layer_, const int mode_);

/**
 * @brief Set publisher maximum transmit bandwidth for the udp layer.
 *
 * @param handle_     Publisher handle.
 * @param bandwidth_  Maximum bandwidth in bytes/s (-1 == unlimited).
 *
 * @return  True if succeeded.
**/
ECAL_API bool pub_set_max_bandwidth_udp(ECAL_HANDLE handle_, long bandwidth_);

/**
 * @brief Send a message to all subscribers.
 *
 * @param handle_   Publisher handle.
 * @param payload_  Buffer that contains content to send.
 * @param length_   Send buffer length.
 * @param time_     Send time (-1 = use eCAL system time in us, default = -1).
 *
 * @return  Number of bytes sent.
**/
ECAL_API int pub_send(ECAL_HANDLE handle_, const char* payload_, const int length_, const long long time_);

/**
 * @brief Add callback function for publisher events.
 *
 * @param handle_    Publisher handle.
 * @param type_      The event type to react on.
 * @param callback_  The callback function to add.
 * @param par_       User defined context that will be forwarded to the callback function.
 *
 * @return  True if succeeded.
**/
ECAL_API bool pub_add_event_callback(ECAL_HANDLE handle_, enum eCAL_Publisher_Event type_, const PubEventCallbackCT callback_, void* par_);

/**
 * @brief Remove callback function for publisher events.
 *
 * @param handle_  Publisher handle.
 * @param type_    The event type to remove.
 *
 * @return  True if succeeded.
**/
ECAL_API bool pub_rem_event_callback(ECAL_HANDLE handle_, enum eCAL_Publisher_Event type_);

/*************************************************************************/
/*  subscriber                                                           */
/*************************************************************************/
/**
* @brief Create a subscriber.
 *
 * @param topic_name_      Unique topic name.
 * @param topic_type_      Topic type name.
 *
 * @return  Handle of the created subscriber or NULL if failed.
**/
ECAL_API ECAL_HANDLE sub_create(const char* topic_name_, const char* topic_type_);

/**
 * @brief Destroy a subscriber.
 *
 * @param handle_  Subscriber handle.
 *
 * @return  True if succeeded.
**/
ECAL_API bool sub_destroy(ECAL_HANDLE handle_);

/**
 * @brief Set subscriber quality of service attributes.
 *
 * @param handle_  Subscriber handle.
 * @param qos_     Quality of service policies.
 *
 * @return  True if succeeded.
**/
ECAL_API bool sub_set_qos(ECAL_HANDLE handle_, struct SReaderQOSC qos_);

/**
 * @brief Get subscriber quality of service attributes.
 *
 * @param handle_  Subscriber handle.
 * @param qos_     Quality of service policies.
 *
 * @return  True if succeeded.
**/
ECAL_API bool sub_get_qos(ECAL_HANDLE handle_, struct SReaderQOSC* qos_);

/**
 * @brief Receive a message from the publisher.
 *
 * @param       handle_       Subscriber handle.
 * @param [out] rcv_buf_      Buffer to store the received message content.
 * @param [out] rcv_buf_len_  Length of allocated buffer,
 *                            eCAL is allocating the buffer for you, use ecal_free_mem to free the buffer finally.
 * @param [out] rcv_time_     Time from publisher in us.
 * @param       timeout_      Maximum time before receive operation returns (in milliseconds, -1 means infinite).
 *
 * @return  Length of received buffer.
**/
ECAL_API int sub_receive(ECAL_HANDLE handle_, const char** rcv_buf_, int* rcv_buf_len_, long long* rcv_time_, const int timeout_);

/**
 * @brief Receive a message from the publisher (able to process zero length buffer).
 *
 * @param       handle_       Subscriber handle.
 * @param [out] rcv_buf_      Buffer to store the received message content.
 * @param [out] rcv_buf_len_  Length of allocated buffer,
 *                            eCAL is allocating the buffer for you, use ecal_free_mem to free the buffer finally.
 * @param [out] rcv_time_     Time from publisher in us.
 * @param       timeout_      Maximum time before receive operation returns (in milliseconds, -1 means infinite).
 *
 * @return  True if succeeded.
**/
ECAL_API bool sub_receive_buffer(ECAL_HANDLE handle_, const char** rcv_buf_, int* rcv_buf_len_, long long* rcv_time_, const int timeout_);

/**
 * @brief Add callback function for incoming receives.
 *
 * @param handle_    Subscriber handle.
 * @param callback_  The callback function to add.
 * @param par_       User defined context that will be forwarded to the callback function.
 *
 * @return  True if succeeded.
**/
ECAL_API bool sub_add_receive_callback(ECAL_HANDLE handle_, const ReceiveCallbackCT callback_, void* par_);

/**
 * @brief Remove callback function for incoming receives.
 *
 * @param  handle_  Subscriber handle.
 *
 * @return  True if succeeded.
**/
ECAL_API bool sub_rem_receive_callback(ECAL_HANDLE handle_);

/**
 * @brief Add callback function for subscriber events.
 *
 * @param handle_    Subscriber handle.
 * @param type_      The event type to react on.
 * @param callback_  The callback function to add.
 * @param par_       User defined context that will be forwarded to the callback function.
 *
 * @return  True if succeeded.
**/
ECAL_API bool sub_add_event_callback(ECAL_HANDLE handle_, enum eCAL_Subscriber_Event type_, const SubEventCallbackCT callback_, void* par_);

/**
 * @brief Remove callback function for subscriber events.
 *
 * @param handle_  Subscriber handle.
 * @param type_    The event type to remove.
 *
 * @return  True if succeeded.
**/
ECAL_API bool sub_rem_event_callback(ECAL_HANDLE handle_, enum eCAL_Subscriber_Event type_);

/**
 * @brief Set the timeout parameter for triggering
 *          the timeout callback.
 *
 * @param handle_   Subscriber handle.
 * @param timeout_  The timeout in milliseconds.
 *
 * @return  True if succeeded.
**/
ECAL_API bool sub_set_timeout(ECAL_HANDLE handle_, int timeout_);

/*************************************************************************/
/*  dyn_json_subscriber                                                  */
/*************************************************************************/
/**
 * @brief Create a subscriber.
 *
 * @param topic_name_  Unique topic name.
 *
 * @return  Handle to created subscriber or NULL if failed.
**/
ECAL_API ECAL_HANDLE dyn_json_sub_create(const char* topic_name_);

/**
 * @brief Destroy a subscriber.
 *
 * @param handle_  Subscriber handle.
 *
 * @return  True if succeeded.
**/
ECAL_API bool dyn_json_sub_destroy(ECAL_HANDLE handle_);

/**
 * @brief Add callback function for incoming receives.
 *
 * @param handle_    Subscriber handle.
 * @param callback_  The callback function to add.
 * @param par_       User defined context that will be forwarded to the callback function.
 *
 * @return  True if succeeded.
**/
ECAL_API bool dyn_json_sub_add_receive_callback(ECAL_HANDLE handle_, const ReceiveCallbackCT callback_, void* par_);

/**
 * @brief Remove callback function for incoming receives.
 *
 * @param  handle_  Subscriber handle.
 *
 * @return  True if succeeded.
**/
ECAL_API bool dyn_json_sub_rem_receive_callback(ECAL_HANDLE handle_);

/* TODO: not implemented and not used for now */
//ECAL_API bool dyn_json_sub_add_event_callback(ECAL_HANDLE handle_, enum eCAL_Subscriber_Event type_, const EventCallbackCT callback_, void* par_);
//ECAL_API bool dyn_json_sub_rem_event_callback(ECAL_HANDLE handle_, enum eCAL_Subscriber_Event type_);
//ECAL_API bool dyn_json_sub_set_timeout(ECAL_HANDLE handle_, int timeout_);

/*************************************************************************/
/*  service                                                              */
/*************************************************************************/
/**
 * @brief Create a server.
 *
 * @param service_name_  Service name.
 *
 * @return  Handle to created server or NULL if failed.
**/
ECAL_API ECAL_HANDLE server_create(const char* service_name_);

/**
 * @brief Destroy a server.
 *
 * @param handle_  Server handle.
 *
 * @return  True if succeeded.
**/
ECAL_API bool server_destroy(ECAL_HANDLE handle_);

/**
 * @brief Add server method callback.
 *
 * @param handle_       Server handle.
 * @param method_name_  Service method name.
 * @param req_type_     Method request type (default = "").
 * @param resp_type_    Method response type (default = "").
 * @param callback_     Callback function for server request.
 * @param par_          User defined context that will be forwarded to the request function.
 *
 * @return  True if succeeded.
**/
ECAL_API bool server_add_method_callback(ECAL_HANDLE handle_, const char* method_name_, const char* req_type_, const char* resp_type_, const MethodCallbackCT callback_, void* par_);

/**
 * @brief Remove server method callback.
 *
 * @param handle_       Server handle.
 * @param method_name_  Service method name.
 *
 * @return  True if succeeded.
**/
ECAL_API bool server_rem_method_callback(ECAL_HANDLE handle_, const char* method_name_);

/**
 * @brief Create a client.
 *
 * @param service_name_  Service name.
 *
 * @return  Handle to created client or NULL if failed.
**/
ECAL_API ECAL_HANDLE client_create(const char* service_name_);

/**
 * @brief Destroy a client.
 *
 * @param handle_  Client handle.
 *
 * @return  True if succeeded.
**/
ECAL_API bool client_destroy(ECAL_HANDLE handle_);

/**
 * @brief Change the host name filter for that client instance
 *
 * @param handle_     Client handle.
 * @param host_name_  Host name filter (empty or "*" == all hosts)
 *
 * @return  True if succeeded.
**/
ECAL_API bool client_set_hostname(ECAL_HANDLE handle_, const char* host_name_);

/**
 * @brief Call method of this service (none blocking variant with callback).
 *
 * @param handle_       Client handle.
 * @param method_name_  Method name.
 * @param request_      Request message buffer.
 * @param request_len_  Request message length.
 * @param timeout_      Maximum time before operation returns (in milliseconds, -1 means infinite).
 *
 * @return  True if succeeded.
**/
ECAL_API bool client_call_method(ECAL_HANDLE handle_, const char* method_name_, const char* request_, const int request_len_, const int timeout_);

/**
 * @brief Call method of this service (asynchronously with callback).
 *
 * @param handle_       Client handle.
 * @param method_name_  Method name.
 * @param request_      Request message buffer.
 * @param request_len_  Request message length.
 * @param timeout_      Maximum time before operation returns (in milliseconds, -1 means infinite).
 *
 * @return  True if succeeded.
**/
ECAL_API bool client_call_method_async(ECAL_HANDLE handle_, const char* method_name_, const char* request_, const int request_len_, const int timeout_);

/* TODO: not implemented and not used for now */
//ECAL_API client_add_response_callback
//ECAL_API client_rem_response_callback


/*************************************************************************/
/*  monitoring                                                           */
/*************************************************************************/
/**
 * @brief Initialize eCAL monitoring API.
 *
 * @return Zero if succeeded, 1 if already initialized, -1 if failed.
**/
ECAL_API int mon_initialize();

/**
 * @brief Finalize eCAL monitoring API.
 *
 * @return Zero if succeeded, 1 if already initialized, -1 if failed.
**/
ECAL_API int mon_finalize();

/**
 * @brief Set topics filter blacklist regular expression.
 *
 * @param filter_  Topic filter as regular expression.
 *
 * @return Zero if succeeded.
**/
ECAL_API int mon_set_excl_filter(const char* filter_);

/**
 * @brief Set topics filter whitelist regular expression.
 *
 * @param filter_  Topic filter as regular expression.
 *
 * @return Zero if succeeded.
**/
ECAL_API int mon_set_incl_filter(const char* filter_);

/**
 * @brief Switch topics filter using regular expression on/off.
 *
 * @param state_  Filter on / off state.
 *
 * @return Zero if succeeded.
**/
ECAL_API int mon_set_filter_state(const bool state_);

/**
 * @brief Get monitoring protobuf string.
 *
 * @param [out] mon_buf_      Pointer to store the monitoring information.
 * @param [out] mon_buf_len_  Length of allocated buffer,
 *                            eCAL is allocating the buffer for you, use ecal_free_mem to free the buffer finally.
 *
 * @return  Monitoring buffer length or zero if failed.
**/
/**
 * @code
 *            // let eCAL allocate memory for the monitoring buffer and return the pointer to 'buf'
 *            const char* mon_buf_     = NULL;
 *            int         mon_buf_len_ = 0;
 *            mon_get_monitoring(subscriber_handle, &mon_buf_, &mon_buf_len_);
 *            if(mon_buf_len_ > 0)
 *            {
 *              ...
 *              // PROCESS THE BUFFER CONTENT HERE
 *              ...
 *              // finally free the allocated memory
 *              ecal_free_mem(((void*)rcv_buf););
 *            }
 * @endcode
**/
ECAL_API int mon_get_monitoring(const char** mon_buf_, int* mon_buf_len_);

/**
 * @brief Get logging string.
 *
 * @param [out] log_buf_      Pointer to store the monitoring information.
 * @param [out] log_buf_len_  Length of allocated buffer,
 *                            eCAL is allocating the buffer for you, use ecal_free_mem to free the buffer finally.
 *
 * @return  Logging buffer length or zero if failed.
**/
ECAL_API int mon_get_logging(const char** log_buf_, int* log_buf_len_);

#endif /* ECAL_CLANG_H_INCLUDED */
