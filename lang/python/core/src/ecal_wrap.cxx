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
* @brief  eCAL python interface
**/
#define PY_SSIZE_T_CLEAN
#include "Python.h"
#include "modsupport.h"

#include <ecal/ecal.h>
#include <ecal/v5/ecal_client.h>
#include <ecal/v5/ecal_server.h>
#include <ecal/v5/ecal_subscriber.h>

#include "ecal_clang.h"

#include <unordered_map>
#include <atomic>


/****************************************/
/*      types                           */
/****************************************/
typedef std::unordered_map<ECAL_HANDLE, PyObject*> PySubscriberCallbackMapT;
typedef std::unordered_map<std::string, PyObject*> PyServerMethodCallbackMapT;
typedef std::unordered_map<ECAL_HANDLE, PyObject*> PyClientCallbackMapT;


/****************************************/
/*      globals                         */
/****************************************/
#if (PY_VERSION_HEX < ((3 << 24) | (9 << 16)))
//   ^^               ^^
//   PY_Version     < 3.9
// 
// => Only call PyEval_InitThreads() for Python up to 3.8
// 
// Version Check Dokumentation: https://github.com/python/cpython/blob/3.7/Include/patchlevel.h
// Init Threads Documentation:  https://docs.python.org/3.10/c-api/init.html#c.PyEval_InitThreads

  #define ECAL_PY_INIT_THREADS_NEEDED 1
#endif

#if ECAL_PY_INIT_THREADS_NEEDED
  static std::atomic<int>     g_pygil_init;
#endif

PySubscriberCallbackMapT    g_subscriber_pycallback_map;
PyServerMethodCallbackMapT  g_server_method_pycallback_map;
PyClientCallbackMapT        g_client_pycallback_map;

eCAL_Logging_eLogLevel      g_python_log_level = log_level_info;


/****************************************/
/*      help                            */
/****************************************/
PyObject* PyAnswerHandle(ECAL_HANDLE handle_)
{
  return(Py_BuildValue("n", handle_));
}


/****************************************/
/*      initialize                      */
/****************************************/
PyObject* initialize(PyObject* /*self*/, PyObject* args)
{
  char* unit_name = nullptr;
  if (!PyArg_ParseTuple(args, "s", &unit_name))
    return nullptr;

  /* pass argument to the initialize function */
  int init{ 0 };
  Py_BEGIN_ALLOW_THREADS
    init = ecal_initialize(unit_name);
  Py_END_ALLOW_THREADS

  return(Py_BuildValue("i", init));
}

/****************************************/
/*      finalize                        */
/****************************************/
PyObject* finalize(PyObject* /*self*/, PyObject* /*args*/)
{
  int finalize{ 0 };
  Py_BEGIN_ALLOW_THREADS
    finalize = ecal_finalize();
  Py_END_ALLOW_THREADS
  return(Py_BuildValue("i", finalize));
}

/****************************************/
/*      is_initialized                  */
/****************************************/
PyObject* is_initialized(PyObject* /*self*/, PyObject* /*args*/)
{
  return(Py_BuildValue("i", ecal_is_initialized()));
}

/****************************************/
/*      getversion                      */
/****************************************/
PyObject* getversion(PyObject* /*self*/, PyObject* /*args*/)
{
  return(Py_BuildValue("s", ecal_getversion()));
}

/****************************************/
/*      getversioncomponents            */
/****************************************/
PyObject* getversion_components(PyObject* /*self*/, PyObject* /*args*/)
{
  int major = 0;
  int minor = 0;
  int patch = 0;
  ecal_getversion_components(&major, &minor, &patch);
  return(Py_BuildValue("iii", major, minor, patch));
}

/****************************************/
/*      getdate                         */
/****************************************/
PyObject* getdate(PyObject* /*self*/, PyObject* /*args*/)
{
  return(Py_BuildValue("s", ecal_getdate()));
}

/****************************************/
/*      getmicroseconds                 */
/****************************************/
PyObject* getmicroseconds(PyObject* /*self*/, PyObject* /*args*/)
{
  return(Py_BuildValue("iL", 0, eCAL::Time::GetMicroSeconds()));
}

/****************************************/
/*      set_process_state               */
/****************************************/
PyObject* set_process_state(PyObject* /*self*/, PyObject* args)
{
  int   severity = 0;
  int   level    = 0;
  char* info     = nullptr;

  if (!PyArg_ParseTuple(args, "iis", &severity, &level, &info))
    return nullptr;

  ecal_set_process_state(severity, level, info);

  Py_RETURN_NONE;
}


/****************************************/
/*      ok                              */
/****************************************/
PyObject* ok(PyObject* /*self*/, PyObject* /*args*/)
{
  return(Py_BuildValue("i", ecal_ok()));
}

/****************************************/
/*      shutdown_process_uname          */
/****************************************/
PyObject* shutdown_process_uname(PyObject* /*self*/, PyObject* args)
{
  char* unit_name = nullptr;

  if (!PyArg_ParseTuple(args, "s", &unit_name)) 
    return nullptr;

  ecal_shutdown_process_uname(unit_name);

  Py_RETURN_NONE;
}

/****************************************/
/*      shutdown_process_id             */
/****************************************/
PyObject* shutdown_process_id(PyObject* /*self*/, PyObject* args)
{
  int process_id = 0;

  if (!PyArg_ParseTuple(args, "i", &process_id)) 
    return nullptr;

  ecal_shutdown_process_id(process_id);

  Py_RETURN_NONE;
}

/****************************************/
/*      shutdown_processes              */
/****************************************/
PyObject* shutdown_processes(PyObject* /*self*/, PyObject* /*args*/)
{
  ecal_shutdown_processes();

  Py_RETURN_NONE;
}

/****************************************/
/*      log_setlevel                    */
/****************************************/
PyObject* log_setlevel(PyObject* /*self*/, PyObject* args)
{
  int level = 0;

  if (!PyArg_ParseTuple(args, "i", &level)) 
    return nullptr;

  g_python_log_level = eCAL_Logging_eLogLevel(level);

  Py_RETURN_NONE;
}

/****************************************/
/*      log_message                     */
/****************************************/
PyObject* log_message(PyObject* /*self*/, PyObject* args)
{
  char* message = nullptr;

  if (!PyArg_ParseTuple(args, "s", &message)) 
    return nullptr;

  log_message(g_python_log_level, message);

  Py_RETURN_NONE;
}

/****************************************/
/*      pub_create                      */
/****************************************/
PyObject* pub_create(PyObject* /*self*/, PyObject* args)
{
  char*       topic_name     = nullptr;
  char*       topic_type     = nullptr;
  char*       topic_enc      = nullptr;
  char*       topic_desc     = nullptr;
  Py_ssize_t  topic_desc_len = 0;

  if (!PyArg_ParseTuple(args, "sssy#", &topic_name, &topic_type, &topic_enc, &topic_desc, &topic_desc_len))
    return nullptr;

  return(PyAnswerHandle(pub_create(topic_name, topic_type, topic_enc, topic_desc, (int)topic_desc_len)));
}

/****************************************/
/*      pub_destroy                     */
/****************************************/
PyObject* pub_destroy(PyObject* /*self*/, PyObject* args)
{
  ECAL_HANDLE topic_handle = nullptr;

  if (!PyArg_ParseTuple(args, "n", &topic_handle)) 
    return nullptr;

  return(Py_BuildValue("i", pub_destroy(topic_handle)));
}

/****************************************/
/*      pub_send                        */
/****************************************/
PyObject* pub_send(PyObject* /*self*/, PyObject* args)
{
  ECAL_HANDLE  topic_handle = nullptr;
  char*        payload      = nullptr;
  Py_ssize_t   length       = 0;
  PY_LONG_LONG time         = 0;

  if (!PyArg_ParseTuple(args, "ny#L", &topic_handle, &payload, &length, &time))
    return nullptr;

  int sent{ 0 };
  //Py_BEGIN_ALLOW_THREADS
    sent = pub_send(topic_handle, payload, (int)length, time);
  //Py_END_ALLOW_THREADS

  return(Py_BuildValue("i", sent));
}


/****************************************/
/*      sub_create                      */
/****************************************/
PyObject* sub_create(PyObject* /*self*/, PyObject* args)
{
  char*       topic_name     = nullptr;
  char*       topic_type     = nullptr;
  char*       topic_enc      = nullptr;
  char*       topic_desc     = nullptr;
  Py_ssize_t  topic_desc_len = 0;

  if (!PyArg_ParseTuple(args, "sssy#", &topic_name, &topic_type, &topic_enc, &topic_desc, &topic_desc_len))
    return nullptr;

  ECAL_HANDLE sub{ nullptr };
  Py_BEGIN_ALLOW_THREADS
    sub = sub_create(topic_name, topic_type, topic_enc, topic_desc, (int)topic_desc_len);
  Py_END_ALLOW_THREADS
    return(PyAnswerHandle(sub));
}


/****************************************/
/*      sub_destroy                     */
/****************************************/
PyObject* sub_destroy(PyObject* /*self*/, PyObject* args)
{
  ECAL_HANDLE topic_handle = nullptr;

  if (!PyArg_ParseTuple(args, "n", &topic_handle)) 
    return nullptr;

  bool destroyed{ false };
  Py_BEGIN_ALLOW_THREADS
    destroyed = sub_destroy(topic_handle);
  Py_END_ALLOW_THREADS
  return(Py_BuildValue("i", destroyed));
}

/****************************************/
/*      sub_receive                     */
/****************************************/
PyObject* sub_receive(PyObject* /*self*/, PyObject* args)
{
  ECAL_HANDLE topic_handle = nullptr;
  int         timeout      = 0;

  if (!PyArg_ParseTuple(args, "ni", &topic_handle, &timeout)) 
    return nullptr;

  const char* rcv_buf     = nullptr;
  int         rcv_buf_len = 0;
  long long   rcv_time    = 0;
  
  int ret{ 0 };
  Py_BEGIN_ALLOW_THREADS
    ret = sub_receive_buffer(topic_handle, &rcv_buf, &rcv_buf_len, &rcv_time, timeout);
  Py_END_ALLOW_THREADS

  PyObject* ret_obj = Py_BuildValue("iy#L", ret, rcv_buf, (Py_ssize_t)rcv_buf_len, rcv_time);
  ecal_free_mem((void*)rcv_buf);

  return(ret_obj);
}

/****************************************/
/*      sub_set_callback                */
/****************************************/
static void c_subscriber_callback(const char* topic_name_, const struct eCAL::v5::SReceiveCallbackData* data_, ECAL_HANDLE handle_, const std::string& /*python_formatter*/)
{
#if ECAL_PY_INIT_THREADS_NEEDED
  if (!g_pygil_init)
  {
    g_pygil_init = 1;
    PyEval_InitThreads();
  }
#endif

  PyGILState_STATE state = PyGILState_Ensure();

  PyObject* topic_name = Py_BuildValue("s",  topic_name_);
  PyObject* content    = PyByteArray_FromStringAndSize((char*)data_->buf, (Py_ssize_t)data_->size);
  PyObject* time       = Py_BuildValue("L",  data_->time);

  PyObject* args = PyTuple_New(3);
  PyTuple_SetItem(args, 0, topic_name);
  PyTuple_SetItem(args, 1, content);
  PyTuple_SetItem(args, 2, time);

  PySubscriberCallbackMapT::const_iterator iter = g_subscriber_pycallback_map.find(handle_);
  if (iter != g_subscriber_pycallback_map.end())
  {
    PyObject* py_callback = iter->second;
    PyObject_CallObject(py_callback, args);
    if (PyErr_Occurred()) { PyErr_Print(); }
  }

  Py_DECREF(args);

  PyGILState_Release(state);
}

PyObject* sub_set_callback(PyObject* /*self*/, PyObject* args)
{
  ECAL_HANDLE topic_handle = nullptr;
  PyObject*   cb_func      = nullptr;

  if (!PyArg_ParseTuple(args, "nO", &topic_handle, &cb_func))
    return nullptr;

  eCAL::v5::CSubscriber* sub = (eCAL::v5::CSubscriber*)topic_handle;
  if (!sub)
  {
    return(Py_BuildValue("is", -1, "subscriber invalid"));
  }

  PySubscriberCallbackMapT::const_iterator iter = g_subscriber_pycallback_map.find(sub);
  if (iter != g_subscriber_pycallback_map.end())
  {
    PyObject* py_callback = iter->second;
    Py_XDECREF(py_callback);                    /* Dispose of previous callback */
    g_subscriber_pycallback_map.erase(iter);               /* Delete previous callback */
  }

  if (PyCallable_Check(cb_func))
  {
#if ECAL_PY_INIT_THREADS_NEEDED
    if (!g_pygil_init)
    {
      g_pygil_init = 1;
      PyEval_InitThreads();
    }
#endif

    Py_XINCREF(cb_func);                        /* Add a reference to new callback */
    g_subscriber_pycallback_map[sub] = cb_func;            /* Add new callback */

    bool added_callback{ false };

    std::string python_formatter{ "y#" };
    Py_BEGIN_ALLOW_THREADS
    added_callback = sub->AddReceiveCallback(std::bind(c_subscriber_callback, std::placeholders::_1, std::placeholders::_2, sub, python_formatter));
    Py_END_ALLOW_THREADS

    if (added_callback)
    {
      return Py_BuildValue("is", 1, "callback set");
    }
  }
  return Py_BuildValue("is", 0, "error: could not set callback");
}

/****************************************/
/*      sub_rem_callback                */
/****************************************/
PyObject* sub_rem_callback(PyObject* /*self*/, PyObject* args)
{
  ECAL_HANDLE topic_handle = nullptr;
  PyObject*   cb_func      = nullptr;

  if (!PyArg_ParseTuple(args, "nO", &topic_handle, &cb_func))
    return nullptr;

  eCAL::v5::CSubscriber* sub = (eCAL::v5::CSubscriber*)topic_handle;
  if (!sub)
  {
    return(Py_BuildValue("is", -1, "subscriber invalid"));
  }

  PySubscriberCallbackMapT::const_iterator iter = g_subscriber_pycallback_map.find(sub);
  if (iter != g_subscriber_pycallback_map.end())
  {
    PyObject* py_callback = iter->second;
    Py_XDECREF(py_callback);                    /* Dispose of previous callback */
    g_subscriber_pycallback_map.erase(iter);               /* Delete previous callback */
  }

  bool removed_callback{ false };
  Py_BEGIN_ALLOW_THREADS
    removed_callback = sub->RemReceiveCallback();
  Py_END_ALLOW_THREADS
  if (removed_callback)
  {
    return Py_BuildValue("is", 1, "callback removed");
  }
  else
  {
    return Py_BuildValue("is", 0, "error: could not remove callback");
  }
}


/****************************************/
/*      server_create                   */
/****************************************/
PyObject* server_create(PyObject* /*self*/, PyObject* args)
{
  char* service_name = nullptr;

  if (!PyArg_ParseTuple(args, "s", &service_name))
    return nullptr;

  ECAL_HANDLE server{ nullptr };
  Py_BEGIN_ALLOW_THREADS
    server = server_create(service_name);
  Py_END_ALLOW_THREADS

  return(PyAnswerHandle(server));
}

/****************************************/
/*      server_destroy                  */
/****************************************/
PyObject* server_destroy(PyObject* /*self*/, PyObject* args)
{
  ECAL_HANDLE server_handle = nullptr;

  if (!PyArg_ParseTuple(args, "n", &server_handle))
    return nullptr;

  bool destroyed{ false };
  Py_BEGIN_ALLOW_THREADS
    destroyed = server_destroy(server_handle);
  Py_END_ALLOW_THREADS

  return(Py_BuildValue("i", destroyed));
}

/****************************************/
/*      server_add_method_callback      */
/****************************************/
static int c_server_method_callback(const std::string& method_name_, const std::string& request_type_, const std::string& response_type_, const std::string& request_, std::string& response_, ECAL_HANDLE handle_)
{
  int ret_state = 0;

#if ECAL_PY_INIT_THREADS_NEEDED
  if (!g_pygil_init)
  {
    g_pygil_init = 1;
    PyEval_InitThreads();
  }
#endif

  PyGILState_STATE gstate = PyGILState_Ensure();

  PyObject* method_name = Py_BuildValue("s", method_name_.c_str());

  const std::string fmt("y#");
  PyObject* request_type  = Py_BuildValue(fmt.data(), request_type_.data(),  (Py_ssize_t)request_type_.size());
  PyObject* response_type = Py_BuildValue(fmt.data(), response_type_.data(), (Py_ssize_t)response_type_.size());
  PyObject* request       = Py_BuildValue(fmt.data(), request_.data(),   (Py_ssize_t)request_.size());

  PyObject* args = PyTuple_New(4);
  PyTuple_SetItem(args, 0, method_name);
  PyTuple_SetItem(args, 1, request_type);
  PyTuple_SetItem(args, 2, response_type);
  PyTuple_SetItem(args, 3, request);

  eCAL::v5::CServiceServer* server = (eCAL::v5::CServiceServer*)handle_;
  std::string server_method = server->GetServiceName();
  server_method = method_name_ + "@" + server_method;

  PyServerMethodCallbackMapT::const_iterator iter = g_server_method_pycallback_map.find(server_method);
  if (iter != g_server_method_pycallback_map.end())
  {
    PyObject* py_callback = iter->second;
    PyObject* result = PyObject_CallObject(py_callback, args);
    if (PyErr_Occurred()) { PyErr_Print(); }

    int         cb_ret_state    = 0;
    const char* cb_response     = nullptr;
    Py_ssize_t  cb_response_len = 0;
    if (PyArg_ParseTuple(result, "iy#", &cb_ret_state, &cb_response, &cb_response_len))
    {
      ret_state = cb_ret_state;
      response_ = std::string(cb_response, (int)cb_response_len);
    }
    else
    {
      // parse error !!
    }
  }

  Py_DECREF(args);

  PyGILState_Release(gstate);

  return ret_state;
}

PyObject* server_add_method_callback(PyObject* /*self*/, PyObject* args)   // (server_handle, method_name, request_type, response_type, callback)
{
  ECAL_HANDLE server_handle     = nullptr;
  char*       method_name       = nullptr;
  char*       request_type      = nullptr;
  char*       response_type     = nullptr;
  PyObject*   cb_func           = nullptr;

  if (!PyArg_ParseTuple(args, "nsssO", &server_handle, &method_name, &request_type, &response_type, &cb_func))
    return nullptr;

  eCAL::v5::CServiceServer* server = (eCAL::v5::CServiceServer*)server_handle;
  if (!server)
  {
    return(Py_BuildValue("is", -1, "server invalid"));
  }

  std::string server_method = server->GetServiceName();
  server_method = std::string(method_name) + "@" + server_method;
  PyServerMethodCallbackMapT::const_iterator iter = g_server_method_pycallback_map.find(server_method);
  if (iter != g_server_method_pycallback_map.end())
  {
    PyObject* py_callback = iter->second;
    Py_XDECREF(py_callback);                     /* Dispose of previous callback */
    g_server_method_pycallback_map.erase(iter);  /* Delete previous callback */
  }

  if (PyCallable_Check(cb_func))
  {
#if ECAL_PY_INIT_THREADS_NEEDED
    if (!g_pygil_init)
    {
      g_pygil_init = 1;
      PyEval_InitThreads();
    }
#endif

    Py_XINCREF(cb_func);                               /* Add a reference to new callback */
    g_server_method_pycallback_map[server_method] = cb_func;  /* Add new callback */

    bool added_callback{ false };
    Py_BEGIN_ALLOW_THREADS
      added_callback = server->AddMethodCallback(method_name, request_type, response_type, std::bind(c_server_method_callback, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4, std::placeholders::_5, server));
    Py_END_ALLOW_THREADS
    
    if (added_callback)
    {
      return Py_BuildValue("is", 1, "callback set");
    }
  }

  return Py_BuildValue("is", 0, "error: could not set callback");
}

/****************************************/
/*      server_rem_method_callback      */
/****************************************/
PyObject* server_rem_method_callback(PyObject* /*self*/, PyObject* args)
{
  ECAL_HANDLE server_handle = nullptr;
  const char* method_name   = nullptr;

  if (!PyArg_ParseTuple(args, "ns", &server_handle, &method_name))
    return nullptr;

  eCAL::v5::CServiceServer* server = (eCAL::v5::CServiceServer*)server_handle;
  if (!server)
  {
    return(Py_BuildValue("is", -1, "server invalid"));
  }

  std::string server_method = server->GetServiceName();
  server_method = std::string(method_name) + "@" + server_method;
  PyServerMethodCallbackMapT::const_iterator iter = g_server_method_pycallback_map.find(server_method);
  if (iter != g_server_method_pycallback_map.end())
  {
    PyObject* py_callback = iter->second;
    Py_XDECREF(py_callback);                     /* Dispose of previous callback */
    g_server_method_pycallback_map.erase(iter);  /* Delete previous callback */
  }

  bool removed_callback{ false };
  Py_BEGIN_ALLOW_THREADS
    removed_callback = server->RemMethodCallback(method_name);
  Py_END_ALLOW_THREADS
  if (removed_callback)
  {
    return Py_BuildValue("is", 1, "callback removed");
  }
  else
  {
    return Py_BuildValue("is", 0, "error: could not remove callback");
  }
}

/****************************************/
/*      client_create                   */
/****************************************/
PyObject* client_create(PyObject* /*self*/, PyObject* args)
{
  char* service_name = nullptr;

  if (!PyArg_ParseTuple(args, "s", &service_name))
    return nullptr;

  ECAL_HANDLE client{ nullptr };
  Py_BEGIN_ALLOW_THREADS
    client = client_create(service_name);
  Py_END_ALLOW_THREADS

  return(PyAnswerHandle(client));
}

/****************************************/
/*      client_destroy                  */
/****************************************/
PyObject* client_destroy(PyObject* /*self*/, PyObject* args)
{
  ECAL_HANDLE client_handle = nullptr;

  if (!PyArg_ParseTuple(args, "n", &client_handle))
    return nullptr;

  bool destroyed{ nullptr };
  Py_BEGIN_ALLOW_THREADS
    destroyed = client_destroy(client_handle);
  Py_END_ALLOW_THREADS

  return(Py_BuildValue("i", destroyed));
}

/****************************************/
/*      client_set_hostname             */
/****************************************/
PyObject* client_set_hostname(PyObject* /*self*/, PyObject* args)   // (client_handle, host_name)
{
  ECAL_HANDLE client_handle = nullptr;
  const char* host_name     = nullptr;

  if (!PyArg_ParseTuple(args, "ns", &client_handle, &host_name))
    return nullptr;

  bool set_hostname{ false };
  Py_BEGIN_ALLOW_THREADS
    set_hostname = client_set_hostname(client_handle, host_name);
  Py_END_ALLOW_THREADS

  return(Py_BuildValue("i", set_hostname));
}

/****************************************/
/*      client_call_method              */
/****************************************/
PyObject* client_call_method(PyObject* /*self*/, PyObject* args)   // (client_handle, method_name, request)
{
  ECAL_HANDLE client_handle = nullptr;
  const char* method_name   = nullptr;
  const char* request       = nullptr;
  Py_ssize_t  request_len   = 0;
  int         timeout       = -1;

  PyArg_ParseTuple(args, "nsy#i", &client_handle, &method_name, &request, &request_len, &timeout);

  bool called_method{ false };
  Py_BEGIN_ALLOW_THREADS
    called_method = client_call_method(client_handle, method_name, request, (int)request_len, timeout);
  Py_END_ALLOW_THREADS

  return(Py_BuildValue("i", called_method));
}

/****************************************/
/*      client_add_response_callback    */
/****************************************/
static void c_client_callback(const struct eCAL::v5::SServiceResponse& service_response_, ECAL_HANDLE handle_)
{
#if ECAL_PY_INIT_THREADS_NEEDED
  if (!g_pygil_init)
  {
    g_pygil_init = 1;
    PyEval_InitThreads();
  }
#endif

  PyGILState_STATE gstate = PyGILState_Ensure();

  PyObject* args = PyTuple_New(2);
  PyObject* dict = PyDict_New();
  PyObject* val;

  val = Py_BuildValue("s", service_response_.host_name.c_str());
  PyDict_SetItemString(dict, "host_name", val); Py_DECREF(val);
  val = Py_BuildValue("s", service_response_.service_name.c_str());
  PyDict_SetItemString(dict, "service_name", val); Py_DECREF(val);
  val = Py_BuildValue("s", service_response_.method_name.c_str());
  PyDict_SetItemString(dict, "method_name", val); Py_DECREF(val);
  val = Py_BuildValue("s", service_response_.error_msg.c_str());
  PyDict_SetItemString(dict, "error_msg", val); Py_DECREF(val);
  val = Py_BuildValue("i", service_response_.ret_state);
  PyDict_SetItemString(dict, "ret_state", val); Py_DECREF(val);
  std::string call_state_s;
  switch (service_response_.call_state)
  {
  case eCAL::eCallState::none:
    call_state_s = "call_state_none";
    break;
  case eCAL::eCallState::executed:
    call_state_s = "call_state_executed";
    break;
  case eCAL::eCallState::failed:
    call_state_s = "call_state_failed";
    break;
  default:
    call_state_s = "unknown";
    break;
  }
  val = Py_BuildValue("s", call_state_s.c_str());
  PyDict_SetItemString(dict, "call_state", val); Py_DECREF(val);
  PyTuple_SetItem(args, 0, dict);

  val = Py_BuildValue("y#", service_response_.response.c_str(), (Py_ssize_t)service_response_.response.size());
  PyTuple_SetItem(args, 1, val);

  PyClientCallbackMapT::const_iterator iter = g_client_pycallback_map.find(handle_);
  if (iter != g_client_pycallback_map.end())
  {
    PyObject* py_callback = iter->second;
    PyObject_CallObject(py_callback, args);
    if (PyErr_Occurred()) { PyErr_Print(); }
  }

  Py_DECREF(args);

  PyGILState_Release(gstate);
}

PyObject* client_add_response_callback(PyObject* /*self*/, PyObject* args)   // (client_handle, callback)
{
  ECAL_HANDLE client_handle = nullptr;
  PyObject*   cb_func = nullptr;

  if (!PyArg_ParseTuple(args, "nO", &client_handle, &cb_func))
    return nullptr;

  eCAL::v5::CServiceClient* client = (eCAL::v5::CServiceClient*)client_handle;
  if (!client)
  {
    return(Py_BuildValue("is", -1, "client invalid"));
  }

  PyClientCallbackMapT::const_iterator iter = g_client_pycallback_map.find(client_handle);
  if (iter != g_client_pycallback_map.end())
  {
    PyObject* py_callback = iter->second;
    Py_XDECREF(py_callback);              /* Dispose of previous callback */
    g_client_pycallback_map.erase(iter);  /* Delete previous callback */
  }

  if (PyCallable_Check(cb_func))
  {
#if ECAL_PY_INIT_THREADS_NEEDED
    if (!g_pygil_init)
    {
      g_pygil_init = 1;
      PyEval_InitThreads();
    }
#endif

    Py_XINCREF(cb_func);                               /* Add a reference to new callback */
    g_client_pycallback_map[client_handle] = cb_func;  /* Add new callback */

    bool added_callback{ false };
    Py_BEGIN_ALLOW_THREADS
      added_callback = client->AddResponseCallback(std::bind(c_client_callback, std::placeholders::_1, client));
    Py_END_ALLOW_THREADS
    
    if (added_callback)
    {
      return Py_BuildValue("is", 1, "callback set");
    }
  }
  return Py_BuildValue("is", 0, "error: could not set callback");
}

/****************************************/
/*      client_rem_response_callback    */
/****************************************/
PyObject* client_rem_response_callback(PyObject* /*self*/, PyObject* args)   // (client_handle)
{
  ECAL_HANDLE client_handle = nullptr;

  if (!PyArg_ParseTuple(args, "n", &client_handle))
    return nullptr;

  eCAL::v5::CServiceClient* client = (eCAL::v5::CServiceClient*)client_handle;
  if (!client)
  {
    return(Py_BuildValue("is", -1, "client invalid"));
  }

  PyClientCallbackMapT::const_iterator iter = g_client_pycallback_map.find(client_handle);
  if (iter != g_client_pycallback_map.end())
  {
    PyObject* py_callback = iter->second;
    Py_XDECREF(py_callback);              /* Dispose of previous callback */
    g_client_pycallback_map.erase(iter);  /* Delete previous callback */
  }

  bool removed_callback{ false };
  Py_BEGIN_ALLOW_THREADS
    removed_callback = client->RemResponseCallback();
  Py_END_ALLOW_THREADS

  if (removed_callback)
  {
    return Py_BuildValue("is", 1, "callback removed");
  }
  else
  {
    return Py_BuildValue("is", 0, "error: could not remove callback");
  }
}


/****************************************/
/*      mon_initialize                  */
/****************************************/
PyObject* mon_initialize(PyObject* /*self*/, PyObject* /*args*/)
{
  return(Py_BuildValue("i", mon_initialize()));
}

/****************************************/
/*      mon_finalize                    */
/****************************************/
PyObject* mon_finalize(PyObject* /*self*/, PyObject* /*args*/)
{
  return(Py_BuildValue("i", mon_finalize()));
}

/****************************************/
/*      mon_monitoring                  */
/****************************************/
namespace
{
  void PopulateTopics2List(PyObject* topic_list_, const std::vector<eCAL::Monitoring::STopicMon>& topic_vec_)
  {
    for (const auto& topic : topic_vec_)
    {
      PyObject* topicDict = PyDict_New();
      PyList_Append(topic_list_, topicDict); Py_DECREF(topicDict);

      PyObject* val = Py_BuildValue("i", topic.registration_clock);
      PyDict_SetItemString(topicDict, "rclock", val); Py_DECREF(val);

      val = Py_BuildValue("s", topic.host_name.c_str());
      PyDict_SetItemString(topicDict, "hname", val); Py_DECREF(val);

      val = Py_BuildValue("s", topic.shm_transport_domain.c_str());
      PyDict_SetItemString(topicDict, "shm_transport_domain", val); Py_DECREF(val);

      val = Py_BuildValue("i", topic.process_id);
      PyDict_SetItemString(topicDict, "pid", val); Py_DECREF(val);

      val = Py_BuildValue("s", topic.process_name.c_str());
      PyDict_SetItemString(topicDict, "pname", val); Py_DECREF(val);

      val = Py_BuildValue("s", topic.unit_name.c_str());
      PyDict_SetItemString(topicDict, "uname", val); Py_DECREF(val);

      val = Py_BuildValue("s", std::to_string(topic.topic_id).c_str());
      PyDict_SetItemString(topicDict, "tid", val); Py_DECREF(val);

      val = Py_BuildValue("s", topic.topic_name.c_str());
      PyDict_SetItemString(topicDict, "tname", val); Py_DECREF(val);

      val = Py_BuildValue("s", topic.direction.c_str());
      PyDict_SetItemString(topicDict, "direction", val); Py_DECREF(val);

      val = Py_BuildValue("s", topic.datatype_information.name.c_str());
      PyDict_SetItemString(topicDict, "tdatatype_name", val); Py_DECREF(val);

      val = Py_BuildValue("s", topic.datatype_information.encoding.c_str());
      PyDict_SetItemString(topicDict, "tdatatype_encoding", val); Py_DECREF(val);

      val = Py_BuildValue("y#", topic.datatype_information.descriptor.c_str(), topic.datatype_information.descriptor.length());
      PyDict_SetItemString(topicDict, "tdatatype_descriptor", val); Py_DECREF(val);

      PyObject* layerList = PyList_New(0);
      for (auto layer : topic.transport_layer)
      {
        PyObject* layerDict = PyDict_New();
        PyList_Append(layerList, layerDict); Py_DECREF(layerDict);

        switch (layer.type)
        {
        case eCAL::Monitoring::eTransportLayerType::none:
          val = Py_BuildValue("s", "tl_none");
          break;
        case eCAL::Monitoring::eTransportLayerType::shm:
          val = Py_BuildValue("s", "tl_ecal_shm");
          break;
        case eCAL::Monitoring::eTransportLayerType::udp_mc:
          val = Py_BuildValue("s", "tl_ecal_udp_mc");
          break;
        case eCAL::Monitoring::eTransportLayerType::tcp:
          val = Py_BuildValue("s", "tl_ecal_tcp");
          break;
        }
        PyDict_SetItemString(layerDict, "type", val); Py_DECREF(val);

        val = Py_BuildValue("i", layer.version);
        PyDict_SetItemString(layerDict, "version", val); Py_DECREF(val);

        val = Py_BuildValue("O", layer.active ? Py_True : Py_False);
        PyDict_SetItemString(layerDict, "active", val); Py_DECREF(val);
      }
      PyDict_SetItemString(topicDict, "layer", layerList); Py_DECREF(layerList);

      val = Py_BuildValue("i", topic.topic_size);
      PyDict_SetItemString(topicDict, "tsize", val); Py_DECREF(val);

      val = Py_BuildValue("i", topic.connections_local);
      PyDict_SetItemString(topicDict, "connections_loc", val); Py_DECREF(val);

      val = Py_BuildValue("i", topic.connections_external);
      PyDict_SetItemString(topicDict, "connections_ext", val); Py_DECREF(val);

      val = Py_BuildValue("i", topic.message_drops);
      PyDict_SetItemString(topicDict, "message_drops", val); Py_DECREF(val);

      val = Py_BuildValue("i", topic.data_id);
      PyDict_SetItemString(topicDict, "did", val); Py_DECREF(val);

      val = Py_BuildValue("i", topic.data_clock);
      PyDict_SetItemString(topicDict, "dclock", val); Py_DECREF(val);

      val = Py_BuildValue("i", topic.data_frequency);
      PyDict_SetItemString(topicDict, "dfreq", val); Py_DECREF(val);

      // TODO: std::map<std::string, std::string> attr
    }
  }
}

PyObject* mon_monitoring(PyObject* /*self*/, PyObject* /*args*/)
{
  PyObject* retDict = PyDict_New();

  eCAL::Monitoring::SMonitoring monitoring;
  if (eCAL::Monitoring::GetMonitoring(monitoring))
  {
    PyObject* val;

    // collect process infos
    {
      PyObject* processList = PyList_New(0);
      PyDict_SetItemString(retDict, "processes", processList); Py_DECREF(processList);

      for (const auto& process : monitoring.processes)
      {
        PyObject* processDict = PyDict_New();
        PyList_Append(processList, processDict); Py_DECREF(processDict);

        val = Py_BuildValue("i", process.registration_clock);
        PyDict_SetItemString(processDict, "rclock", val); Py_DECREF(val);

        val = Py_BuildValue("s", process.host_name.c_str());
        PyDict_SetItemString(processDict, "hname", val); Py_DECREF(val);

        val = Py_BuildValue("s", process.shm_transport_domain.c_str());
        PyDict_SetItemString(processDict, "shm_transport_domain", val); Py_DECREF(val);

        val = Py_BuildValue("i", process.process_id);
        PyDict_SetItemString(processDict, "pid", val); Py_DECREF(val);

        val = Py_BuildValue("s", process.process_name.c_str());
        PyDict_SetItemString(processDict, "pname", val); Py_DECREF(val);

        val = Py_BuildValue("s", process.unit_name.c_str());
        PyDict_SetItemString(processDict, "uname", val); Py_DECREF(val);

        val = Py_BuildValue("s", process.process_parameter.c_str());
        PyDict_SetItemString(processDict, "pparam", val); Py_DECREF(val);

        val = Py_BuildValue("i", process.state_severity);
        PyDict_SetItemString(processDict, "state_severity", val); Py_DECREF(val);

        val = Py_BuildValue("i", process.state_severity_level);
        PyDict_SetItemString(processDict, "state_severity_level", val); Py_DECREF(val);

        val = Py_BuildValue("s", process.state_info.c_str());
        PyDict_SetItemString(processDict, "state_info", val); Py_DECREF(val);

        val = Py_BuildValue("i", process.time_sync_state);
        PyDict_SetItemString(processDict, "tsync_state", val); Py_DECREF(val);

        val = Py_BuildValue("s", process.time_sync_module_name.c_str());
        PyDict_SetItemString(processDict, "tsync_mod_name", val); Py_DECREF(val);

        val = Py_BuildValue("i", process.component_init_state);
        PyDict_SetItemString(processDict, "component_init_state", val); Py_DECREF(val);

        val = Py_BuildValue("s", process.component_init_info.c_str());
        PyDict_SetItemString(processDict, "component_init_info", val); Py_DECREF(val);

        val = Py_BuildValue("s", process.ecal_runtime_version.c_str());
        PyDict_SetItemString(processDict, "ecal_runtime_version", val); Py_DECREF(val);
      }
    }

    // collect service infos
    {
      PyObject* serviceList = PyList_New(0);
      PyDict_SetItemString(retDict, "services", serviceList); Py_DECREF(serviceList);

      for (const auto& service : monitoring.server)
      {
        PyObject* serviceDict = PyDict_New();
        PyList_Append(serviceList, serviceDict); Py_DECREF(serviceDict);

        val = Py_BuildValue("i", service.registration_clock);
        PyDict_SetItemString(serviceDict, "rclock", val); Py_DECREF(val);

        val = Py_BuildValue("s", service.host_name.c_str());
        PyDict_SetItemString(serviceDict, "hname", val); Py_DECREF(val);

        val = Py_BuildValue("s", service.process_name.c_str());
        PyDict_SetItemString(serviceDict, "pname", val); Py_DECREF(val);

        val = Py_BuildValue("s", service.unit_name.c_str());
        PyDict_SetItemString(serviceDict, "uname", val); Py_DECREF(val);

        val = Py_BuildValue("i", service.process_id);
        PyDict_SetItemString(serviceDict, "pid", val); Py_DECREF(val);

        val = Py_BuildValue("s", service.service_name.c_str());
        PyDict_SetItemString(serviceDict, "sname", val); Py_DECREF(val);

        val = Py_BuildValue("s", std::to_string(service.service_id).c_str());
        PyDict_SetItemString(serviceDict, "sid", val); Py_DECREF(val);

        val = Py_BuildValue("i", service.version);
        PyDict_SetItemString(serviceDict, "version", val); Py_DECREF(val);

        val = Py_BuildValue("i", service.tcp_port_v0);
        PyDict_SetItemString(serviceDict, "tcp_port_v0", val); Py_DECREF(val);

        val = Py_BuildValue("i", service.tcp_port_v1);
        PyDict_SetItemString(serviceDict, "tcp_port_v1", val); Py_DECREF(val);

        PyObject* methodsDict = PyDict_New();
        PyDict_SetItemString(serviceDict, "methods", methodsDict); Py_DECREF(methodsDict);

        for (const auto method : service.methods)
        {
          val = Py_BuildValue("s", method.method_name.c_str());
          PyDict_SetItemString(methodsDict, "mname", val); Py_DECREF(val);

          val = Py_BuildValue("s", method.request_datatype_information.name.c_str());
          PyDict_SetItemString(methodsDict, "request_datatype_name", val); Py_DECREF(val);

          val = Py_BuildValue("s", method.request_datatype_information.encoding.c_str());
          PyDict_SetItemString(methodsDict, "request_datatype_encoding", val); Py_DECREF(val);

          val = Py_BuildValue("y#", method.request_datatype_information.descriptor.c_str(), method.request_datatype_information.descriptor.length());
          PyDict_SetItemString(methodsDict, "request_datatype_descriptor", val); Py_DECREF(val);

          val = Py_BuildValue("s", method.response_datatype_information.name.c_str());
          PyDict_SetItemString(methodsDict, "response_datatype_name", val); Py_DECREF(val);

          val = Py_BuildValue("s", method.response_datatype_information.encoding.c_str());
          PyDict_SetItemString(methodsDict, "response_datatype_encoding", val); Py_DECREF(val);

          val = Py_BuildValue("y#", method.response_datatype_information.descriptor.c_str(), method.response_datatype_information.descriptor.length());
          PyDict_SetItemString(methodsDict, "response_datatype_descriptor", val); Py_DECREF(val);

          val = Py_BuildValue("i", method.call_count);
          PyDict_SetItemString(methodsDict, "call_count", val); Py_DECREF(val);
        }
      }
    }

    // collect client infos
    {
      PyObject* clientList = PyList_New(0);
      PyDict_SetItemString(retDict, "clients", clientList); Py_DECREF(clientList);

      for (const auto& client : monitoring.clients)
      {
        PyObject* clientDict = PyDict_New();
        PyList_Append(clientList, clientDict); Py_DECREF(clientDict);

        val = Py_BuildValue("i", client.registration_clock);
        PyDict_SetItemString(clientDict, "rclock", val); Py_DECREF(val);

        val = Py_BuildValue("s", client.host_name.c_str());
        PyDict_SetItemString(clientDict, "hname", val); Py_DECREF(val);

        val = Py_BuildValue("s", client.process_name.c_str());
        PyDict_SetItemString(clientDict, "pname", val); Py_DECREF(val);

        val = Py_BuildValue("s", client.unit_name.c_str());
        PyDict_SetItemString(clientDict, "uname", val); Py_DECREF(val);

        val = Py_BuildValue("i", client.process_id);
        PyDict_SetItemString(clientDict, "pid", val); Py_DECREF(val);

        val = Py_BuildValue("s", client.service_name.c_str());
        PyDict_SetItemString(clientDict, "sname", val); Py_DECREF(val);

        val = Py_BuildValue("s", std::to_string(client.service_id).c_str());
        PyDict_SetItemString(clientDict, "sid", val); Py_DECREF(val);

        PyObject* methodsDict = PyDict_New();
        PyDict_SetItemString(clientDict, "methods", methodsDict); Py_DECREF(methodsDict);

        for (const auto method : client.methods)
        {
          val = Py_BuildValue("s", method.method_name.c_str());
          PyDict_SetItemString(methodsDict, "mname", val); Py_DECREF(val);

          val = Py_BuildValue("s", method.request_datatype_information.name.c_str());
          PyDict_SetItemString(methodsDict, "request_datatype_type", val); Py_DECREF(val);

          val = Py_BuildValue("s", method.request_datatype_information.encoding.c_str());
          PyDict_SetItemString(methodsDict, "request_datatype_encoding", val); Py_DECREF(val);

          val = Py_BuildValue("y#", method.request_datatype_information.descriptor.c_str(), method.request_datatype_information.descriptor.length());
          PyDict_SetItemString(methodsDict, "request_datatype_descriptor", val); Py_DECREF(val);

          val = Py_BuildValue("s", method.response_datatype_information.name.c_str());
          PyDict_SetItemString(methodsDict, "response_datatype_name", val); Py_DECREF(val);

          val = Py_BuildValue("s", method.response_datatype_information.encoding.c_str());
          PyDict_SetItemString(methodsDict, "response_datatype_encoding", val); Py_DECREF(val);

          val = Py_BuildValue("y#", method.response_datatype_information.descriptor.c_str(), method.response_datatype_information.descriptor.length());
          PyDict_SetItemString(methodsDict, "response_datatype_descriptor", val); Py_DECREF(val);

          val = Py_BuildValue("i", method.call_count);
          PyDict_SetItemString(methodsDict, "call_count", val); Py_DECREF(val);
        }

        val = Py_BuildValue("i", client.version);
        PyDict_SetItemString(clientDict, "version", val); Py_DECREF(val);
      }
    }

    // collect topic infos
    {
      PyObject* topicList = PyList_New(0);
      PyDict_SetItemString(retDict, "topics", topicList); Py_DECREF(topicList);
      // read out publisher
      PopulateTopics2List(topicList, monitoring.publisher);
      // read out subscriber
      PopulateTopics2List(topicList, monitoring.subscriber);
    }
  }

  auto* retVal = Py_BuildValue("iO", 0, retDict); Py_DECREF(retDict);
  return(retVal);
}

/****************************************/
/*      mon_logging                     */
/****************************************/
PyObject* mon_logging(PyObject* /*self*/, PyObject* /*args*/)
{
  PyObject* retList = PyList_New(0);

  eCAL::Logging::SLogging logging;
  if (eCAL::Logging::GetLogging(logging))
  {
    for (const auto& log : logging.log_messages)
    {
      PyObject* logDict = PyDict_New();
      PyList_Append(retList, logDict); Py_DECREF(logDict);

      PyObject* val;

      val = Py_BuildValue("L", log.time);
      PyDict_SetItemString(logDict, "time", val); Py_DECREF(val);

      val = Py_BuildValue("s", log.host_name.c_str());
      PyDict_SetItemString(logDict, "hname", val); Py_DECREF(val);

      val = Py_BuildValue("i", log.process_id);
      PyDict_SetItemString(logDict, "pid", val); Py_DECREF(val);

      val = Py_BuildValue("s", log.process_name.c_str());
      PyDict_SetItemString(logDict, "pname", val); Py_DECREF(val);

      val = Py_BuildValue("i", log.level);
      PyDict_SetItemString(logDict, "level", val); Py_DECREF(val);

      val = Py_BuildValue("s", log.content.c_str());
      PyDict_SetItemString(logDict, "content", val); Py_DECREF(val);
    }
  }

  auto* retVal = Py_BuildValue("iO", 0, retList); Py_DECREF(retList);
  return(retVal);
}


/****************************************/
/*                                      */
/*              METHODS                 */
/*                                      */
/****************************************/
static PyMethodDef _ecal_methods[] = 
{
  {"initialize",                    initialize,                    METH_VARARGS,  "initialize(unit_name)"},
  {"finalize",                      finalize,                      METH_NOARGS,   "finalize()"},
  {"is_initialized",                is_initialized,                METH_NOARGS,   "is_initialized()"},

  {"getversion",                    getversion,                    METH_NOARGS,   "getversion()"},
  {"getversion_components",         getversion_components,         METH_NOARGS,   "getversion_components()"},
  {"getdate",                       getdate,                       METH_NOARGS,   "getdate()"},
  {"getmicroseconds",               getmicroseconds,               METH_NOARGS,   "getmicroseconds()"},

  {"set_process_state",             set_process_state,             METH_VARARGS,  "set_process_state(severity, level, info)"},
  {"ok",                            ok,                            METH_NOARGS,   "ok()"},
  {"shutdown_process_uname",        shutdown_process_uname,        METH_VARARGS,  "shutdown_process_uname(unit_name)"},
  {"shutdown_processes",            shutdown_processes,            METH_NOARGS,   "shutdown_processes()"},

  {"log_setlevel",                  log_setlevel,                  METH_VARARGS,  "log_setlevel(level)"},
  {"log_message",                   log_message,                   METH_VARARGS,  "log_message(message)"},

  {"pub_create",                    pub_create,                    METH_VARARGS,  "pub_create(topic_name, topic_type_name, topic_type_encoding, topic_type_desc)"},
  {"pub_destroy",                   pub_destroy,                   METH_VARARGS,  "pub_destroy(topic_handle)"},

  {"pub_send",                      pub_send,                      METH_VARARGS,  "pub_send(topic_handle, payload, time)"},

  {"sub_create",                    sub_create,                    METH_VARARGS,  "sub_create(topic_name, topic_type_name, topic_type_encoding, topic_type_desc)"},
  {"sub_destroy",                   sub_destroy,                   METH_VARARGS,  "sub_destroy(topic_handle)"},

  {"sub_receive",                   sub_receive,                   METH_VARARGS,  "sub_receive(topic_handle, timeout)"},

  {"sub_set_callback",              sub_set_callback,              METH_VARARGS,  "sub_set_callback(topic_handle, callback)"},
  {"sub_rem_callback",              sub_rem_callback,              METH_VARARGS,  "sub_rem_callback(topic_handle, callback)"},

  {"server_create",                 server_create,                 METH_VARARGS,  "server_create(service_name)" },
  {"server_destroy",                server_destroy,                METH_VARARGS,  "server_destroy(server_handle)" },

  {"server_add_method_callback",    server_add_method_callback,    METH_VARARGS,  "server_add_method_callback(server_handle, method_name, request_type_name, response_type_name, callback)" },
  {"server_rem_method_callback",    server_rem_method_callback,    METH_VARARGS,  "server_rem_method_callback(server_handle, method_name)" },

  {"client_create",                 client_create,                 METH_VARARGS,  "client_create(service_name)" },
  {"client_destroy",                client_destroy,                METH_VARARGS,  "client_destroy(client_handle)" },

  {"client_set_hostname",           client_set_hostname,           METH_VARARGS,  "client_set_hostname(client_handle, host_name)" },
  {"client_call_method",            client_call_method,            METH_VARARGS,  "client_call_method(client_handle, method_name, request, timeout)" },

  {"client_add_response_callback",  client_add_response_callback,  METH_VARARGS,  "client_add_response_callback(client_handle, callback)" },
  {"client_rem_response_callback",  client_rem_response_callback,  METH_VARARGS,  "client_rem_response_callback(client_handle)" },
  
  {"mon_initialize",                mon_initialize,                METH_NOARGS,   "mon_initialize()"},
  {"mon_finalize",                  mon_finalize,                  METH_NOARGS,   "mon_finalize()"},

  {"mon_monitoring",                mon_monitoring,                METH_NOARGS,   "mon_monitoring()"},
  {"mon_logging",                   mon_logging,                   METH_NOARGS,   "mon_logging()"},

  {nullptr, nullptr, 0, ""}
};


/****************************************/
/*                                      */
/*               INIT                   */
/*                                      */
/****************************************/
struct module_state {
  PyObject *error;
};

#define GETSTATE(m) ((struct module_state*)PyModule_GetState(m))

static int _ecal_traverse(PyObject *m, visitproc visit, void *arg) {
  Py_VISIT(GETSTATE(m)->error);
  return 0;
}

static int _ecal_clear(PyObject *m) {
  Py_CLEAR(GETSTATE(m)->error);
  return 0;
}

static struct PyModuleDef moduledef = {
  PyModuleDef_HEAD_INIT,
  "_ecal_core_py",
  nullptr,
  sizeof(struct module_state),
  _ecal_methods,
  nullptr,
  _ecal_traverse,
  _ecal_clear,
  nullptr
};

PyMODINIT_FUNC PyInit__ecal_core_py(void)
{
  PyObject *module = PyModule_Create(&moduledef);

  if (module == nullptr)
    return nullptr;
  struct module_state *st = GETSTATE(module);
  
  char err_msg[] = "_ecal_core_py.Error";
  st->error = PyErr_NewException(err_msg, nullptr, nullptr);
  if (st->error == nullptr) {
    Py_DECREF(module);
    return nullptr;
  }

  return module;
}
