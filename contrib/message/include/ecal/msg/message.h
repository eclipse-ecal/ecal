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

#pragma once

/*
A type needs to implement the following functions in order that a measurement object
can be constructed for that type (read / write)
*/
namespace eCAL
{
  namespace message
  {
    //// unfortunately, we need an actual object for this :/
    //template<typename T>
    //std::string GetTypeName(const T& message);
  
    //// unfortunately, we need an actual object for this :/
    //template<typename T>
    //std::string GetDescription(const T& message);
  
    //template<typename T>
    //bool Serialize(const T& message, std::string& buffer);
  
    //template<typename T>
    //bool Deserialize(const std::string& buffer, T& message);
  }
}