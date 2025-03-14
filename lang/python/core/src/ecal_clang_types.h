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
 * @brief  eCAL C language interface types
**/

#pragma once

/**
 * @brief Flag to indicate eCAL to allocate/deallocate memory.
**/
#define ECAL_ALLOCATE_4ME 0

/**
 * @brief Common handle for eCAL C API function calls.
**/
typedef void* ECAL_HANDLE;

/**
 * @brief eCAL version struct (C variant)
 **/
typedef struct
{
  int major; //!< major version number
  int minor; //!< minor version number
  int patch; //!< patch version number
} eCAL_SVersion;

/**
 * @brief eCAL transport layer types.
**/
enum eTransportLayerC
{
  tlayer_none       = 0,
  tlayer_udp_mc     = 1,
  tlayer_shm        = 4,
  tlayer_tcp        = 5,
  tlayer_all        = 255
};

/**
 * @brief eCAL transport layer modes.
**/
enum eSendModeC
{
  smode_none = -1,
  smode_off  = 0,
  smode_on,
  smode_auto
};

enum eCAL_Process_eStartMode
{
  proc_smode_normal = 0,  /*!<  0 == start mode normal     */
  proc_smode_hidden = 1,  /*!<  1 == start mode hidden     */
  proc_smode_minimized = 2,  /*!<  2 == start mode minimized  */
  proc_smode_maximized = 3,  /*!<  3 == start mode maximized  */
};

/**
 @brief  Process severity
**/
enum eCAL_Process_eSeverity
{
  proc_sev_unknown = 0,  /*!<  0 == condition unknown     */
  proc_sev_healthy = 1,  /*!<  1 == process healthy       */
  proc_sev_warning = 2,  /*!<  2 == process warning level */
  proc_sev_critical = 3,  /*!<  3 == process critical      */
  proc_sev_failed = 4,  /*!<  4 == process failed        */
};

/**
 * @brief Process Severity Level
 *
 * enumerations for ECAL_API::SetState functionality
 * where the lowest process severity is generally proc_sev_level1
**/
enum eCAL_Process_eSeverity_Level
{
  proc_sev_level1 = 1,  /*!<  default severity level 1 */
  proc_sev_level2 = 2,  /*!<  severity level 2         */
  proc_sev_level3 = 3,  /*!<  severity level 3         */
  proc_sev_level4 = 4,  /*!<  severity level 4         */
  proc_sev_level5 = 5,  /*!<  severity level 5         */
};

/**
 * @brief Values that represent different log level to filter on monitoring.
**/
enum eCAL_Logging_eLogLevel
{
  log_level_none = 0,
  log_level_all = 255,
  log_level_info = 1,
  log_level_warning = 2,
  log_level_error = 4,
  log_level_fatal = 8,
  log_level_debug1 = 16,
  log_level_debug2 = 32,
  log_level_debug3 = 64,
  log_level_debug4 = 128,
};

#define eCAL_Init_Publisher   0x01                          /*!< Initialize Publisher API            */
#define eCAL_Init_Subscriber  0x02                          /*!< Initialize Subscriber API           */
#define eCAL_Init_Service     0x04                          /*!< Initialize Service API              */
#define eCAL_Init_Monitoring  0x08                          /*!< Initialize Monitoring API           */
#define eCAL_Init_Logging     0x10                          /*!< Initialize Logging API              */
#define eCAL_Init_TimeSync    0x20                          /*!< Initialize Time API                 */

#define eCAL_Init_All        (eCAL_Init_Publisher  \
                            | eCAL_Init_Subscriber \
                            | eCAL_Init_Service    \
                            | eCAL_Init_Monitoring \
                            | eCAL_Init_Logging    \
                            | eCAL_Init_TimeSync)           /*!< Initialize complete eCAL API        */

#define eCAL_Init_Default    (eCAL_Init_Publisher  \
                            | eCAL_Init_Subscriber \
                            | eCAL_Init_Service    \
                            | eCAL_Init_Logging    \
                            | eCAL_Init_TimeSync)           /*!< Initialize default eCAL API          */

/**
 * @brief eCAL subscriber event callback type.
**/
enum eCAL_Subscriber_Event
{
  sub_event_none              = 0,
  sub_event_connected         = 1,
  sub_event_disconnected      = 2,
  sub_event_dropped           = 3,
  sub_event_corrupted         = 5,
  sub_event_update_connection = 6,
};

/**
 * @brief eCAL publisher event callback type.
**/
enum eCAL_Publisher_Event
{
  pub_event_none              = 0,
  pub_event_connected         = 1,
  pub_event_disconnected      = 2,
  pub_event_dropped           = 3,
  pub_event_update_connection = 4,
};

/**
 * @brief eCAL service client event callback type.
**/
enum eCAL_Client_Event
{
  client_event_none         = 0,
  client_event_connected    = 1,
  client_event_disconnected = 2,
  client_event_timeout      = 3,
};

/**
 * @brief eCAL service server event callback type.
**/
enum eCAL_Server_Event
{
  server_event_none         = 0,
  server_event_connected    = 1,
  server_event_disconnected = 2,
};

#ifdef _MSC_VER
#pragma pack(push, 8)
#endif

/**
 * @brief eCAL subscriber receive callback struct (C variant).
**/
struct SReceiveCallbackDataC
{
  void*     buf;    //!< payload buffer
  long      size;   //!< payload buffer size
  long long id;     //!< source id
  long long time;   //!< source time stamp
  long long clock;  //!< source write clock
};

/**
 * @brief eCAL publisher event callback struct (C variant).
**/
struct SPubEventCallbackDataC
{
  enum eCAL_Publisher_Event  type;         //!< event type
  long long                  time;         //!< event time stamp
  long long                  clock;        //!< event clock
  const char*                topic_id;     //!< topic id of the connected subscriber                          (for pub_event_update_connection only)
  const char*                topic_name;   //!< topic type encoding of the connected subscriber               (for pub_event_update_connection only)
  const char*                tencoding;    //!< topic type name of the connected subscriber                   (for pub_event_update_connection only)
  const char*                tdesc;        //!< topic type descriptor information of the connected subscriber (for pub_event_update_connection only)
};

/**
 * @brief eCAL subscriber event callback struct (C variant).
**/
struct SSubEventCallbackDataC
{
  enum eCAL_Subscriber_Event type;         //!< event type
  long long                  time;         //!< event time stamp
  long long                  clock;        //!< event clock
  const char*                topic_id;     //!< topic id of the connected publisher                          (for sub_event_update_connection only)
  const char*                topic_name;   //!< topic type encoding of the connected publisher               (for sub_event_update_connection only)
  const char*                tencoding;    //!< topic type name of the connected publisher                   (for sub_event_update_connection only)
  const char*                tdesc;        //!< topic type descriptor information of the connected publisher (for sub_event_update_connection only)
};

/**
 * @brief eCAL client event callback struct (C variant).
**/
struct SClientEventCallbackDataC
{
  enum eCAL_Client_Event type;  //!< event type
  long long              time;  //!< event time stamp
};

/**
 * @brief eCAL server event callback struct (C variant).
**/
struct SServerEventCallbackDataC
{
  enum eCAL_Server_Event type;  //!< event type
  long long              time;  //!< event time stamp
};

#ifdef _MSC_VER
#pragma pack(pop)
#endif

/**
 * @brief eCAL receive callback function
 *
 * @param topic_name_  Topic name of the data source (publisher).
 * @param data_        Data payload struct.
 * @param par_         Forwarded user defined parameter.
**/
typedef void (*ReceiveCallbackCT)(const char* topic_name_, const struct SReceiveCallbackDataC* data_, void* par_);

/**
 * @brief eCAL timer callback function
 *
 * @param par_         Forwarded user defined parameter.
**/
typedef void (*TimerCallbackCT)(void* par_);

/**
 * @brief eCAL publisher event callback function
 *
 * @param topic_name_  The topic name of the data connection (subscriber).
 * @param data_        Event type struct.
 * @param par_         Forwarded user defined parameter.
**/
typedef void(*SubEventCallbackCT)(const char* topic_name_, const struct SSubEventCallbackDataC* data_, void* par_);

/**
 * @brief eCAL subscriber event callback function
 *
 * @param topic_name_  The topic name of the data connection (publisher).
 * @param data_        Event type struct.
 * @param par_         Forwarded user defined parameter.
**/
typedef void(*PubEventCallbackCT)(const char* topic_name_, const struct SPubEventCallbackDataC* data_, void* par_);

/**
 * @brief eCAL client event callback function
 *
 * @param name_  The service name.
 * @param data_  Event type struct.
 * @param par_   Forwarded user defined parameter.
**/
typedef void(*ClientEventCallbackCT)(const char* name_, const struct SClientEventCallbackDataC* data_, void* par_);

/**
 * @brief eCAL server event callback function
 *
 * @param name_  The service name.
 * @param data_  Event type struct.
 * @param par_   Forwarded user defined parameter.
**/
typedef void(*ServerEventCallbackCT)(const char* name_, const struct SServerEventCallbackDataC* data_, void* par_);

/**
 * @brief  Service call state.
**/
enum eCallState
{
  call_state_none = 0,    //!< undefined
  call_state_executed,    //!< executed (successfully)
  call_state_timeouted,   //!< timeout
  call_state_failed       //!< failed
};

#ifdef _MSC_VER
#pragma pack(push, 8)
#endif

/**
 * @brief Service response struct containing the (responding) server informations and the response itself (C API).
**/
struct SServiceResponseC
{
  const char*      host_name;     //!< service host name
  const char*      service_name;  //!< name of the service
  const char*      service_id;    //!< id of the service
  const char*      method_name;   //!< name of the service method
  const char*      error_msg;     //!< human readable error message
  int              ret_state;     //!< return state of the called service method
  enum eCallState  call_state;    //!< call state (see eCallState)
  const char*      response;      //!< service response
  int              response_len;  //!< service response length (response string could include zeros)
};

#ifdef _MSC_VER
#pragma pack(pop)
#endif

/**
 * @brief eCAL service method callback function (server side)
 *
 * @param method_             Method name.
 * @param req_type_           Type of the request message.
 * @param resp_type_          Type of the response message.
 * @param request_            Request payload.
 * @param request_len_        Request payload length.
 * @param [out] reponse_      Method response payload.
 * @param [out] reponse_len_  Method response payload length.
 * @param par_                Forwarded user defined parameter.
**/
typedef int(*MethodCallbackCT)(const char* method_, const char* req_type_, const char* resp_type_, const char* request_, int request_len_, void** response_, int* response_len_, void* par_);

/**
 * @brief eCAL service response callback function (client side)
 *
 * @param service_response_   Service response struct containing the (responding) server informations and the response itself.
 * @param par_                Forwarded user defined parameter.
**/
typedef void(*ResponseCallbackCT)(const struct SServiceResponseC* service_response_, void* par_);
