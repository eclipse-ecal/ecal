
.. _program_listing_file_ecal_core_include_ecal_msg_dynamic.h:

Program Listing for File dynamic.h
==================================

|exhale_lsh| :ref:`Return to documentation for file <file_ecal_core_include_ecal_msg_dynamic.h>` (``ecal/core/include/ecal/msg/dynamic.h``)

.. |exhale_lsh| unicode:: U+021B0 .. UPWARDS ARROW WITH TIP LEFTWARDS

.. code-block:: cpp

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
   
   #include <string>
   #include <exception>
   
   namespace eCAL
   {
     
     /* @cond */
     class DynamicReflectionException : public std::exception
     {
     public:
       DynamicReflectionException(const std::string& message) : message_(message) {}
       virtual const char* what() const throw() { return message_.c_str(); }
     private:
       std::string message_;
     };
     /* @endcond */
     
     /* @cond */
     inline bool StrEmptyOrNull(const std::string& str)
     {
       if (str.empty())
       {
         return true;
       }
       else
       {
         for (auto c : str)
         {
           if (c != '\0')
           {
             return false;
           }
         }
         return true;
       }
     }
     /* @endcond */
   }
