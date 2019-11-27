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
* @brief  eCALHDF5 python interface
**/

#include "Python.h"
#include "modsupport.h"
#include "structmember.h"

#include <ecalhdf5/eh5_meas.h>
#include <ecalhdf5/eh5_types.h>

#ifdef __GNUC__
#pragma GCC diagnostic ignored "-Wstrict-aliasing"
#endif

/****************************************/
/*      HDF5 measurement                */
/****************************************/
typedef struct
{
  PyObject_HEAD
  eCAL::eh5::HDF5Meas *hdf5_meas;
} Meas;

/****************************************/
/*      Meas new                        */
/****************************************/
static PyObject* Meas_New(PyTypeObject *type, PyObject* /*args*/, PyObject* /*kwds*/)
{
  Meas *self;

  self = (Meas *)type->tp_alloc(type, 0);
  if (self != NULL)
  {
    self->hdf5_meas = new eCAL::eh5::HDF5Meas();
  }

  return (PyObject *)self;
}

/****************************************/
/*      Meas dealloc                    */
/****************************************/
static void Meas_Dealloc(Meas *self)
{
  if (self->hdf5_meas != nullptr)
  {
    delete self->hdf5_meas;
    self->hdf5_meas = nullptr;
  }

  Py_TYPE(self)->tp_free((PyObject*)self);
}

/****************************************/
/*      Meas members                    */
/****************************************/
#ifdef __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wwrite-strings"
#endif /* __GNUC__ */
static PyMemberDef Meas_Members[] =
{
  { "hdf5_meas", T_OBJECT, offsetof(Meas, hdf5_meas), 0, "hdf5 meas" },
  { nullptr, 0, 0, 0, nullptr }
};
#ifdef __GNUC__
#pragma GCC diagnostic pop
#endif /* __GNUC__ */

/****************************************/
/*      Meas methods                    */
/****************************************/

/****************************************/
/*      Open                            */
/****************************************/
static PyObject* Meas_Open(Meas *self, PyObject *args)
{
  char* path = nullptr;
  size_t access = 0;

  if (!PyArg_ParseTuple(args, "s|i", &path, &access))
    return nullptr;

  bool open_meas = false;
  switch (access)
  {
  case 0:
    open_meas = self->hdf5_meas->Open(path, eCAL::eh5::eAccessType::RDONLY);
    break;

  case 1:
    open_meas = self->hdf5_meas->Open(path, eCAL::eh5::eAccessType::CREATE);
    break;

  default:
    break;
  }
  
  return(Py_BuildValue("i", open_meas));
}

/****************************************/
/*      Close                           */
/****************************************/
static PyObject* Meas_Close(Meas *self, PyObject* /*args*/)
{
  return(Py_BuildValue("i", self->hdf5_meas->Close()));
}

/****************************************/
/*      IsOk                            */
/****************************************/
static PyObject* Meas_IsOk(Meas *self, PyObject* /*args*/)
{
  return(Py_BuildValue("i", self->hdf5_meas->IsOk()));
}

/****************************************/
/*      GetFileVersion                  */
/****************************************/
static PyObject* Meas_GetFileVersion(Meas *self, PyObject* /*args*/)
{
  return(Py_BuildValue("s", self->hdf5_meas->GetFileVersion().c_str()));
}

/****************************************/
/*      GetMaxSizePerFile               */
/****************************************/
static PyObject* Meas_GetMaxSizePerFile(Meas *self, PyObject* /*args*/)
{
  return(Py_BuildValue("i", self->hdf5_meas->GetMaxSizePerFile()));
}

/****************************************/
/*      SetMaxSizePerFile               */
/****************************************/
static PyObject* Meas_SetMaxSizePerFile(Meas *self, PyObject *args)
{
  size_t size = 0;

  if (!PyArg_ParseTuple(args, "i", &size))
    return nullptr;

  self->hdf5_meas->SetMaxSizePerFile(size);
  Py_RETURN_NONE;
}

/****************************************/
/*      GetChannelNames                 */
/****************************************/
static PyObject* Meas_GetChannelNames(Meas *self, PyObject* /*args*/)
{
  auto channel_names = self->hdf5_meas->GetChannelNames();
  PyObject* channels = PyList_New(0);

  for (const auto& channel : channel_names)
  {
    PyObject* ch = Py_BuildValue("s", channel.c_str());
    PyList_Append(channels, ch);
  }

  return channels;
}

/****************************************/
/*      GetChannelDescription           */
/****************************************/
static PyObject* Meas_GetChannelDescription(Meas *self, PyObject *args)
{
  char* channel_name = nullptr;

  if (!PyArg_ParseTuple(args, "s", &channel_name))
    return nullptr;

  std::string description = self->hdf5_meas->GetChannelDescription(channel_name);

  return(Py_BuildValue("y#", description.c_str(), description.size()));
}

/****************************************/
/*      SetChannelDescription           */
/****************************************/
static PyObject* Meas_SetChannelDescription(Meas *self, PyObject *args)
{
  char* channel_name = nullptr;
  char* description = nullptr;
  int size = 0;

  if (!PyArg_ParseTuple(args, "sy#", &channel_name, &description, &size))
    return nullptr;

  self->hdf5_meas->SetChannelDescription(channel_name, std::string(description, size));
  Py_RETURN_NONE;
}

/****************************************/
/*      GetChannelType                  */
/****************************************/
static PyObject* Meas_GetChannelType(Meas *self, PyObject *args)
{
  char* channel_name = nullptr;

  if (!PyArg_ParseTuple(args, "s", &channel_name))
    return nullptr;

  return(Py_BuildValue("s", self->hdf5_meas->GetChannelType(channel_name).c_str()));
}

/****************************************/
/*      SetChannelType                  */
/****************************************/
static PyObject* Meas_SetChannelType(Meas *self, PyObject *args)
{
  char* channel_name = nullptr;
  char* type = nullptr;

  if (!PyArg_ParseTuple(args, "ss", &channel_name, &type))
    return nullptr;

  self->hdf5_meas->SetChannelType(channel_name, type);
  Py_RETURN_NONE;
}

/****************************************/
/*      GetMinTimestamp                 */
/****************************************/
static PyObject* Meas_GetMinTimestamp(Meas *self, PyObject *args)
{
  char* channel_name = nullptr;

  if (!PyArg_ParseTuple(args, "s", &channel_name))
    return nullptr;

  return(Py_BuildValue("L", self->hdf5_meas->GetMinTimestamp(channel_name)));
}

/****************************************/
/*      GetMaxTimestamp                 */
/****************************************/
static PyObject* Meas_GetMaxTimestamp(Meas *self, PyObject *args)
{
  char* channel_name = nullptr;

  if (!PyArg_ParseTuple(args, "s", &channel_name))
    return nullptr;

  return(Py_BuildValue("L", self->hdf5_meas->GetMaxTimestamp(channel_name)));
}

/****************************************/
/*      GetEntriesInfo                  */
/****************************************/
static PyObject* Meas_GetEntriesInfo(Meas *self, PyObject *args)
{
  char* channel_name = nullptr;

  if (!PyArg_ParseTuple(args, "s", &channel_name))
    return nullptr;

  eCAL::eh5::EntryInfoSet entries;
  self->hdf5_meas->GetEntriesInfo(channel_name, entries);
  PyObject* entries_info = PyList_New(0);

  for (const auto& entry : entries)
  {
    PyObject* dict = PyDict_New();
    PyList_Append(entries_info, dict);
    Py_DECREF(dict);

    PyObject* val;

    val = Py_BuildValue("L", entry.SndTimestamp);
    PyDict_SetItemString(dict, "snd_timestamp", val);
    Py_DECREF(val);

    val = Py_BuildValue("L", entry.RcvTimestamp);
    PyDict_SetItemString(dict, "rcv_timestamp", val);
    Py_DECREF(val);

    val = Py_BuildValue("L", entry.ID);
    PyDict_SetItemString(dict, "id", val);
    Py_DECREF(val);

    val = Py_BuildValue("L", entry.SndClock);
    PyDict_SetItemString(dict, "snd_clock", val);
    Py_DECREF(val);

    val = Py_BuildValue("L", entry.SndID);
    PyDict_SetItemString(dict, "snd_id", val);
    Py_DECREF(val);
  }

  return entries_info;
}

/****************************************/
/*      GetEntriesInfoRange             */
/****************************************/
static PyObject* Meas_GetEntriesInfoRange(Meas *self, PyObject *args)
{
  char* channel_name = nullptr;
  long long begin = 0;
  long long end = 0;

  if (!PyArg_ParseTuple(args, "sLL", &channel_name, &begin, &end))
    return nullptr;

  eCAL::eh5::EntryInfoSet entries;
  self->hdf5_meas->GetEntriesInfoRange(channel_name, begin, end, entries);
  PyObject* entries_info = PyList_New(0);

  for (const auto& entry : entries)
  {
    PyObject* dict = PyDict_New();
    PyList_Append(entries_info, dict);
    Py_DECREF(dict);

    PyObject* val;

    val = Py_BuildValue("L", entry.SndTimestamp);
    PyDict_SetItemString(dict, "snd_timestamp", val);
    Py_DECREF(val);

    val = Py_BuildValue("L", entry.RcvTimestamp);
    PyDict_SetItemString(dict, "rcv_timestamp", val);
    Py_DECREF(val);

    val = Py_BuildValue("L", entry.ID);
    PyDict_SetItemString(dict, "id", val);
    Py_DECREF(val);

    val = Py_BuildValue("L", entry.SndClock);
    PyDict_SetItemString(dict, "snd_clock", val);
    Py_DECREF(val);

    val = Py_BuildValue("L", entry.SndID);
    PyDict_SetItemString(dict, "snd_id", val);
    Py_DECREF(val);
  }

  return entries_info;
}

/****************************************/
/*      GetEntryDataSize                */
/****************************************/
static PyObject* Meas_GetEntryDataSize(Meas *self, PyObject *args)
{
  long long entry_id = 0;

  if (!PyArg_ParseTuple(args, "L", &entry_id))
    return nullptr;

  size_t size = 0;
  self->hdf5_meas->GetEntryDataSize(entry_id, size);
  return(Py_BuildValue("i", size));
}

/****************************************/
/*      GetEntryData                    */
/****************************************/
static PyObject* Meas_GetEntryData(Meas *self, PyObject *args)
{
  long long entry_id = 0;

  if (!PyArg_ParseTuple(args, "L", &entry_id))
    return nullptr;

  size_t data_size = 0;
  self->hdf5_meas->GetEntryDataSize(entry_id, data_size);
  auto data = calloc(data_size, sizeof(char));
  self->hdf5_meas->GetEntryData(entry_id, data);

  PyObject* py_data;

  py_data = Py_BuildValue("y#", data, data_size);

  free(data);
  return py_data;
}

/****************************************/
/*      SetFileBaseName                 */
/****************************************/
static PyObject* Meas_SetFileBaseName(Meas *self, PyObject *args)
{
  char* base_name = nullptr;

  if (!PyArg_ParseTuple(args, "s", &base_name))
    return nullptr;

  self->hdf5_meas->SetFileBaseName(base_name);
  Py_RETURN_NONE;
}

/****************************************/
/*      AddEntryToFile                  */
/****************************************/
static PyObject* Meas_AddEntryToFile(Meas *self, PyObject *args)
{
  char*     data(nullptr);
  int       size(0);
  long long snd_timestamp(0);
  long long rcv_timestamp(0);
  char*     channel_name(nullptr);
  long long counter(0);

  if (!PyArg_ParseTuple(args, "y#LLs|L", &data, &size, &snd_timestamp, &rcv_timestamp, &channel_name, &counter))
    return nullptr;

  return(Py_BuildValue("i", self->hdf5_meas->AddEntryToFile(data, size, snd_timestamp, rcv_timestamp, channel_name, 0, counter)));
}

/****************************************/
/*      ConnectPreSplitCallback         */
/****************************************/

/****************************************/
/*      DisconnectPreSplitCallback      */
/****************************************/


/****************************************/
/*      Meas methods definition         */
/****************************************/
static PyMethodDef Meas_Methods[] = 
{
  {"open",                    (PyCFunction)Meas_Open,                   METH_VARARGS, "open(path, access)"},
  {"close",                   (PyCFunction)Meas_Close,                  METH_NOARGS,  "close()"},
  {"is_ok",                   (PyCFunction)Meas_IsOk,                   METH_NOARGS,  "is_ok()" },

  {"get_file_version",        (PyCFunction)Meas_GetFileVersion,         METH_NOARGS,  "get_file_version()"},

  {"get_max_size_per_file",   (PyCFunction)Meas_GetMaxSizePerFile,      METH_NOARGS,  "get_max_size_per_file()"},
  {"set_max_size_per_file",   (PyCFunction)Meas_SetMaxSizePerFile,      METH_VARARGS, "set_max_size_per_file(size)"},

  {"get_channel_names",       (PyCFunction)Meas_GetChannelNames,        METH_NOARGS,  "get_channel_names()"},

  {"get_channel_description", (PyCFunction)Meas_GetChannelDescription,  METH_VARARGS, "get_channel_description(channel_name)"},
  {"set_channel_description", (PyCFunction)Meas_SetChannelDescription,  METH_VARARGS, "set_channel_description(channel_name, description)"},

  {"get_channel_type",        (PyCFunction)Meas_GetChannelType,         METH_VARARGS, "get_channel_type(channel_name)"},
  {"set_channel_type",        (PyCFunction)Meas_SetChannelType,         METH_VARARGS, "set_channel_type(channel_name, type)"},
  
  {"get_min_timestamp",       (PyCFunction)Meas_GetMinTimestamp,        METH_VARARGS, "get_min_timestamp(channel_name)"},
  {"get_max_timestamp",       (PyCFunction)Meas_GetMaxTimestamp,        METH_VARARGS, "get_max_timestamp(channel_name)"},

  {"get_entries_info",        (PyCFunction)Meas_GetEntriesInfo,         METH_VARARGS, "get_entries_info(channel_name)"},
  {"get_entries_info_range",  (PyCFunction)Meas_GetEntriesInfoRange,    METH_VARARGS, "get_entries_info_range(channel_name, begin, end)"},
  {"get_entry_data_size",     (PyCFunction)Meas_GetEntryDataSize,       METH_VARARGS, "get_entry_data_size(entry_id)"},
  {"get_entry_data",          (PyCFunction)Meas_GetEntryData,           METH_VARARGS, "get_entry_data(entry_id)"},

  {"set_file_base_name",      (PyCFunction)Meas_SetFileBaseName,        METH_VARARGS, "set_file_base_name(base_name)"},

  {"add_entry_to_file",       (PyCFunction)Meas_AddEntryToFile,         METH_VARARGS, "add_entry_to_file(data, size, timestamp, channel_name, counter)"},

  { nullptr, nullptr, 0, nullptr }
};

/****************************************/
/*      Meas type definition            */
/****************************************/
static PyTypeObject MeasType = {
  PyVarObject_HEAD_INIT(NULL, 0)
  "_ecal_hdf5_py.Meas",                     /*tp_name*/
  sizeof(Meas),                             /*tp_basicsize*/
  0,                                        /*tp_itemsize*/
  (destructor)Meas_Dealloc,                 /*tp_dealloc*/
  0,                                        /*tp_print*/
  0,                                        /*tp_getattr*/
  0,                                        /*tp_setattr*/
  0,                                        /*tp_compare*/
  0,                                        /*tp_repr*/
  0,                                        /*tp_as_number*/
  0,                                        /*tp_as_sequence*/
  0,                                        /*tp_as_mapping*/
  0,                                        /*tp_hash */
  0,                                        /*tp_call*/
  0,                                        /*tp_str*/
  0,                                        /*tp_getattro*/
  0,                                        /*tp_setattro*/
  0,                                        /*tp_as_buffer*/
  Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE, /*tp_flags*/
  "Meas objects",                           /* tp_doc */
  0,                                        /* tp_traverse */
  0,                                        /* tp_clear */
  0,                                        /* tp_richcompare */
  0,                                        /* tp_weaklistoffset */
  0,                                        /* tp_iter */
  0,                                        /* tp_iternext */
  Meas_Methods,                             /* tp_methods */
  Meas_Members,                             /* tp_members */
  0,                                        /* tp_getset */
  0,                                        /* tp_base */
  0,                                        /* tp_dict */
  0,                                        /* tp_descr_get */
  0,                                        /* tp_descr_set */
  0,                                        /* tp_dictoffset */
  0,                                        /* tp_init */
  0,                                        /* tp_alloc */
  Meas_New,                                 /* tp_new */
  0,                                        /* tp_free */
  0,                                        /* tp_is_gc */
  0,                                        /* tp_bases */
  0,                                        /* tp_mro */
  0,                                        /* tp_cache */
  0,                                        /* tp_subclasses */
  0,                                        /* tp_weaklist */
  0,                                        /* tp_del */
  0,                                        /* tp_version_tag */
  0                                         /* tp_finalize */
};

static PyMethodDef module_methods[] = {
  { nullptr, nullptr, 0, "" }
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

static int _ecal_hdf5_traverse(PyObject *m, visitproc visit, void *arg) {
  Py_VISIT(GETSTATE(m)->error);
  return 0;
}

static int _ecal_hdf5_clear(PyObject *m) {
  Py_CLEAR(GETSTATE(m)->error);
  return 0;
}

static struct PyModuleDef moduledef = {
  PyModuleDef_HEAD_INIT,
  "_ecal_hdf5_py",
  nullptr,
  sizeof(struct module_state),
  module_methods,
  nullptr,
  _ecal_hdf5_traverse,
  _ecal_hdf5_clear,
  nullptr
};

PyMODINIT_FUNC PyInit__ecal_hdf5_py(void)
{
  if (PyType_Ready(&MeasType) < 0)
    return nullptr;

  PyObject *module = PyModule_Create(&moduledef);

  if (PyType_Ready(&MeasType) < 0)
    return nullptr;
  
  Py_INCREF(&MeasType); //-V1027
  PyModule_AddObject(module, "Meas", (PyObject *)&MeasType); //-V1027
  
  if (module == nullptr)
    return nullptr;
  struct module_state *st = GETSTATE(module);
  
  char err_msg[] = "_ecal_hdf5_py.Error";
  st->error = PyErr_NewException(err_msg, nullptr, nullptr);
  if (st->error == nullptr) 
  {
    Py_DECREF(module);
    return nullptr;
  }

  return module;
}
