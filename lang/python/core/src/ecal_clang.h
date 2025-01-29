/* ========================= eCAL LICENSE =================================
 *
 * Copyright (C) 2016 - 2025 Continental Corporation
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

#pragma once

#include <ecal_c/ecal.h>

/*************************************************************************/
/*  common                                                               */
/*************************************************************************/
/**
 * @brief  Get eCAL version string.
 *
 * @return  Full eCAL version string.
**/
const char* ecal_getversion();

/**
 * @brief  Get eCAL version as separated integer values.
 *
 * @param [out] major_  The eCAL major version number.
 * @param [out] minor_  The eCAL minor version number.
 * @param [out] patch_  The eCAL patch version number.
 *
 * @return  Zero if succeeded.
**/
int ecal_getversion_components(int* major_, int* minor_, int* patch_);

/**
 * @brief  Get eCAL version date.
 *
 * @return  Full eCAL version date string.
**/
const char* ecal_getdate();

/**
 * @brief Initialize eCAL API.
 *
 * @param argc_        Number of command line arguments.
 * @param argv_        Array of command line arguments.
 * @param unit_name_   Defines the name of the eCAL unit.
 *
 * @return Zero if succeeded.
**/
int ecal_initialize(const char* unit_name_);

/**
 * @brief Finalize eCAL API.
 *
 * @return Zero if succeeded.
**/
int ecal_finalize();

/**
 * @brief Check eCAL initialize state.
 *
 * @return 1 if eCAL is initialized.
**/
int ecal_is_initialized();

/**
 * @brief  Set process state info.
 *
 * @param severity_  Severity.
 * @param level_     Severity level.
 * @param info_      Info message.
 *
**/
void ecal_set_process_state(int severity_, int level_, const char* info_);

/**
 * @brief Return the eCAL process state.
 *
 * @return  True if eCAL is in proper state.
**/
bool ecal_ok();

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
void ecal_free_mem(void* mem_);

/**
 * @brief  Sleep current thread.
 *
 * @param  time_ms_  Time to sleep in ms.
**/
void ecal_sleep_ms(long time_ms_);

/**
 * @brief Send shutdown event to specified local user process using it's unit name.
 *
 * @param unit_name_  Process unit name.
**/
void ecal_shutdown_process_uname(const char* unit_name_);

/**
 * @brief Send shutdown event to specified local user process using it's process id.
 *
 * @param process_id_  Process id.
**/
void ecal_shutdown_process_id(int process_id_);

/**
 * @brief Send shutdown event to all local user processes.
**/
void ecal_shutdown_processes();

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
bool ecal_get_type_name(const char* topic_name_, const char** topic_type_, int* topic_type_len_);

/**
 * @brief Gets type encoding of the specified topic.
 *
 * @param       topic_name_          Topic name.
 * @param [out] topic_encoding_      Pointer to store the type encoding information.
 * @param [out] topic_encoding_len_  Length of allocated buffer,
 *                                   eCAL is allocating the buffer for you, use ecal_free_mem to free the buffer finally.
 *
 * @return  True if succeeded.
**/
bool ecal_get_type_encoding(const char* topic_name_, const char** topic_encoding_, int* topic_encoding_len_);

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
bool ecal_get_description(const char* topic_name_, const char** topic_desc_, int* topic_desc_len_);

/**
 * @brief Log a message (with current log level).
 *
 * @param message_  The log message string.
**/
void log_message(const eCAL_Logging_eLogLevel& level_, const char* message_);

/*************************************************************************/
/*  publisher                                                            */
/*************************************************************************/
/**
 * @brief Create a publisher.
 *
 * @param topic_name_         Unique topic name.
 * @param topic_type_         Topic type name.
 * @param topic_enc_          Topic type encoding.
 * @param topic_desc_         Topic type description.
 * @param topic_desc_length_  Topic type description length.
 *
 * @return  Handle of the created publisher or NULL if failed.
**/
ECAL_HANDLE pub_create(const char* topic_name_, const char* topic_type_, const char* topic_enc_, const char* topic_desc_, const int topic_desc_length_);

/**
 * @brief Destroy a publisher. 
 *
 * @param handle_  Publisher handle. 
 *
 * @return  True if succeeded.
**/
bool pub_destroy(ECAL_HANDLE handle_);

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
int pub_send(ECAL_HANDLE handle_, const char* payload_, int length_, long long time_);

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
bool pub_add_event_callback(ECAL_HANDLE handle_, enum eCAL_Publisher_Event type_, PubEventCallbackCT callback_, void* par_);

/**
 * @brief Remove callback function for publisher events.
 *
 * @param handle_  Publisher handle.
 * @param type_    The event type to remove.
 *
 * @return  True if succeeded.
**/
bool pub_rem_event_callback(ECAL_HANDLE handle_, enum eCAL_Publisher_Event type_);

/*************************************************************************/
/*  subscriber                                                           */
/*************************************************************************/
/**
* @brief Create a subscriber.
 *
 * @param topic_name_         Unique topic name.
 * @param topic_type_         Topic type name.
 * @param topic_enc_          Topic type encoding.
 * @param topic_desc_         Topic type description.
 * @param topic_desc_length_  Topic type description length.
 *
 * @return  Handle of the created subscriber or NULL if failed.
**/
ECAL_HANDLE sub_create(const char* topic_name_, const char* topic_type_, const char* topic_enc_, const char* topic_desc_, const int topic_desc_length_);

/**
 * @brief Destroy a subscriber.
 *
 * @param handle_  Subscriber handle.
 *
 * @return  True if succeeded.
**/
bool sub_destroy(ECAL_HANDLE handle_);

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
int sub_receive(ECAL_HANDLE handle_, const char** rcv_buf_, int* rcv_buf_len_, long long* rcv_time_, int timeout_);

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
bool sub_receive_buffer(ECAL_HANDLE handle_, const char** rcv_buf_, int* rcv_buf_len_, long long* rcv_time_, int timeout_);

/**
 * @brief Add callback function for incoming receives.
 *
 * @param handle_    Subscriber handle.
 * @param callback_  The callback function to add.
 * @param par_       User defined context that will be forwarded to the callback function.
 *
 * @return  True if succeeded.
**/
bool sub_add_receive_callback(ECAL_HANDLE handle_, ReceiveCallbackCT callback_, void* par_);

/**
 * @brief Remove callback function for incoming receives.
 *
 * @param  handle_  Subscriber handle.
 *
 * @return  True if succeeded.
**/
bool sub_rem_receive_callback(ECAL_HANDLE handle_);

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
bool sub_add_event_callback(ECAL_HANDLE handle_, enum eCAL_Subscriber_Event type_, SubEventCallbackCT callback_, void* par_);

/**
 * @brief Remove callback function for subscriber events.
 *
 * @param handle_  Subscriber handle.
 * @param type_    The event type to remove.
 *
 * @return  True if succeeded.
**/
bool sub_rem_event_callback(ECAL_HANDLE handle_, enum eCAL_Subscriber_Event type_);

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
ECAL_HANDLE dyn_json_sub_create(const char* topic_name_);

/**
 * @brief Destroy a subscriber.
 *
 * @param handle_  Subscriber handle.
 *
 * @return  True if succeeded.
**/
bool dyn_json_sub_destroy(ECAL_HANDLE handle_);

/**
 * @brief Add callback function for incoming receives.
 *
 * @param handle_    Subscriber handle.
 * @param callback_  The callback function to add.
 * @param par_       User defined context that will be forwarded to the callback function.
 *
 * @return  True if succeeded.
**/
bool dyn_json_sub_add_receive_callback(ECAL_HANDLE handle_, ReceiveCallbackCT callback_, void* par_);

/**
 * @brief Remove callback function for incoming receives.
 *
 * @param  handle_  Subscriber handle.
 *
 * @return  True if succeeded.
**/
bool dyn_json_sub_rem_receive_callback(ECAL_HANDLE handle_);

/* TODO: not implemented and not used for now */
//bool dyn_json_sub_add_event_callback(ECAL_HANDLE handle_, enum eCAL_Subscriber_Event type_, const EventCallbackCT callback_, void* par_);
//bool dyn_json_sub_rem_event_callback(ECAL_HANDLE handle_, enum eCAL_Subscriber_Event type_);
//bool dyn_json_sub_set_timeout(ECAL_HANDLE handle_, int timeout_);

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
ECAL_HANDLE server_create(const char* service_name_);

/**
 * @brief Destroy a server.
 *
 * @param handle_  Server handle.
 *
 * @return  True if succeeded.
**/
bool server_destroy(ECAL_HANDLE handle_);

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
bool server_add_method_callback(ECAL_HANDLE handle_, const char* method_name_, const char* req_type_, const char* resp_type_, MethodCallbackCT callback_, void* par_);

/**
 * @brief Remove server method callback.
 *
 * @param handle_       Server handle.
 * @param method_name_  Service method name.
 *
 * @return  True if succeeded.
**/
bool server_rem_method_callback(ECAL_HANDLE handle_, const char* method_name_);

/**
 * @brief Create a client.
 *
 * @param service_name_  Service name.
 *
 * @return  Handle to created client or NULL if failed.
**/
ECAL_HANDLE client_create(const char* service_name_);

/**
 * @brief Destroy a client.
 *
 * @param handle_  Client handle.
 *
 * @return  True if succeeded.
**/
bool client_destroy(ECAL_HANDLE handle_);

/**
 * @brief Change the host name filter for that client instance
 *
 * @param handle_     Client handle.
 * @param host_name_  Host name filter (empty or "*" == all hosts)
 *
 * @return  True if succeeded.
**/
bool client_set_hostname(ECAL_HANDLE handle_, const char* host_name_);

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
bool client_call_method(ECAL_HANDLE handle_, const char* method_name_, const char* request_, int request_len_, int timeout_);

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
bool client_call_method_async(ECAL_HANDLE handle_, const char* method_name_, const char* request_, int request_len_, int timeout_);

/*************************************************************************/
/*  monitoring                                                           */
/*************************************************************************/
/**
 * @brief Initialize eCAL monitoring API.
 *
 * @return Zero if succeeded, 1 if already initialized, -1 if failed.
**/
int mon_initialize();

/**
 * @brief Finalize eCAL monitoring API.
 *
 * @return Zero if succeeded, 1 if already initialized, -1 if failed.
**/
int mon_finalize();
