///* ========================= eCAL LICENSE =================================
// *
// * Copyright (C) 2016 - 2025 Continental Corporation
// *
// * Licensed under the Apache License, Version 2.0 (the "License");
// * you may not use this file except in compliance with the License.
// * You may obtain a copy of the License at
// * 
// *      http://www.apache.org/licenses/LICENSE-2.0
// * 
// * Unless required by applicable law or agreed to in writing, software
// * distributed under the License is distributed on an "AS IS" BASIS,
// * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// * See the License for the specific language governing permissions and
// * limitations under the License.
// *
// * ========================= eCAL LICENSE =================================
//*/
//
///**
// * @file   util.h
// * @brief  eCAL utility interface
//**/
//
//#pragma once
//
//#include <ecal/os.h>
//#include <ecal/deprecate.h>
//
//#include <string>
//#include <utility>
//
//namespace eCAL
//{
//  namespace Util
//  {
//    /**
//     * @brief Retrieve eCAL data path.
//     *
//     *        Checks if a valid eCAL data path is available in the following order:
//     *        1. ECAL_DATA environment variable path
//     *        2. Local user path (win: Appdata/Local, unix: ~/.ecal)
//     *        3. System paths like /etc/ecal, ProgramData/eCAL
//     *
//     * @return  First directory that is not empty.
//     *          Returns empty string if no valid directory is found.
//    **/
//    ECAL_API std::string GeteCALDataDir();
//
//    /**
//     * @brief Retrieve eCAL standard logging path.
//     *          This is path is for the eCAL logging files.
//     *          This path has read/write permissions for standard users.
//     * 
//     *        1. ECAL_LOG_DIR environment variable path
//     *        2. ECAL_DATA environment variable path
//     *        3. Path provided by eCAL configuration
//     *        4. Path to local eCAL directory
//     *        5. For windows: ProgramData/eCAL if available
//     *        6. System temp path if available
//     *        7. Fallback path /ecal_tmp
//     *
//     * @return  eCAL logging path if exists.
//     *          Returns empty string if no valid path is found.
//    **/
//    ECAL_API std::string GeteCALLogDir();
//
//    /**
//     * @brief Send shutdown event to specified local user process using it's unit name.
//     *
//     * @param unit_name_   Process unit name.
//    **/
//    ECAL_API void ShutdownProcess(const std::string& unit_name_);
//
//    /**
//     * @brief Send shutdown event to specified local user process using it's process id.
//     *
//     * @param process_id_   Process id.
//    **/
//    ECAL_API void ShutdownProcess(int process_id_);
//
//    /**
//     * @brief Send shutdown event to all local user processes.
//    **/
//    ECAL_API void ShutdownProcesses();
//}
//
