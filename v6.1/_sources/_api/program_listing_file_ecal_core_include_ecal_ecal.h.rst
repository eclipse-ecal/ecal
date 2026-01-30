
.. _program_listing_file_ecal_core_include_ecal_ecal.h:

Program Listing for File ecal.h
===============================

|exhale_lsh| :ref:`Return to documentation for file <file_ecal_core_include_ecal_ecal.h>` (``ecal/core/include/ecal/ecal.h``)

.. |exhale_lsh| unicode:: U+021B0 .. UPWARDS ARROW WITH TIP LEFTWARDS

.. code-block:: cpp

   /* ========================= eCAL LICENSE =================================
    *
    * Copyright (C) 2016 - 2025 Continental Corporation
    * Copyright 2025 AUMOVIO and subsidiaries. All rights reserved.
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
   
   // all ecal includes
   // IWYU pragma: begin_exports
   #include <ecal/os.h>
   #include <ecal/defs.h>
   #include <ecal/service/client.h>
   #include <ecal/config.h>
   #include <ecal/core.h>
   #include <ecal/log.h>
   #include <ecal/process.h>
   #include <ecal/monitoring.h>
   #include <ecal/registration.h>
   #include <ecal/service/server.h>
   #include <ecal/service/types.h>
   #include <ecal/time.h>
   #include <ecal/timer.h>
   #include <ecal/util.h>
   #include <ecal/config/configuration.h>
   #include <ecal/pubsub/publisher.h>
   #include <ecal/pubsub/subscriber.h>
   #include <ecal/service/client.h>
   #include <ecal/service/server.h>
   // IWYU pragma: end_exports
