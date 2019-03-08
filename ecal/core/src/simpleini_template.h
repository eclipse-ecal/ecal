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

#include <simpleini.h>
#include <string>
#include <utility>
#include <cstdint>

/*
* This file provides two methods to access the Simple Inifile Setter/Getters on a template basis.
*/

template<typename T>
T GetValue(const CSimpleIni& ini_, const std::string& section_, const std::string& key_, const T& default_);
template<typename T>
SI_Error SetValue(CSimpleIni& ini_, const std::string& section_, const std::string& key_, const T& value_, const std::string& comment_, bool force_overwrite_);

